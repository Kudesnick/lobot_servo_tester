#include "serialport.h"

serialPort::serialPort(QObject *parent)
    : QAbstractListModel(parent)
{
    // serial_info_list = QSerialPortInfo::availablePorts();
}

int serialPort::rowCount(const QModelIndex &/*parent*/) const
{
    return serial_info_list.count();
}

QVariant serialPort::data(const QModelIndex &index, int role) const
{
    const QSerialPortInfo& data = serial_info_list.at(index.row());
    QVariant value;

    switch ( role )
    {
        case Qt::DisplayRole:
        {
            value = data.portName();
        }
        break;

        case Qt::UserRole:
        {
            value = data.isBusy();
        }
        break;

        default:
            break;
    }

    return value;
}

void serialPort::scan(void)
{
    serial_info_list = QSerialPortInfo::availablePorts();

    emit scan_complete(serial_info_list.size() > 0);
}

bool serialPort::open_port(const QString &name)
{
    serial.close();
    serial.setPortName(name);
    serial.setBaudRate(115200);

    bool result = serial.open(QIODevice::ReadWrite);
    emit connect(result);

    return result;
}

void serialPort::close_port(void)
{
    serial.close();
    emit connect(false);
}

void serialPort::sync_data_send(QVector<uint8_t> &data)
{
    int tx_len = data.size();

    serial.clear();
    serial.write(reinterpret_cast<const char *>(data.data()), data.size());
    serial.waitForReadyRead(3000);
    QByteArray tmp_bufQByte(serial.readAll().mid(tx_len, -1));
    data.resize(tmp_bufQByte.size());
    if (data.size() > 0)
    {
        memcpy(data.data(), tmp_bufQByte.data(), data.size());
    }
    serial.clear();
}
