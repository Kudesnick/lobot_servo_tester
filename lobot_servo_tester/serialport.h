#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QAbstractListModel>
#include <QSerialPort>
#include <QSerialPortInfo>

class serialPort : public QAbstractListModel
{
    Q_OBJECT

    QList<QSerialPortInfo> serial_info_list;
    QSerialPort serial;

public:
    explicit serialPort(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool open_port(const QString &name);
    void close_port(void);
    void sync_data_send(QByteArray &data);

public slots:
    void scan(void);

signals:
    void scan_complete(bool is_not_empty);
    void connect(bool is_connect);

private:

};

#endif // SERIALPORT_H
