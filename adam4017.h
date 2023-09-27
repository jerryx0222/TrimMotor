#ifndef ADAM4017_H
#define ADAM4017_H

#include <QObject>
#include <QThread>
#include <QSerialPort>

class ADAM4017 : public QThread
{
    Q_OBJECT
public:
    explicit ADAM4017(QObject *parent = nullptr);
     ~ADAM4017();

signals:
    void OnCommuncation(bool send,QByteArray);
    void OnError(QString);

public slots:


public:
    void run();

    bool Open(int port,int baud);
    void Close();

private:
    void InitCommands();
    bool SendCommand();
    bool ReceiveData();
    bool CheckReceiveData();

private:
    QSerialPort m_SerialPort;
    int     m_Step;
    int     m_nPort;
    int     m_nBaudRate;
     bool    m_stop;

};

#endif // ADAM4017_H
