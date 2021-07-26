#ifndef MODBUS_H
#define MODBUS_H

#include <QObject>
#include <QSerialPort>
#include <QModbusDataUnit>
#include <QModbusRtuSerialMaster>

class Modbus : public QObject
{
    Q_OBJECT

public:
    Modbus();
    ~Modbus();

    int connectDevice(QString portName);
    void disconnectDevice(void);
    QModbusDataUnit read(int serverAddress, QModbusDataUnit::RegisterType table, int startAddress, quint16 numOfEntries,int timeOut=1000);
    QModbusDataUnit read(int serverAddress, QModbusDataUnit dataunit,int timeOut=1000);
    bool write(int serverAddress, QModbusDataUnit::RegisterType table, int startAddress, quint16 numOfEntries, QVector<quint16> data,int timeOut=1000);
    bool write(int serverAddress, QModbusDataUnit dataunit,int timeOut=1000);

signals:
    void readReady(QModbusDataUnit dataUnit);

public slots:
    void on_modbusStateChanged(int state);
    void modbusMessage(void);
    void modbusMessage(QString mess);

public:
    struct Settings {
        QString portName;
        int parity = QSerialPort::NoParity;
        int baud = QSerialPort::Baud115200;
        int dataBits = QSerialPort::Data8;
        int stopBits = QSerialPort::OneStop;
        int responseTime = 1000;
        int numberOfRetries = 1;
    };

private:
    QModbusClient *m_modbusDev = nullptr;
    Settings m_settings;
};

#endif

