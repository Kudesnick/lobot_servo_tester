#include <QThread>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

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
    "params": [],
    "write": 11
},
{
    "name": "SERVO_MOVE_STOP",
    "params": [],
    "write": 12
},
{
    "name": "SERVO_ID",
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
    "params": [],
    "write": 18
},
{
    "name": "SERVO_ANGLE_LIMIT",
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
    "params":
    [
        {
            "name": "load",
            "type": "radio",
            "range": []
        },
        {
            "name": "unload",
            "type": "radio",
            "range": []
        }
    ],
    "write": 31,
    "read" : 32
},
{
    "name": "SERVO_LED_CTL",
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
        ui->lay_cmd_list_layout->addWidget(tmp_obj);
    }

    serial = new serialPort(this);
    ui->uart_list->setModel(serial);

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
                     ui->gr_servo_actions, SLOT(setEnabled(bool)));

    ui->gr_servo_actions->setEnabled(false);
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

static bool LobotCheckSum(QByteArray &data)
{
  char temp = 0;

  for (uint8_t i = 2; i < data[3] + 2; i++)
  {
    temp += data[i];
  }
  temp = ~temp;

  bool result = false;

  if (data[data.size() - 1] == temp)
  {
      result = true;
  }
  else
  {
    data[data.size() - 1] = temp;
  }

  return result;
}

void MainWindow::on_btn_SERVO_ID_READ_clicked()
{
    const uint8_t cmd[] = {0x55, 0x55, 0xfe, 0x03, 0x0e, 0x00};
    QByteArray data((const char *)cmd, sizeof(cmd));

    LobotCheckSum(data);
    serial->sync_data_send(data);

    if (data.size() >= 6 && LobotCheckSum(data) == true)
    {
        ui->ind_SERVO_ID->setValue(data[5]);
    }
}

void MainWindow::on_uart_list_currentIndexChanged(int /*index*/)
{
    ui->btn_connect_uart->setEnabled(!ui->uart_list->currentData(Qt::UserRole).toBool());
}
