#include <QThread>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QSpacerItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "servo_cmd.h"

#include "serialport.h"

static serialPort * serial;
static QVector<servo_cmd* > commands;

#define countof(e) (sizeof(e)/sizeof((e)[0]))

static const char* source_str =
R"({"cmd_arr":[

{
    "name": "SERVO_MOVE_TIME",
    "type": "move",
    "params":
    [
        {
            "name": "angle",
            "type": "int",
            "range": [0, 24000]
        },
        {
            "name": "time",
            "type": "int",
            "range": [0, 30000]
        }
    ],
    "write": 1,
    "read" : 2
},
{
    "name": "SERVO_MOVE_TIME_WAIT",
    "type": "move ext",
    "params":
    [
        {
            "name": "angle",
            "type": "int",
            "range": [0, 24000]
        },
        {
            "name": "time",
            "type": "int",
            "range": [0, 30000]
        }
    ],
    "write": 7,
    "read" : 8
},
{
    "name": "SERVO_MOVE_START",
    "type": "move ext",
    "params": [],
    "write": 11
},
{
    "name": "SERVO_MOVE_STOP",
    "type": "move ext",
    "params": [],
    "write": 12
},
{
    "name": "SERVO_ID",
    "type": "adj",
    "params":
    [
        {
            "name": "ID",
            "type": "int",
            "range": [0, 253]
        }
    ],
    "write": 13,
    "read" : 14
},
{
    "name": "SERVO_ANGLE_OFFSET_ADJUST",
    "type": "adj",
    "params":
    [
        {
            "name": "deviation",
            "type": "int",
            "range": [-125, 125]
        }
    ],
    "write": 17,
    "read" : 19
},
{
    "name": "SERVO_ANGLE_OFFSET_WRITE",
    "type": "adj",
    "params": [],
    "write": 18
},
{
    "name": "SERVO_ANGLE_LIMIT",
    "type": "lim",
    "params":
    [
        {
            "name": "minimum angle",
            "type": "int",
            "range": [0, 24000]
        },
        {
            "name": "maximum angle",
            "type": "int",
            "range": [0, 24000]
        }
    ],
    "write": 20,
    "read" : 21
},
{
    "name": "SERVO_VIN_LIMIT",
    "type": "lim",
    "params":
    [
        {
            "name": "minimum voltaje",
            "type": "int",
            "range": [4500, 12000]
        },
        {
            "name": "maximum voltaje",
            "type": "int",
            "range": [4500, 12000]
        }
    ],
    "write": 22,
    "read" : 23
},
{
    "name": "SERVO_TEMP_MAX_LIMIT",
    "type": "lim",
    "params":
    [
        {
            "name": "temperature",
            "type": "int",
            "range": [50, 100]
        }
    ],
    "write": 24,
    "read" : 25
},
{
    "name": "SERVO_TEMP_READ",
    "type": "mon",
    "params":
    [
        {
            "name": "temperature",
            "type": "int",
            "range": [0, 255]
        }
    ],
    "read" : 26
},
{
    "name": "SERVO_VIN_READ",
    "type": "mon",
    "params":
    [
        {
            "name": "voltage",
            "type": "int",
            "range": [0, 32768]
        }
    ],
    "read" : 27
},
{
    "name": "SERVO_POS_READ",
    "type": "mon",
    "params":
    [
        {
            "name": "voltage",
            "type": "int",
            "range": [-32767, 32768]
        }
    ],
    "read" : 28
},
{
    "name": "SERVO_OR_MOTOR_MODE",
    "type": "move",
    "params":
    [
        {
            "name": "servo",
            "type": "radio",
            "range": []
        },
        {
            "name": "motor",
            "type": "radio",
            "range": []
        },
        {
            "name": "speed",
            "type": "int",
            "range": [-1000, 1000]
        }
    ],
    "write": 29,
    "read" : 30
},
{
    "name": "SERVO_LOAD_OR_UNLOAD",
    "type": "move",
    "params":
    [
        {
            "name": "unload",
            "type": "radio",
            "range": []
        },
        {
            "name": "load",
            "type": "radio",
            "range": []
        }
    ],
    "write": 31,
    "read" : 32
},
{
    "name": "SERVO_LED_CTL",
    "type": "led",
    "params":
    [
        {
            "name": "on",
            "type": "radio",
            "range": []
        },
        {
            "name": "off",
            "type": "radio",
            "range": []
        }
    ],
    "write": 33,
    "read" : 34
},
{
    "name": "SERVO_LED_ERROR",
    "type": "led",
    "params":
    [
        {
            "name": "Over temperature",
            "type": "flag",
            "range": []
        },
        {
            "name": "Over voltage",
            "type": "flag",
            "range": []
        },
        {
            "name": "Locked-rotor",
            "type": "flag",
            "range": []
        }
    ],
    "write": 35,
    "read" : 36
}

]})";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QJsonArray cmd_json = QJsonDocument::fromJson(QByteArray(source_str)).object()["cmd_arr"].toArray();

    foreach (QJsonValue json_val, cmd_json)
    {
        servo_cmd* tmp_obj = new servo_cmd(nullptr, json_val.toObject());
        commands << tmp_obj;
        QString cmd_type = json_val.toObject()["type"].toString();

        int tab_num = 0;
        for (tab_num = 0; tab_num < ui->tabWidget->count(); tab_num++)
        {
            if (ui->tabWidget->tabText(tab_num) == cmd_type)
            {
                ui->tabWidget->widget(tab_num)->layout()->addWidget(tmp_obj);
                break;
            }
        }
        if (tab_num >= ui->tabWidget->count())
        {
            tab_num = ui->tabWidget->count();
            ui->tabWidget->addTab(new QWidget(), cmd_type);
            ui->tabWidget->widget(tab_num)->setLayout(new QVBoxLayout());
            ui->tabWidget->widget(tab_num)->layout()->addWidget(tmp_obj);
        }
    }
    for (int tab_num = 0; tab_num < ui->tabWidget->count(); tab_num++)
    {
        QSpacerItem * tmp_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->tabWidget->widget(tab_num)->layout()->addItem(tmp_spacer);
    }

    serial = new serialPort(this);
    ui->uart_list->setModel(serial);
    servo_cmd::set_serial_port(serial);

    // Сканируем порты при нажатии на кнопку
    QObject::connect(ui->btn_refresh_uart_list, SIGNAL(clicked(void)),
                     serial, SLOT(scan(void)));
    // Активируем список, если нашли порты
    QObject::connect(serial, SIGNAL(scan_complete(bool)),
                     ui->uart_list, SLOT(setEnabled(bool)));
    // Включаем или выключаем элементы в зависимости от того, открыт или закрыт порт
    QObject::connect(serial, SIGNAL(connect(bool)),
                     ui->btn_connect_uart, SLOT(setHidden(bool)));
    QObject::connect(serial, SIGNAL(connect(bool)),
                     ui->btn_close_uart, SLOT(setVisible(bool)));
    QObject::connect(serial, SIGNAL(connect(bool)),
                     ui->tabWidget, SLOT(setEnabled(bool)));

    ui->tabWidget->setEnabled(false);

    ui->btn_close_uart->setHidden(true);
    ui->btn_connect_uart->setHidden(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_connect_uart_clicked()
{
    serial->open_port(ui->uart_list->currentText());
}

void MainWindow::on_btn_close_uart_clicked()
{
    serial->close_port();
}

void MainWindow::on_uart_list_currentIndexChanged(int /*index*/)
{
    ui->btn_connect_uart->setVisible(!ui->uart_list->currentData(Qt::UserRole).toBool());
}
