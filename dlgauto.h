#ifndef DLGAUTO_H
#define DLGAUTO_H

#include <QDialog>
#include <QTimer>
#include "serialthread.h"

namespace Ui {
class dlgAuto;
}

class dlgAuto : public QDialog
{
    Q_OBJECT

public:
    explicit dlgAuto(SerialThread* pSer,QWidget *parent = nullptr);
    ~dlgAuto();

    enum STEP
    {
        stepIdle,

        stepAMovP,
        stepAMovPLim,
        stepAMovN,


        stepGetPos,
        stepRMovePitch,
        stepCheckLim,

        stepStopMotor,
        stepStopOK,

    };

private slots:
    void on_btnExit_clicked();
    void on_btnReset_clicked();
    void OnCount(uint);
    void on_btnAMove_clicked();
    void on_btnRMove_clicked();
    void on_btnStop_clicked();
    void OnTimeout();
    void OnStatus(char,char,char,char,char,float);
    void OnPosition(float,float);

    void on_btnPause_clicked();
    void OnShowTable(bool bShow);
    void DrawLiveData(int id,QVector<double> x, QVector<double> y);

    void on_btnPort_clicked();

    void on_btnSaveJpg_clicked();

    void on_btnSaveXLSX_clicked();

private:
    void ACycle();
    void RCycle();
    int ExportExcel(QString strFile);

private:
    Ui::dlgAuto *ui;
    SerialThread    *m_pSerial;
    QTimer          *m_pTmCycle;
    int             m_Step;
    double          m_dblNowPos,m_dblPitch,m_dblTarget;
    bool            m_bInP,m_bInN,m_bError;

    bool        m_bInitional;

    QReadWriteLock  m_lockXY;
    QVector<double> m_vX,m_vY;
};

#endif // DLGAUTO_H
