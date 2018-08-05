#include "servo_cmd.h"
#include "ui_servo_cmd.h"

servo_cmd::servo_cmd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::servo_cmd)
{
    ui->setupUi(this);
}

servo_cmd::~servo_cmd()
{
    delete ui;
}
