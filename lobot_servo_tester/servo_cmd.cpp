#include <QDebug>
#include <QJsonArray>
#include <QSpinBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>

#include "servo_cmd.h"
#include "ui_servo_cmd.h"
#include "serialport.h"

servo_cmd::servo_cmd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::servo_cmd)
{
    ui->setupUi(this); 
}

servo_cmd::servo_cmd(QWidget *parent, const QJsonObject &cmd):
    servo_cmd(parent)
{
    json_cmd = cmd;

    // Title
    ui->set_group->setTitle(json_cmd["name"].toString());
    // Buttons
    ui->btn_set->setVisible(json_cmd["write"].isDouble());
    ui->btn_get->setVisible(json_cmd["read"].isDouble());
    // Params
    QJsonArray params = json_cmd["params"].toArray();

    for (int i = 0; i < params.count(); i++)
    {
        QJsonObject param = params[i].toObject();

        if (param["type"].toString() == "int")
        {
            QSpinBox* tmpSpinBox = new QSpinBox();
            VectorLabel << new QLabel(param["name"].toString());
            VectorSpin << tmpSpinBox;
            tmpSpinBox->setMinimum(param["range"].toArray()[0].toInt());
            tmpSpinBox->setMaximum(param["range"].toArray()[1].toInt());
            tmpSpinBox->setValue(tmpSpinBox->minimum());
        }
        else if (param["type"].toString() == "radio")
        {
            VectorRadio << new QRadioButton(param["name"].toString());
        }
        else if (param["type"].toString() == "flag")
        {
            VectorFlag << new QCheckBox(param["name"].toString());
        }
    }

    for (int i = 0; i < VectorLabel.count(); i++)
    {
        ui->lay_params_layout->addRow(VectorLabel.at(i), VectorSpin.at(i));
    }
    if (VectorSpin.count() == 1)
    {
        ui->lonely_spin_layout->addWidget(VectorSpin.at(0));
    }
    foreach(QRadioButton * element, VectorRadio)
    {
        ui->lay_radio_layout->addWidget(element);
    }
    foreach(QCheckBox * element, VectorFlag)
    {
        ui->lay_flags_layout->addWidget(element);
    }
    ui->lay_params->setVisible(VectorSpin.count() > 1);
    ui->lay_radio->setVisible(VectorRadio.count() > 1);
    ui->lay_flags->setVisible(VectorFlag.count() > 0);
}

bool servo_cmd::LobotCheckSum(void)
{
  uint8_t temp = 0;

  for (uint8_t i = 2; i < msg[3] + 2; i++)
  {
    temp += msg[i];
  }
  temp = ~temp;

  bool result = false;

  if (msg[msg.size() - 1] == temp)
  {
      result = true;
  }
  else
  {
    msg[msg.size() - 1] = temp;
  }

  return result;
}

bool servo_cmd::reqParse(void)
{
    if (msg.count() < 6) return false;
    if (msg[3] != static_cast<uint8_t>(msg.count() - 3) || !LobotCheckSum()) return false;

    if (msg.takeFirst() != 0x55) return false;
    if (msg.takeFirst() != 0x55) return false;

    uint8_t dev_id = msg.takeFirst();
    /*uint8_t len = */ msg.takeFirst();
    uint8_t cmd_id = msg.takeFirst();

    if (dev_id != id && cmd_id != 0x0E) return false;
    if (cmd_id != json_cmd["read"].toInt() && cmd_id != json_cmd["write"].toInt()) return false;

    if (VectorRadio.count() > 1)
    {
        uint8_t data = msg.takeFirst();

        for (uint8_t i = 0; i < VectorRadio.count(); i++)
        {
            VectorRadio[i]->setChecked(i == data);
        }
    }

    if (VectorFlag.count() > 1)
    {
        uint8_t data = msg.takeFirst();

        for (uint8_t i = 0; i < VectorFlag.count(); i++)
        {
            VectorFlag[i]->setChecked(1 << i == data);
        }

        msg.append(data);
    }

    if (VectorSpin.count() > 0)
    {
        for (uint8_t i = 0; i < VectorSpin.count(); i++)
        {
            if (VectorSpin[i]->maximum() < 0xFF && VectorSpin[i]->minimum() > -127)
            {
                if (VectorSpin[i]->minimum() >= 0)
                {
                    VectorSpin[i]->setValue(msg.takeFirst());
                }
                else
                {
                    VectorSpin[i]->setValue((int8_t)msg.takeFirst());
                }
            }
            else
            {
                if (i == 0)
                {
                    if (VectorRadio.count() > 1 || VectorFlag.count() > 1)
                    {
                        msg.takeFirst();
                    }
                }

                uint16_t data = msg.takeFirst();
                data |= msg.takeFirst() << 8;

                if (VectorSpin[i]->minimum() >= 0)
                {
                    VectorSpin[i]->setValue(data);
                }
                else
                {
                    VectorSpin[i]->setValue((int16_t)data);
                }
            }
        }
    }

    if (cmd_id == 0x0E)
    {
        id = dev_id;
    }

    return true;
}

