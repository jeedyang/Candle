#include "frmio.h"
#include "ui_frmio.h"
#include "plc.h"
#include <QDebug>
#include <QSettings>


frmIo::frmIo(Plc* plc,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmIo),
    m_plc(plc)
{
    ui->setupUi(this);

    connect(&m_timer,&QTimer::timeout,this,&frmIo::refresh);

    for(auto btn : ui->groupBoxOut->findChildren<QPushButton*>())
    {
        m_buttonGroup.addButton(btn);
        btn->setCheckable(true);
        btn->setAutoDefault(false);
    }
    m_buttonGroup.setExclusive(false);
    connect(&m_buttonGroup,&QButtonGroup::idToggled,[&](int id,bool checked){
        int num = this->m_buttonGroup.button(id)->property("number").toInt();
        m_plc->getModbus()->write(1,QModbusDataUnit::Coils,0x6100+num,1,{static_cast<quint16>(checked)});
    });

    connect(ui->btnEditLabel,&QPushButton::released,this,[&](){
        for(auto element : m_outEdits){
            if(element.second->isHidden()){
                element.second->setText(element.first->text());
                element.second->show();
            }else{
                element.second->hide();
                saveOutLabel();
            }
        }
        for(auto element : m_inEdits){
            if(element.second->isHidden()){
                element.second->setText(element.first->text());
                element.second->show();
            }else{
                element.second->hide();
                saveInLabel();
            }
        }
    });

    for(auto btn : ui->groupBoxOut->findChildren<QPushButton*>())
    {
        QLineEdit* edit=new QLineEdit(btn->text());
        edit->setParent(btn);
        edit->hide();
        m_outEdits.append(QPair<QPushButton*,QLineEdit*>(btn,edit));
        connect(edit,&QLineEdit::textEdited,btn,&QPushButton::setText);
    }
    for(const auto &lab : ui->groupBoxIn->findChildren<QLabel*>())
    {
        QLineEdit* edit=new QLineEdit(lab->text());
        edit->setParent(lab);
        edit->hide();
        m_inEdits.append(QPair<QLabel*,QLineEdit*>(lab,edit));
        connect(edit,&QLineEdit::textEdited,lab,&QLabel::setText);
    }
    loadLabels();
}

frmIo::~frmIo()
{
    delete ui;
}

void frmIo::startRefresh()
{
    m_timer.start(300);
}

void frmIo::stopRefresh()
{
    m_timer.stop();
}

void frmIo::refresh()
{

    //第一个模块，输出模块
    QModbusDataUnit data = m_plc->getModbus()->read(1,QModbusDataUnit::Coils,0x6100,16);
    qDebug()<<data.values();
    if(data.values().length()!=16)
        return;
    for(auto btn : ui->groupBoxOut->findChildren<QPushButton*>())
    {
        int num = btn->property("number").toInt();
        if(num>=data.values().length())
            return;
        btn->setChecked(data.values().at(num));
    }

    //background-color: rgb(34, 68, 50);
    //background-color: rgb(0, 170, 0);
    //
    //第二个模块，输入模块
    data = m_plc->getModbus()->read(1,QModbusDataUnit::Coils,0x5140,32);
    qDebug()<<data.values();
    if(data.values().length()!=32)
        return;
    for(auto lab : ui->groupBoxIn->findChildren<QLabel*>())
    {
        int num = lab->property("number").toInt();
        if(num>=data.values().length())
            return;
        if(data.values().at(num))
            lab->setStyleSheet("background-color: rgb(0, 170, 0);");
        else
            lab->setStyleSheet("background-color: rgb(34, 68, 50);");
    }


}

void frmIo::saveInLabel()
{
    QSettings set("settings.ini", QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    QString labels;
    for(auto element : m_inEdits){
        labels.append(element.first->text());
        labels.append(",");
    }
    labels.remove(labels.size()-1,1);
    set.setValue("inLabels",labels);

}

void frmIo::saveOutLabel()
{
    QSettings set("settings.ini", QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    QString labels;
    for(auto element : m_outEdits){
        labels.append(element.first->text());
        labels.append(",");
    }
    labels.remove(labels.size()-1,1);
    set.setValue("outLabels",labels);
}

void frmIo::loadLabels()
{
    QSettings set("settings.ini", QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    QStringList inLabels = set.value("inLabels").toString().split (',');
    QStringList outLabels = set.value("outLabels").toString().split (',');
    qDebug()<<inLabels<<outLabels;

    qDebug()<<m_outEdits.size()<<outLabels.size();
    qDebug()<<m_inEdits.size()<<inLabels.size();

    if(32!=inLabels.size())
        return;
    if(16!=outLabels.count())
        return;
    int i=0;
    for(auto element : m_inEdits){

        element.first->setText(inLabels.at(i));
        i++;
    }
    i=0;
    for(auto element : m_outEdits){

        element.first->setText(outLabels.at(i));
        i++;
    }



}
