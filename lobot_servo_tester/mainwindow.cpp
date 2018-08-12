#include <QThread>
#include <QJsonDocument>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "servo_cmd.h"

#include "serialport.h"

static serialPort * serial;
static servo_cmd * commands[2];

#define countof(e) (sizeof(e)/sizeof((e)[0]))

static const char* source_str = R"(
{
    "SERVO_ID":
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

        "write":
        {
            "code": "0x13"
        },

        "read":
        {
            "code": "0x14"
        }
    }
}
)";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QJsonObject cmd_json = QJsonDocument::fromJson(QByteArray(source_str)).object();

    for (uint8_t i = 0; i < cmd_json.count(); i++)
    {
        commands[i] = new servo_cmd(nullptr, cmd_json.value(cmd_json.keys()[i]).toObject());
        ui->lay_cmd_list_layout->addWidget(commands[i]);
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