void servo_cmd::on_btn_set_clicked()
{
    msg.clear();

    msg.append(0x55);
    msg.append(0x55);

    if (json_cmd["name"].toString() == "SERVO_ID")
    {
        msg.append(0xFE);
    }
    else
    {
        msg.append(servo_cmd::id);
    }

    msg.append(0);

    msg.append(static_cast<uint8_t>(json_cmd["write"].toInt()));

    if (VectorRadio.count() > 1)
    {
        uint8_t data = 0;

        for (uint8_t i = 0; i < VectorRadio.count(); i++)
        {
            if (VectorRadio[i]->isChecked())
            {
                data = i;
                break;
            }
        }

        msg.append(data);
    }

    if (VectorFlag.count() > 1)
    {
        uint8_t data = 0;

        for (uint8_t i = 0; i < VectorFlag.count(); i++)
        {
            if (VectorFlag[i]->isChecked())
            {
                data |= 1 << i;
                break;
            }
        }

        msg.append(data);
    }

    if (VectorSpin.count() > 0)
    {
        for (uint8_t i = 0; i < VectorSpin.count(); i++)
        {
            if (VectorSpin[i]->maximum() < 0xFF && VectorSpin[i]->minimum() > -127)
            {
                msg.append(static_cast<uint8_t>(VectorSpin[i]->value()));
            }
            else
            {
                if (i == 0)
                {
                    if (VectorRadio.count() > 1 || VectorFlag.count() > 1)
                    {
                        msg.append(0);
                    }
                }

                msg.append(static_cast<uint8_t>(VectorSpin[i]->value()));
                msg.append(static_cast<uint8_t>(VectorSpin[i]->value() >> 8));
            }
        }
    }

    msg[3] = static_cast<uint8_t>(msg.count() - 2);

    msg.append(0);

    LobotCheckSum();

    if (serial != nullptr)
    {
        serial->sync_data_send(msg);

        reqParse();
    }
}

void servo_cmd::on_btn_get_clicked()
{
    msg.clear();

    msg.append(0x55);
    msg.append(0x55);

    if (json_cmd["name"].toString() == "SERVO_ID")
    {
        msg.append(0xFE);
    }
    else
    {
        msg.append(servo_cmd::id);
    }

    msg.append(3);

    msg.append(static_cast<uint8_t>(json_cmd["read"].toInt()));

    msg.append(0);

    LobotCheckSum();

    if (serial != nullptr)
    {
        serial->sync_data_send(msg);

        reqParse();
    }
}

uint8_t servo_cmd::id = 0;
const uint8_t servo_cmd::preamble[] = {0x55, 0x55};
serialPort * servo_cmd::serial = nullptr;

void servo_cmd::set_serial_port(serialPort * _serial)
{
    serial = _serial;
}

servo_cmd::~servo_cmd()
{
    delete ui;
}
