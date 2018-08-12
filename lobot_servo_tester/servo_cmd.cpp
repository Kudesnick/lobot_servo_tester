#include <QDebug>
#include <QJsonArray>
#include <QSpinBox>

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
    ui->set_group->setTitle(json_cmd.find("name").value().toString());
    // Buttons
    ui->btn_set->setVisible(json_cmd.find("write").value().isObject());
    ui->btn_get->setVisible(json_cmd.find("read").value().isObject());
    // Params
    ui->lay_radio->setHidden(true);
    ui->lay_flags->setHidden(true);
//    ui->lay_params->setHidden(true);

    QJsonArray params = json_cmd.find("params").value().toArray();
    QVector<QSpinBox* >     VectorSpin;
    QVector<QLabel* >       VectorLabel;
    QVector<QRadioButton* > VectorRadio;
    QVector<QCheckBox* >    VectorFlag;

    for (int i = 0; i < params.count(); i++)
    {
        QJsonObject param = params[i].toObject();

        if (param.find("type").value().toString() == "int")
        {
            QSpinBox* tmpSpinBox = new QSpinBox();
            VectorLabel << new QLabel(param.find("name").value().toString());
            VectorSpin << tmpSpinBox;
            tmpSpinBox->setMinimum(param.find("rande").value().toArray().at(0).toInt());
            tmpSpinBox->setMaximum(param.find("rande").value().toArray().at(1).toInt());
            tmpSpinBox->setValue(tmpSpinBox->minimum());
        }
        else if (param.find("type").value().toString() == "radio")
        {
            VectorRadio << new QRadioButton(param.find("name").value().toString());
        }
        else if (param.find("type").value().toString() == "flag")
        {
            VectorFlag << new QCheckBox(param.find("name").value().toString());
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
}

servo_cmd::~servo_cmd()
{
    delete ui;
}
