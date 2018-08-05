#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>

static QSerialPort serial;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->gr_servo_actions->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_refresh_uart_list_clicked()
{
    ui->uart_list->clear();

    static auto infos = QSerialPortInfo::availablePorts();

    infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos)
    {
        ui->uart_list->addItem(info.portName());
    }

    ui->uart_list->setEnabled(ui->uart_list->count() > 0);
    ui->btn_connect_uart->setEnabled(ui->uart_list->count() > 0);
}

void MainWindow::on_btn_connect_uart_clicked()
{
    serial.close();
    serial.setPortName(ui->uart_list->currentText());
    serial.setBaudRate(115200);

    if (serial.open(QIODevice::ReadWrite))
    {
        ui->btn_connect_uart->setVisible(false);
        ui->uart_list->setEnabled(false);
        ui->gr_servo_actions->setEnabled(true);
    }
}

void MainWindow::on_btn_close_uart_clicked()
{
    serial.close();
    ui->btn_connect_uart->setVisible(true);
    ui->uart_list->setEnabled(true);
    ui->gr_servo_actions->setEnabled(false);
}

static bool LobotCheckSum(std::vector<uint8_t> &data)
{
  uint8_t temp = 0;

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

    std::vector<uint8_t> outpt_data = {0x55, 0x55, 0xfe, 0x03, 0x0e, 0x00};
    LobotCheckSum(outpt_data);

    serial.clear();
    serial.write((const char *)(outpt_data.data()), outpt_data.size());
    serial.waitForReadyRead(3000);
    QByteArray rx_data = serial.readAll();
    std::vector<uint8_t> inpt_data(static_cast<std::vector<uint8_t>::size_type>(rx_data.size()) - outpt_data.size());
    memcpy(inpt_data.data(), rx_data.data() + outpt_data.size(), static_cast<size_t>(inpt_data.size()));
    serial.clear();

    if (inpt_data.size() >= 6 && LobotCheckSum(inpt_data) == true)
    {
        ui->ind_SERVO_ID->setValue(inpt_data[5]);
    }
}
