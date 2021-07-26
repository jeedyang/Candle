#include "plc.h"
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <QTimer>


const int     MODBUS_ID       =1;
const int       CMD_NEXT_PLATE  =4;
const int CMD_LAST_PLATE  =3;
const int CMD_PLATE_HOME  =2;
const int CMD_PLATE_EN    =0;


static void delayMs(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}

Plc::Plc(QObject *parent) : QObject(parent)
{

    QObject::connect(&m_waitTimer,SIGNAL(timeout()),this,SLOT(waitTimeOut()));

    QObject::connect(&m_waitTimeOutTimer,SIGNAL(timeout()),this,SLOT(waitTimeOutTimeOut()));
}

Plc::~Plc()
{
    m_waitTimer.stop();
}

int Plc::connect(QString port)
{
    return m_modbus.connectDevice(port);
}


void Plc::lastPlate()
{
    //  PLC addr    modbus addr
    //  M3          0+3          lastPlate

    if(waritParame())
    {
        m_modbus.write(MODBUS_ID,QModbusDataUnit::Coils,CMD_LAST_PLATE,1,{0x00ff});
    }else{
        emit hasMessage(tr("comm error"));
    }
    waitCoilOff(CMD_LAST_PLATE);
}

void Plc::nextPlate()
{
    //  PLC addr    modbus addr
    //  M4          0+4          lastPlate

    if(waritParame())
    {
        m_modbus.write(MODBUS_ID,QModbusDataUnit::Coils,CMD_NEXT_PLATE,1,{0x00ff});
    }else{
        emit hasMessage(tr("comm error"));
    }
    waitCoilOff(CMD_NEXT_PLATE);
}

void Plc::waitTimeOut()
{
    QModbusDataUnit data = m_modbus.read(MODBUS_ID,QModbusDataUnit::Coils,m_waitCoilAddr,1);
    if(data.isValid() && data.values().size()!=0 && data.value(0)==0){
        coilOff(m_waitCoilAddr);
        m_waitTimeOutTimer.stop();
        m_waitTimer.stop();
    }
}

void Plc::waitTimeOutTimeOut()
{
    emit hasMessage(tr("Execute code:%1 timeout!").arg(m_waitCoilAddr));
    emit hasError(m_waitCoilAddr);
    m_waitTimeOutTimer.stop();
    m_waitTimer.stop();
}


void Plc::indexPlateHome()
{
    emit hasMessage(tr("Index plate homing..."));
    //  PLC addr    modbus addr
    //  M2          0+2          home
    if(waritParame())
    {
        m_modbus.write(MODBUS_ID,QModbusDataUnit::Coils,CMD_PLATE_HOME,1,{0x00ff});
    }else{
        emit hasMessage(tr("comm error"));
    }
    waitCoilOff(CMD_PLATE_HOME,50000);
}


int Plc::waritParame()
{
    //  param name      PLC addr    modbus addr
    //  homeOffset      HD0         0x0A080+0       0 ~ 36000
    //  velocityRange   HD2         0x0A080+2       0,1,2
    //  angle           HD4         0x0A080+4       0 ~ 36000
    Q_ASSERT(m_set);
    int homeOffset = static_cast<int>(m_set->indexPlateHomeOffset()*100.0);
    int velocityRange = m_set->indexPlateSpeed();
    int angle = 36000 / m_set->plateCount();

    QVector<quint16> data;

    qDebug("%x",homeOffset);

    data.append((homeOffset)&0x0000ffff);
    data.append((homeOffset>>16)&0x0000ffff);

    qDebug("%x",data.at(0));
    qDebug("%x",data.at(1));


    data.append((velocityRange)&0x0000ffff);
    data.append((velocityRange>>16)&0x0000ffff);

    data.append((angle)&0x0000ffff);
    data.append((angle>>16)&0x0000ffff);

    return m_modbus.write(MODBUS_ID,QModbusDataUnit::HoldingRegisters,0x0A080,6,data);
}

void Plc::setIndexPlateEn(bool en)
{
    //  PLC addr    modbus addr
    //  M0          0+0          en
    quint16 data = 0;
    if (en)
        data = 0x00ff;
    m_modbus.write(1,QModbusDataUnit::Coils,0,1,{data});
}

void Plc::waitCoilOff(int addr, int timeOut)
{
    m_waitCoilAddr = addr;

    m_waitTimer.start(100);
    m_waitTimeOutTimer.start(timeOut);
}

void Plc::coilOff(int addr)
{
    QObject::disconnect(&m_waitTimer);
    if(addr == CMD_NEXT_PLATE)
        emit nextPlateDone();
    else if (addr == CMD_LAST_PLATE)
        emit lastPlateDone();
    else if (addr == CMD_PLATE_HOME){
        emit hasMessage(tr("Index plate home done"));
        emit indexPlateHomeDone();
    }


}

bool Plc::indexPlateEn()
{
    QModbusDataUnit result = m_modbus.read(MODBUS_ID,QModbusDataUnit::Coils,0,1);
    if(result.isValid()){
        if(result.value(0)!=0)
            return true;
    }
    return false;
}

