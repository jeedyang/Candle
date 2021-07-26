#include "modbus.h"
#include <QModbusDevice>
#include <QtDebug>
#include <QEventLoop>
#include <QTimer>


Modbus::Modbus()
{
    m_modbusDev = new QModbusRtuSerialMaster(nullptr);
    connect(m_modbusDev, SIGNAL(errorOccurred()), this, SLOT(modbusMessage()));

    if(m_modbusDev)
    {
        qDebug("创建 Modbus Master 成功。");
        connect(m_modbusDev, &QModbusClient::stateChanged, this, &Modbus::on_modbusStateChanged);
    }
    else
    {
        qDebug("创建 Modbus Master 失败。");
    }
}

Modbus::~Modbus()
{
    delete m_modbusDev;
}

int Modbus::connectDevice(QString portName)
{
    if(!m_modbusDev) return -2;

    m_settings.portName = portName;
    if(m_modbusDev->state() != QModbusDevice::ConnectedState)
    {
        m_modbusDev->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_settings.portName);
        m_modbusDev->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_settings.baud);
        m_modbusDev->setConnectionParameter(QModbusDevice::SerialParityParameter, m_settings.parity);
        m_modbusDev->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, m_settings.dataBits);
        m_modbusDev->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, m_settings.stopBits);

        m_modbusDev->setTimeout(m_settings.responseTime);
        m_modbusDev->setNumberOfRetries(m_settings.numberOfRetries);

        if(m_modbusDev->connectDevice())
        {
            qDebug("Modbus 连接成功。");
            return 0;
        }
        else
        {
            qDebug("Modbus 连接失败。");
            return -1;
        }
    }
    return -2;
}

void Modbus::disconnectDevice(void)
{
    if (m_modbusDev)   m_modbusDev->disconnectDevice();
    delete m_modbusDev;
    m_modbusDev = nullptr;
}

void Modbus::on_modbusStateChanged(int state)
{
    if(state == QModbusDevice::UnconnectedState)
    {
        qDebug("Modbus 已断开。");
    }
    else if(state == QModbusDevice::ConnectedState)
    {
        qDebug("Modbus 已连接。");
    }
}

void Modbus::modbusMessage(void)
{
    qDebug() << m_modbusDev->errorString();
}

void Modbus::modbusMessage(QString mess)
{
    qDebug() << mess;
}

QModbusDataUnit Modbus::read(int serverAddress, QModbusDataUnit::RegisterType type, int startAddress, quint16 numOfEntries,int timeOut)
{
    if(!m_modbusDev || m_modbusDev->state() != QModbusDevice::ConnectedState)
    {
        qDebug("Modbus Device is not connected!");
        return QModbusDataUnit();
    }
    QModbusDataUnit req = QModbusDataUnit(type,startAddress,numOfEntries);
    return read(serverAddress,req,timeOut);
}

QModbusDataUnit Modbus::read(int serverAddress, QModbusDataUnit dataunit,int timeOut)
{
    //qDebug()<<dataunit.startAddress();
    if(!m_modbusDev || m_modbusDev->state() != QModbusDevice::ConnectedState)
    {
        qDebug("Modbus Device is not connected!");
        return QModbusDataUnit();
    }
    if(auto * reply = m_modbusDev->sendReadRequest(dataunit,serverAddress)){
        QEventLoop loop;
        auto conn = QObject::connect(reply,&QModbusReply::finished,&loop,&QEventLoop::quit);
        QTimer::singleShot(timeOut, &loop, SLOT(quit()));
        loop.exec(QEventLoop::ExcludeUserInputEvents);
        if(reply->error() == QModbusDevice::ProtocolError){
            qDebug()<<"reply->error() == QModbusDevice::ProtocolError";
            return QModbusDataUnit();
        }
        QObject::disconnect(conn);
        auto result = reply->result();
        reply->deleteLater();
        return result;
    }
    return QModbusDataUnit();
}

bool Modbus::write(int serverAddress, QModbusDataUnit::RegisterType table, int startAddress, quint16 numOfEntries, QVector<quint16> data,int timeOut)
{
    if(!m_modbusDev || m_modbusDev->state() != QModbusDevice::ConnectedState){
        qDebug("Modbus Device is not connected!");
        return false;
    }

    QModbusDataUnit dataunit = QModbusDataUnit(table, startAddress,static_cast<quint16>(numOfEntries));

    for(uint i = 0; i < dataunit.valueCount(); i++)
    {
        int index = static_cast<int>(i);
        dataunit.setValue(index,  data.at(index));
    }
    return write(serverAddress,dataunit,timeOut);

}

bool Modbus::write(int serverAddress, QModbusDataUnit dataunit,int timeOut)
{
    QModbusReply* reply = m_modbusDev->sendWriteRequest(dataunit,  serverAddress);
    QEventLoop loop;
    bool ret = true;
    auto conn = QObject::connect(reply,&QModbusReply::finished,&loop,&QEventLoop::quit);
    QTimer::singleShot(timeOut, &loop, SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    if(reply->error() == QModbusDevice::ProtocolError){
        qDebug()<<"fail ";
        ret = false;
    }
    QObject::disconnect(conn);
    reply->deleteLater();
    return ret;
}
