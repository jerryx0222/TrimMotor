#include "dlgauto.h"
#include "ui_dlgauto.h"
#include "xlsx/xlsxdocument.h"
#include "xlsx/xlsxcellreference.h"

dlgAuto::dlgAuto(SerialThread* pSer,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgAuto)
{
    m_pTmCycle=nullptr;
    m_dblNowPos=-999;
    m_bInitional=false;

    ui->setupUi(this);
    ui->lineEdit->setText("0.1");
    m_pSerial=pSer;

    QObject::connect(m_pSerial,SIGNAL(OnCount(uint)),this,SLOT(OnCount(uint)));
    QObject::connect(m_pSerial,SIGNAL(OnStatus(char,char,char,char,char,float)),this,SLOT(OnStatus(char,char,char,char,char,float)));
    QObject::connect(m_pSerial,SIGNAL(OnPosition(float,float)),this,SLOT(OnPosition(float,float)));


    OnCount(m_pSerial->m_Count);
    OnShowTable(true);
}

dlgAuto::~dlgAuto()
{
    if(m_pTmCycle!=nullptr)
        delete m_pTmCycle;
    delete ui;
}

void dlgAuto::on_btnExit_clicked()
{


    emit this->close();
}

void dlgAuto::on_btnReset_clicked()
{
    m_pSerial->ResetCount(true);
}

void dlgAuto::OnCount(uint count)
{
    QString strCount=QString("Count:%1").arg(count);
    ui->label->setText(strCount);
}

void dlgAuto::on_btnAMove_clicked()
{
    if(m_pTmCycle!=nullptr)
        return;
    m_pTmCycle=new QTimer();
    connect(m_pTmCycle,SIGNAL(timeout()),this,SLOT(OnTimeout()));
    m_Step=stepAMovP;
    m_pSerial->ResetCount(true);

    m_pTmCycle->start(50);
    ui->btnRMove->setEnabled(false);
}

void dlgAuto::on_btnRMove_clicked()
{
    double dblPitch=ui->lineEdit->text().toDouble();
    if(dblPitch<=0)
        return;
    if(m_pTmCycle!=nullptr)
        return;

    m_pTmCycle=new QTimer(this);
    connect(m_pTmCycle,SIGNAL(timeout()),this,SLOT(OnTimeout()));

    m_Step=stepGetPos;
    m_dblPitch=dblPitch;
     m_dblNowPos=-999;
    m_pSerial->ResetCount(true);

    m_pTmCycle->start(50);
    ui->btnAMove->setEnabled(false);
}

void dlgAuto::on_btnStop_clicked()
{
    if(m_pTmCycle!=nullptr)
        m_Step=stepStopMotor;


}

void dlgAuto::OnTimeout()
{
    if(ui->btnPause->isChecked())
        return;

    if(m_Step==stepStopMotor)
    {
        if(m_pSerial->RunStopMotor())
            m_Step=stepStopOK;
    }
    else if(m_Step==stepStopOK)
    {
        if(!m_bError)
        {
            ui->btnRMove->setEnabled(true);
            ui->btnAMove->setEnabled(true);
            m_Step=stepIdle;
        }
    }
    else
    {
        if(ui->btnAMove->isEnabled() && !ui->btnRMove->isEnabled())
            ACycle();
        else if(!ui->btnAMove->isEnabled() && ui->btnRMove->isEnabled())
            RCycle();
    }
}

void dlgAuto::OnStatus(char status,char sp,char tmout,char home,char stop,float fPosition)
{
    m_bInN=((status & 0x1)!=0);
    m_bInP=((status & 0x2)!=0);
    m_bError=((status & 0x80)!=0);
    //m_dblNowPos=fPosition;
}

void dlgAuto::OnPosition(float, float fPos)
{
    m_dblNowPos=fPos;
}

void dlgAuto::ACycle()
{
    switch(m_Step)
    {
    case stepAMovP:
        if(m_pSerial->RunAMove(true,9))
        {
            //m_pSerial->m_bReadStatus=true;
            m_bInN=false;
            m_Step=stepAMovPLim;
        }
        break;
    case stepAMovPLim:
        if(m_bInN)
        {
            if(m_pSerial->RunAMove(true,0))
            {
                m_bInP=false;
                m_pSerial->ResetCount(false);
                m_Step=stepAMovN;
            }
        }
        break;
    case stepAMovN:
        if(m_bInP)
        {
            m_pSerial->ResetCount(false);
            m_Step=stepAMovP;
        }
        break;
    }
}

void dlgAuto::RCycle()
{
    switch(m_Step)
    {
    case stepGetPos:
        //m_pSerial->m_bReadStatus=true;
        m_bInP=false;
        m_bInN=false;
        m_bError=false;
        m_dblNowPos=-999;
        m_Step=stepRMovePitch;
        break;
    case stepRMovePitch:
        if(abs(m_dblNowPos)>900)
            break;
        m_dblTarget=m_dblNowPos+m_dblPitch;
        if(m_pSerial->RunAMove(true,m_dblTarget))
        {
            m_bInP=false;
            m_bInN=false;
            m_bError=false;
            m_dblNowPos=-999;
            m_Step=stepCheckLim;
        }
        break;
    case stepCheckLim:
        if(abs(m_dblNowPos)>900)
            break;
        if(m_bInP || m_bInN)
        {
            m_dblPitch=-1*m_dblPitch;
            m_pSerial->ResetCount(false);
            m_Step=stepRMovePitch;
        }
        else if(!m_bError)
        {
            m_pSerial->ResetCount(false);
            m_Step=stepRMovePitch;
        }
        break;
    }
}

void dlgAuto::on_btnPause_clicked()
{

}


