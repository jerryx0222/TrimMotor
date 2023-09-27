#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QReadWriteLock>
#include <QElapsedTimer>

enum COMSTEP
{
    stepIdle,
    stepOpen,
    stepWaitCmd,
    stepReadStatus,
    stepCheckStatus,

    stepRead,
    stepWrite,
    stepError,
    stepClose,
};


struct CMD
{
    char commadn;
    char type;
    QByteArray byData;
};

class SerialThread : public QThread
{
    Q_OBJECT
public:
    explicit SerialThread(QObject *parent = nullptr);
    ~SerialThread();

    void run();


    bool Open(int port,int baud);
    void Close();

    bool RunAMove(bool,float);
    bool RunCali(char,float);
    bool RunSpeed(int);
    bool RunHome();
    bool RunTemVoltage();
    bool RunLVDT();
    bool RunOffset(float);

    bool RunSetHomeTimeout(int sec);
    bool RunSetMotorTimeout(int sec);
    bool RunStopMotor();

    void ResetCount(bool);

public:
    bool    m_stop;
    bool    m_bReadStatus;

private:
    QSerialPort m_SerialPort;
    int     m_Step;
    int     m_nPort;
    int     m_nBaudRate;
    QByteArray m_ReceivedData;
    QByteArray m_Status;

    QElapsedTimer           m_Timer;
    QElapsedTimer           *m_ptmStatus;

    QReadWriteLock          m_lockCommand;
    std::list<CMD*>         m_lstCommands;

    std::list<CMD>          m_lstStatus;
    std::list<CMD>::iterator m_itCmd;

public:
    uint                    m_Count;
    double                  m_unitMMV;

private:
    bool SendCommand();
    bool ReceiveData();
    bool CheckReceiveData();

private:
    void InitCommands();
    void Trans2ByteData(QByteArray& in,char& out);
    void Trans2ShortData(QByteArray& in,short& out);

public:
    void TransReal2Float(QByteArray& in,float& out);
    void TransFloat2Byte(float in,QByteArray& out);
    void TransFloat2Byte(float in,QString& out);

public:
    int  GetCheckSum(QByteArray,int,QString&);
    int  GetCheckSum(QByteArray,int);
    bool CheckCheckSum(QByteArray);

signals:
    void OnCommuncation(bool send,QByteArray);
    void OnError(QString);
    void OnStatus(char,char,char,char,char,float);
    void OnPosition(float,float);
    void OnTempVoltage(float,float);
    void OnLVDTDiff(float,float);
    void OnCount(uint);

public slots:

};

#endif // SERIALTHREAD_H
