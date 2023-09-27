#include "dialog.h"
#include "ui_dialog.h"
#include <QDateTime>
#include <QSerialPortInfo>
#include <QMessageBox>
#include "dlgauto.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("TrimMotorTest V2.5"));

    m_btnStatus.push_back(ui->btnStatus0);
    m_btnStatus.push_back(ui->btnStatus1);
    m_btnStatus.push_back(ui->btnStatus2);
    m_btnStatus.push_back(ui->btnStatus3);
    m_btnStatus.push_back(ui->btnStatus4);
    m_btnStatus.push_back(ui->btnStatus5);
    m_btnStatus.push_back(ui->btnStatus6);
    m_btnStatus.push_back(ui->btnStatus7);

    ui->cmbCali->addItem("0: -limit");
    ui->cmbCali->addItem("1: center");
    ui->cmbCali->addItem("2: +limit");

    QObject::connect(&m_tmInit,&QTimer::timeout,this,&Dialog::OnTimeout);
    QObject::connect(&m_SerialThread,SIGNAL(OnError(QString)),this,SLOT(OnError(QString)));
    QObject::connect(&m_SerialThread,SIGNAL(OnCommuncation(bool,QByteArray)),this,SLOT(OnCommuncation(bool,QByteArray)));

    QObject::connect(&m_SerialThread,SIGNAL(OnStatus(char,char,char,char,char,float)),this,SLOT(OnStatus(char,char,char,char,char,float)));
    QObject::connect(&m_SerialThread,SIGNAL(OnPosition(float,float)),this,SLOT(OnPosition(float,float)));
    QObject::connect(&m_SerialThread,SIGNAL(OnTempVoltage(float,float)),this,SLOT(OnTempVoltage(float,float)));
    QObject::connect(&m_SerialThread,SIGNAL(OnLVDTDiff(float,float)),this,SLOT(OnLVDTDiff(float,float)));




    m_SerialThread.start();
    m_tmInit.start(1000);
}

Dialog::~Dialog()
{
    m_SerialThread.m_stop=true;

    m_SerialThread.requestInterruption();
    m_SerialThread.wait();
    delete ui;
}

void Dialog::EnumPorts()
{
    m_vSerialInfos.clear();
    ui->cmdPort->clear();
    m_vSerialInfos = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &serialPort : m_vSerialInfos)
    {
        ui->cmdPort->addItem(serialPort.portName());
    }

    ui->cmdBaud->addItem("1200");
    ui->cmdBaud->addItem("2400");
    ui->cmdBaud->addItem("4800");
    ui->cmdBaud->addItem("9600");
    ui->cmdBaud->addItem("19200");
    ui->cmdBaud->addItem("38400");
    ui->cmdBaud->addItem("57600");
    ui->cmdBaud->addItem("115200");
    ui->cmdBaud->setCurrentIndex(3);
}