void dlgAuto::OnShowTable(bool bShow)
{
    if(!m_bInitional)
    {
        m_bInitional=true;

        ui->vCustomPlot->addGraph();
        ui->vCustomPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
        ui->vCustomPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue

        ui->vCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

        // x:速度(mm/s),y:轉矩(kg.cm)
        double dblMaxX=10,dblMinX=0;
        double dblMaxY=20,dblMinY=0;

        double rangeX=(dblMaxX-dblMinX)*0.05;
        double rangeY=(dblMaxY-dblMinY)*0.05;
        ui->vCustomPlot->yAxis->setRange(dblMinY-rangeY, dblMaxY+rangeY);
        ui->vCustomPlot->xAxis->setRange(dblMinX-rangeX, dblMaxX+rangeX);
        ui->vCustomPlot->graph(0)->rescaleAxes();
        ui->vCustomPlot->xAxis->setLabel("Speed(mm/s)");
        ui->vCustomPlot->yAxis->setLabel("Torque(kg.cm)");


        ui->vCustomPlot->addGraph();
        ui->vCustomPlot->graph(1)->setPen(QPen(Qt::red));
        ui->vCustomPlot->graph(1)->setBrush(QBrush(QColor(0, 255, 0, 20)));

        //on_btnLoadMeasure_clicked();

       // InitHeader();
    }

    /*
    if(bShow)
    {
        OnLanguageChange(0);
        m_tmTrigger.start(100);

        connect(m_pLSystem,SIGNAL(OnLaserLive(PoltDatas*)),this,SLOT(OnLive(PoltDatas*)));
        connect(m_pLSystem,SIGNAL(OnTriggerLaser(int)),this,SLOT(OnTriggerLaser(int)));
        connect(m_pLSystem,SIGNAL(OnTriggerCount(int)),this,SLOT(OnTriggerCount(int)));
    }
    else
    {
        m_tmTrigger.stop();

        m_pLSystem->Stop();
        ui->btnLive->setChecked(false);
        ui->btnTrigger->setChecked(false);

        disconnect(m_pLSystem,SIGNAL(OnLaserLive(PoltDatas*)),this,SLOT(OnLive(PoltDatas*)));
        disconnect(m_pLSystem,SIGNAL(OnTriggerLaser(int)),this,SLOT(OnTriggerLaser(int)));
        disconnect(m_pLSystem,SIGNAL(OnTriggerCount(int)),this,SLOT(OnTriggerCount(int)));
    }
    */
}


void dlgAuto::DrawLiveData(int id,QVector<double> x, QVector<double> y)
{
    if(id>=ui->vCustomPlot->graphCount() || id<0)
        return;
    ui->vCustomPlot->graph(id)->setData(x,y);
    ui->vCustomPlot->replot();
}

void dlgAuto::on_btnPort_clicked()
{

    std::random_device rd; // 獲取一個隨機設備
    std::mt19937 gen(rd()); // 使用Mersenne Twister引擎初始化

    std::uniform_real_distribution<> disX(0, 10.0f); // 定義範圍
    std::uniform_real_distribution<> disY(0, 20.0f); // 定義範圍

    m_lockXY.lockForWrite();
    m_vX.clear();
    m_vY.clear();
    for(int i=0;i<100;i++)
    {
        m_vX.push_back(disX(gen));
        m_vY.push_back(disY(gen));
    }

    DrawLiveData(0,m_vX,m_vY);
    m_lockXY.unlock();
}

void dlgAuto::on_btnSaveJpg_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "SaveJPG", "", "Image(*.jpg);;All (*)");
    bool bOK=false;
    if (!filePath.isEmpty())
    {
        bOK=ui->vCustomPlot->saveJpg(filePath);
    }
    if(!bOK)
        QMessageBox::critical(nullptr, "Warming", QString("SaveFailed:"), QMessageBox::Ok);
}

void dlgAuto::on_btnSaveXLSX_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Export File"),
        "",
        tr("Excel Files (*.XLSX *.xlsx);;All Files (*)"));
    int ret=-99;
    if (!filePath.isEmpty())
    {
        ret=ExportExcel(filePath);
    }
    if(ret!=0)
        QMessageBox::critical(nullptr, "Warming", QString("SaveFailed:"), QMessageBox::Ok);
}



int  dlgAuto::ExportExcel(QString strFile)
{
    int ret=0;

    if(!m_lockXY.tryLockForWrite())
        return -2;

    if(m_vY.size()<=0 || m_vX.size()<=0 || m_vY.size()!=m_vX.size())
    {
        m_lockXY.unlock();
        return -3;
    }

    // 創建一個QXlsx::Document對象
    QXlsx::Document xlsx;

    // 在工作表中寫入數據
    QTableWidgetItem* pItem;
    QXlsx::Format format;
    int nRow=m_vY.size();
    int nCol=3;

    format.setFontColor(Qt::red);
    QXlsx::CellReference cell1(1,1);
    xlsx.write(cell1,"");
    QXlsx::CellReference cellX(1,2);
    xlsx.write(cellX, ui->vCustomPlot->xAxis->label());
    QXlsx::CellReference cellY(1,3);
    xlsx.write(cellY, ui->vCustomPlot->yAxis->label());

    for(int i=0;i<nRow;i++)
    {
        QXlsx::CellReference cId(i+2,1);
        xlsx.write(cId,i+1);

        QXlsx::CellReference cX(i+2,2);
        xlsx.write(cX,m_vY[i]);

        QXlsx::CellReference cY(i+2,3);
        xlsx.write(cY,m_vY[i]);

    }

    // 保存Excel檔案
    xlsx.saveAs(strFile);

    m_lockXY.unlock();
    return ret;
}

