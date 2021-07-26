#ifndef PLC_H
#define PLC_H

#include <QObject>
#include <QTimer>
#include "modbus.h"
#include "frmsettings.h"

class Plc : public QObject
{
    Q_OBJECT
public:
    explicit Plc(QObject *parent = nullptr);
    ~Plc();

    int connect(QString port);

    void disConnect();

    void setSeting(frmSettings* set){m_set = set;}


    void indexPlateHome();
    int waritParame();
    bool indexPlateEn();

    Modbus* getModbus(){return &m_modbus;}

public slots:
    void setIndexPlateEn(bool en);
    void lastPlate();
    void nextPlate();

    void waitTimeOut();
    void waitTimeOutTimeOut();

signals:
    void hasMessage(QString msg);
    void hasError(int addr);

    void nextPlateDone();
    void lastPlateDone();
    void indexPlateHomeDone();

private:
    void waitCoilOff(int addr,int timeOut=10000);//uint: ms
    void coilOff(int addr);

    Modbus m_modbus;
    frmSettings* m_set;
    QTimer m_waitTimer;
    QTimer m_waitTimeOutTimer;
    int m_waitCoilAddr=0;
};

#endif // PLC_H
