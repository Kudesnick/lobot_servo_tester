#ifndef SERVO_CMD_H
#define SERVO_CMD_H

#include <QWidget>
#include <QJsonObject>
#include <QSpinBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include "serialport.h"

namespace Ui {
class servo_cmd;
}

class servo_cmd : public QWidget
{
    Q_OBJECT

public:
    explicit servo_cmd(QWidget *parent = nullptr);
    servo_cmd(QWidget *parent, const QJsonObject &cmd);
    ~servo_cmd();
    static uint8_t id;
    static const uint8_t preamble[];
    static void set_serial_port(serialPort * _serial);

private slots:
    void on_btn_set_clicked();
    void on_btn_get_clicked();

private:
    Ui::servo_cmd *ui;
    QJsonObject json_cmd;
    QVector<QSpinBox* >     VectorSpin;
    QVector<QLabel* >       VectorLabel;
    QVector<QRadioButton* > VectorRadio;
    QVector<QCheckBox* >    VectorFlag;
    static serialPort * serial;
    QVector<uint8_t> msg;
    bool LobotCheckSum(void);
    bool reqParse(void);
};

#endif // SERVO_CMD_H
