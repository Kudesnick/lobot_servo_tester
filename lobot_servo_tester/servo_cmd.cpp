#include <QDebug>
#include <QJsonArray>

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
    ui->lay_params->setHidden(true);
    ui->sp_param->setHidden(true);

    QJsonArray params = json_cmd.find("params").value().toArray();
    for (int i = 0; i < params.count(); i++)
    {
        QJsonObject param = params[i].toObject();


    }
}

servo_cmd::~servo_cmd()
{
    delete ui;
}
