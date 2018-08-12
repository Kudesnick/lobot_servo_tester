#ifndef SERVO_CMD_H
#define SERVO_CMD_H

#include <QWidget>
#include <QJsonObject>

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

private:
    Ui::servo_cmd *ui;
    QJsonObject json_cmd;
};

#endif // SERVO_CMD_H
