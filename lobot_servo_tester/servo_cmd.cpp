#include <QDebug>
#include <QJsonArray>
#include <QSpinBox>
#include <QLabel>

#include "servo_cmd.h"
#include "ui_servo_cmd.h"

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
    QVector<QSpinBox* >     VectorSpin;
    QVector<QLabel* >       VectorLabel;
    QVector<QRadioButton* > VectorRadio;
    QVector<QCheckBox* >    VectorFlag;

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
    ui->lay_params->setVisible(VectorSpin.count() > 1);
    ui->lay_radio->setVisible(VectorRadio.count() > 1);
    ui->lay_flags->setVisible(VectorFlag.count() > 0);
}

servo_cmd::~servo_cmd()
{
    delete ui;
}