void Dialog::ShowMessage(QString msg)
{
    QListWidgetItem* pItem;
    int nCount=ui->lstMessage->count();
    if(nCount>100)
    {
        pItem=ui->lstMessage->item(nCount-1);
        ui->lstMessage->removeItemWidget(pItem);
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QString message = QString("%1 %2").arg(currentTime.toString("yyyy-MM-dd hh:mm:ss")).arg(msg);

    pItem = new QListWidgetItem(message);
    ui->lstMessage->addItem(pItem);
    ui->lstMessage->scrollToBottom();
}

void Dialog::OnTimeout()
{
    EnumPorts();
    m_tmInit.stop();

}

void Dialog::OnEnumComPort(QStringList datas)
{
    ui->cmdPort->clear();
    for(const QString &port : datas)
    {
        ui->cmdPort->addItem(port);
    }
}



void Dialog::OnCommuncation(bool send, QByteArray data)
{
    QListWidgetItem* pItem;
    int nCount=ui->lstReceive->count();
    if(nCount>100)
    {
        pItem=ui->lstReceive->item(nCount-1);
        ui->lstReceive->removeItemWidget(pItem);
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QString message,msg=QString::fromLatin1(data,data.size());
    if(send)
        message = QString("%1 > %2").arg(currentTime.toString("yyyy-MM-dd hh:mm:ss")).arg(msg);
    else
        message = QString("%1 < %2").arg(currentTime.toString("yyyy-MM-dd hh:mm:ss")).arg(msg);

    pItem = new QListWidgetItem(message);
    ui->lstReceive->addItem(pItem);

    if(!ui->btnPause->isChecked())
        ui->lstReceive->scrollToBottom();
}


void Dialog::OnError(QString error)
{
    if(error.indexOf("PortOpen Error!")>=0)
        ui->btnPort->setEnabled(true);
    else if(error.indexOf("PortClose")>=0)
        ui->btnPort->setEnabled(true);

    ShowMessage(error);
}

void Dialog::OnStatus(char status,char sp,char tmout,char home,char stop,float fPosition)
{
    char mask;
    for(int i=0;i<8;i++)
    {
        mask=(0x1<<i);
        if((status & mask)!=0)
        {
            if(!m_btnStatus[i]->isChecked())
                m_btnStatus[i]->setChecked(true);
        }
        else
        {
            if(m_btnStatus[i]->isChecked())
                m_btnStatus[i]->setChecked(false);
        }
    }

    ui->lblSpeed_2->setText(QString("%1").arg(sp));
    ui->lblMoveTimeout_2->setText(QString("%1").arg(tmout));
    ui->lblHomeTimout_2->setText(QString("%1").arg(home));
    if(stop==1)
    {
        ui->btnRunning->setChecked(true);
        ui->btnRunning->setText("Running");
    }
    else
    {
        ui->btnRunning->setChecked(false);
        ui->btnRunning->setText("Stop");
    }



    QString stringValue = QString::number(fPosition, 'f', 2);
    ui->lcdPosition->display(stringValue);
}

void Dialog::OnPosition(float pos, float speed)
{
    QString stringValue = QString::number(speed, 'f', 2);
    ui->lcdSpeed->display(stringValue);

    stringValue = QString::number(pos, 'f', 2);
    ui->lcdRealPos->display(stringValue);
}

void Dialog::OnTempVoltage(float temp, float voltage)
{
    QString stringValue = QString::number(temp, 'f', 2);
    ui->lcdTemp->display(stringValue);
    stringValue = QString::number(voltage, 'f', 2);
    ui->lcdVoltage->display(stringValue);
}

void Dialog::OnLVDTDiff(float offset, float diff)
{
    QString stringValue = QString::number(offset, 'f', 2);
    ui->lblOffset->setText(QString("Offset:%1").arg(offset));

    //ui->lcdTemp->display(stringValue);
    //stringValue = QString::number(diff, 'f', 2);
    //ui->lcdSpeed->display(stringValue);
}

void Dialog::on_btnPort_clicked()
{
    QString strCOM=ui->cmdPort->currentText();
    int com=strCOM.indexOf("COM");
    int port=strCOM.right(strCOM.size()-3).toInt();
    if(m_SerialThread.Open(port,ui->cmdBaud->currentText().toInt()))
        ui->btnPort->setEnabled(false);
}

void Dialog::on_btnClose_clicked()
{
    m_SerialThread.Close();
    ui->btnPort->setEnabled(true);


}


void Dialog::on_btnAMove_clicked()
{
    bool bAMove=ui->rdoComm->isChecked();
    float fPos=ui->edtAMove->text().toFloat();
    if(!m_SerialThread.RunAMove(bAMove,fPos))
        QMessageBox::critical(this, tr("Error"),tr("RunFailed"), QMessageBox::Ok);
}

void Dialog::on_btnCali_clicked()
{
    char nPos=ui->cmbCali->currentIndex();
    float fPos=ui->edtCali->text().toFloat();
    if(!m_SerialThread.RunCali(nPos,fPos))
        QMessageBox::critical(this, tr("Error"),tr("RunFailed"), QMessageBox::Ok);
}

void Dialog::on_chkStatus_clicked()
{
    m_SerialThread.m_bReadStatus=ui->chkStatus->isChecked();
}



void Dialog::on_btnClear_clicked()
{
    ui->lstMessage->clear();
    ui->lstReceive->clear();
}




void Dialog::on_btnHome_clicked()
{
    //OnTempVoltage(1234.56,7890.12);
    //OnPosition(1234.56,7890.12);
    if(!m_SerialThread.RunHome())
        QMessageBox::critical(this, tr("Error"),tr("RunFailed"), QMessageBox::Ok);
}

void Dialog::on_btnAuto_clicked()
{
    dlgAuto* pAuto=new dlgAuto(&m_SerialThread,this);
    pAuto->setModal(true);
    pAuto->exec();
    delete pAuto;

}

void Dialog::on_btnOffset_clicked()
{
    float fDiff=ui->edtOffset->text().toFloat();
    if(!m_SerialThread.RunOffset(fDiff))
        QMessageBox::critical(this, tr("Error"),tr("RunOffset"), QMessageBox::Ok);
}

void Dialog::on_btnStop_clicked()
{
    if(!m_SerialThread.RunStopMotor())
        QMessageBox::critical(this, tr("Error"),tr("RunStopMotor"), QMessageBox::Ok);
}


void Dialog::on_btnSpeed_clicked()
{
    int speed=ui->lblSpeed->text().toInt();
    if(!m_SerialThread.RunSpeed(speed))
        QMessageBox::critical(this, tr("Error"),tr("RunFailed"), QMessageBox::Ok);
}


void Dialog::on_btnHomeTimeout_clicked()
{
    int speed=ui->lblHomeTimout->text().toInt();
    if(!m_SerialThread.RunSetHomeTimeout(speed))
            QMessageBox::critical(this, tr("Error"),tr("RunSetHomeTimeout"), QMessageBox::Ok);
}

void Dialog::on_btnMoveTimeout_clicked()
{
    int speed=ui->lblMoveTimeout->text().toInt();
    if(!m_SerialThread.RunSetMotorTimeout(speed))
            QMessageBox::critical(this, tr("Error"),tr("RunSetMotorTimeout"), QMessageBox::Ok);
}

void Dialog::on_btnC_clicked()
{

}

void Dialog::on_btnT_clicked()
{
    if(!m_SerialThread.RunTemVoltage())
        QMessageBox::critical(this, tr("Error"),tr("RunTemVoltage"), QMessageBox::Ok);
}


void Dialog::on_btnClear_2_clicked()
{
    QListWidgetItem* pItem;
    while(ui->lstReceive->count()>0)
    {
        pItem=ui->lstReceive->item(0);
        ui->lstReceive->removeItemWidget(pItem);
    }
}

void Dialog::on_horizontalSlider_valueChanged(int value)
{
    ui->lblSpeed->setText(QString("%1").arg(value));
}

void Dialog::on_hsHomeTimeout_valueChanged(int value)
{
    ui->lblHomeTimout->setText(QString("%1").arg(value));
}

void Dialog::on_hsMoveTimeout_valueChanged(int value)
{
    ui->lblMoveTimeout->setText(QString("%1").arg(value));
}
