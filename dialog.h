#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QToolButton>
#include "serialthread.h"
#include "customtoolbutton.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    void ShowMessage(QString);
    void EnumPorts();

private slots:
    void OnTimeout();
    void OnEnumComPort(QStringList);
    void on_btnPort_clicked();
    void on_btnClose_clicked();
    void OnCommuncation(bool send,QByteArray);
    void OnError(QString);
    void OnStatus(char,char,char,char,char,float);
    void OnPosition(float,float);
    void OnTempVoltage(float,float);
    void OnLVDTDiff(float,float);
    void on_btnAMove_clicked();
    void on_btnCali_clicked();
    void on_chkStatus_clicked();
    void on_btnClear_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_btnHome_clicked();
    void on_btnSpeed_clicked();
    void on_btnAuto_clicked();
    void on_btnOffset_clicked();
    void on_btnStop_clicked();
    void on_btnHomeTimeout_clicked();
    void on_btnMoveTimeout_clicked();
    void on_btnC_clicked();
    void on_btnT_clicked();
    void on_btnClear_2_clicked();
    void on_hsHomeTimeout_valueChanged(int value);
    void on_hsMoveTimeout_valueChanged(int value);

private:
    Ui::Dialog *ui;
    QList<QSerialPortInfo>  m_vSerialInfos;
    SerialThread    m_SerialThread;
    QTimer          m_tmInit;
    QList<CustomToolButton*> m_btnStatus;
};
#endif // DIALOG_H
