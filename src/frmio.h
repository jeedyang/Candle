#ifndef FRMIO_H
#define FRMIO_H

#include <QDialog>
#include <QTimer>
#include <QButtonGroup>
#include <QPair>
#include <QList>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>


namespace Ui {
class frmIo;
}

class Plc;

class frmIo : public QDialog
{
    Q_OBJECT

public:
    explicit frmIo(Plc* plc,QWidget *parent = nullptr);
    ~frmIo();

    void startRefresh();
    void stopRefresh();

public slots:
    void refresh();

private:
    void saveInLabel();
    void saveOutLabel();
    void loadLabels();

    Ui::frmIo *ui;
    Plc* m_plc = nullptr;
    QTimer m_timer;
    QButtonGroup m_buttonGroup;

    QList<QPair<QPushButton*,QLineEdit*>> m_outEdits;
    QList<QPair<QLabel*,QLineEdit*>> m_inEdits;
};

#endif // FRMIO_H
