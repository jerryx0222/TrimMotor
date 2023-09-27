#include "adam4017.h"

ADAM4017::ADAM4017(QObject *parent)
        : QThread(parent)
{
    m_stop=false;
    m_Step=0;
    m_nPort=0;
    m_nBaudRate=9600;

    InitCommands();
}

ADAM4017::~ADAM4017()
{
    if (isRunning())
    {
        m_stop = true;
        wait();
    }
}

void ADAM4017::run()
{
    while (!m_stop)
    {
        if(!m_SerialPort.isOpen())
        {
            if(m_nPort>0)
            {
                m_SerialPort.setPortName(QString("COM%1").arg(m_nPort));
                m_SerialPort.setBaudRate(m_nBaudRate);
                if (!m_SerialPort.open(QIODevice::ReadWrite))
                {
                    m_nPort=-1;
                    emit OnError("Failed to open serial port");
                }
                else
                {
                    emit OnError("open serial port success");
                }
            }
        }
        else
        {
            if(m_nPort>0)
            {
                SendCommand();
                ReceiveData();
            }
            else
            {
                m_SerialPort.close();
                emit OnError("serial port close");
            }
        }
    }

    m_SerialPort.close();
    emit OnError("serial port close");
}

bool ADAM4017::Open(int port, int baud)
{
    m_nPort=port;
    m_nBaudRate=baud;
    return true;
}

void ADAM4017::Close()
{
    m_nPort=-1;
}

void ADAM4017::InitCommands()
{

}

bool ADAM4017::SendCommand()
{
    return false;
}

bool ADAM4017::ReceiveData()
{
    return false;
}

bool ADAM4017::CheckReceiveData()
{
    return false;
}
