#include "serialthread.h"

SerialThread::SerialThread(QObject *parent)
    : QThread(parent)
{
    m_unitMMV=5.0915;   // mm/V

    m_Count=0;
    m_stop=false;
    m_Step=stepIdle;
    m_nPort=0;
    m_nBaudRate=9600;
    m_bReadStatus=false;
    m_ptmStatus=nullptr;
    InitCommands();
}

SerialThread::~SerialThread()
{
    if (isRunning())
    {
        m_stop = true;
        wait();
    }
}


void SerialThread::run()
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





bool SerialThread::Open(int port,int baud)
{
    m_nPort=port;
    m_nBaudRate=baud;
    return true;
}

void SerialThread::Close()
{
    m_nPort=-1;
}

bool SerialThread::RunAMove(bool bComm, float fPos)
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='S';
    if(bComm)
        pNew->commadn='1';
    else
        pNew->commadn='0';
    TransFloat2Byte(fPos,strPos);
    pNew->byData=QString("$S%1%2").arg(pNew->commadn).arg(strPos).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$S%1%2%3").arg(pNew->commadn).arg(strPos).arg(strCS).toLatin1();
    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunOffset(float off)
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='O';
    pNew->commadn='0';
    TransFloat2Byte(off,strPos);

    pNew->byData=QString("$O%1%2").arg(pNew->commadn).arg(strPos).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$O%1%2%3").arg(pNew->commadn).arg(strPos).arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}



bool SerialThread::RunCali(char nType, float fPos)
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='L';
    if(nType<=0)
        pNew->commadn='0';
    else if(nType>=2)
        pNew->commadn='2';
    else if(nType>10)
        pNew->commadn=nType-16+'A';
    else
        pNew->commadn=nType+'0';
    TransFloat2Byte(fPos,strPos);

    pNew->byData=QString("$L%1%2").arg(pNew->commadn).arg(strPos).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$L%1%2%3").arg(pNew->commadn).arg(strPos).arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunSpeed(int sp)
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='V';
    if(sp<=0)
        pNew->commadn='0';
    else if(sp>=9)
        pNew->commadn='9';
    else
        pNew->commadn=sp+'0';

    pNew->byData=QString("$V%1%2").arg(pNew->commadn).arg("00000000").toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$V%1%2%3").arg(pNew->commadn).arg("00000000").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunHome()
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='T';

    pNew->byData=QString("$T000000000").arg(pNew->commadn).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$T000000000%1").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunTemVoltage()
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='R';

    pNew->byData=QString("$R200000000").arg(pNew->commadn).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$R200000000%1").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunLVDT()
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='R';

    pNew->byData=QString("$R300000000").arg(pNew->commadn).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$R300000000%1").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}


bool SerialThread::RunSetHomeTimeout(int sec)
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='H';
    if(sec<=0)
        pNew->commadn='0';
    else if(sec>=9)
        pNew->commadn='9';
    else
        pNew->commadn=sec+'0';

    pNew->byData=QString("$H%1%2").arg(pNew->commadn).arg("00000000").toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$H%1%2%3").arg(pNew->commadn).arg("00000000").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunSetMotorTimeout(int sec)
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='B';
    if(sec<=0)
        pNew->commadn='0';
    else if(sec>=9)
        pNew->commadn='9';
    else
        pNew->commadn=sec+'0';

    pNew->byData=QString("$B%1%2").arg(pNew->commadn).arg("00000000").toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$B%1%2%3").arg(pNew->commadn).arg("00000000").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}

bool SerialThread::RunStopMotor()
{
    QString strPos,strCS="00";
    CMD* pNew;
    if(!m_SerialPort.isOpen())
        return false;

    m_lockCommand.lockForWrite();
    pNew=new CMD();
    pNew->type='E';

    pNew->byData=QString("$E100000000").arg(pNew->commadn).toLatin1();
    GetCheckSum(pNew->byData,pNew->byData.size(),strCS);
    strCS+="#";
    pNew->byData=QString("$E100000000%1").arg(strCS).toLatin1();

    m_lstCommands.push_back(pNew);
    m_lockCommand.unlock();
    return true;
}


void SerialThread::ResetCount(bool reset)
{
    if(reset)
        m_Count=0;
    else
        m_Count++;
    emit OnCount(m_Count);
}

bool SerialThread::SendCommand()
{
    CMD* pCmd=nullptr;
    if(m_lockCommand.tryLockForWrite())
    {
        if(m_lstCommands.size()>0)
        {
            pCmd=(*m_lstCommands.begin());
            m_lstCommands.erase(m_lstCommands.begin());
        }
        m_lockCommand.unlock();
    }

    if(pCmd!=nullptr)
    {
        m_SerialPort.write(pCmd->byData);
        m_SerialPort.waitForBytesWritten();
        emit OnCommuncation(true,pCmd->byData);
        delete pCmd;
    }
    else
    {
        if(m_itCmd!=m_lstStatus.end())
        {
            pCmd=&(*m_itCmd);
            if(m_ptmStatus==nullptr)
            {
                m_ptmStatus=new QElapsedTimer();
                m_ptmStatus->start();
            }

        }
        if(pCmd!=nullptr && m_ptmStatus!=nullptr && m_ptmStatus->elapsed()>1000)
        {
            if(m_bReadStatus)
            {
                m_SerialPort.write(pCmd->byData);
                m_SerialPort.waitForBytesWritten();
                emit OnCommuncation(true,pCmd->byData);
            }
            delete m_ptmStatus;
            m_ptmStatus=nullptr;

            m_itCmd++;
            if(!(m_itCmd!=m_lstStatus.end()))
                m_itCmd=m_lstStatus.begin();
        }
    }

    return true;
}





bool SerialThread::ReceiveData()
{
    int st,end;
    m_SerialPort.waitForReadyRead(10);
    QByteArray byData=m_SerialPort.readAll();
    if(byData.size()>0)
    {
        emit OnCommuncation(false,byData);
        m_ReceivedData+=byData;
        st=m_ReceivedData.indexOf('$');
        end=m_ReceivedData.indexOf('#',st);
        if(st>=0 && end>st)
        {
            m_ReceivedData=m_ReceivedData.mid(st,end-st+1);
            CheckReceiveData();
        }
        return true;
    }

    return false;
}

bool SerialThread::CheckReceiveData()
{
    QString strCSError;
    int nCS;
    int nCount=m_ReceivedData.size();
    if(nCount<5)
        return false;
    if(m_ReceivedData[0]!='$')
        return false;
    if(m_ReceivedData[nCount-1]!='#')
        return false;
    int cs=GetCheckSum(m_ReceivedData,nCount-3);

    char type,temp,cDatas[8];
    float fData,fTemp;
    QByteArray byTemp;
    temp=(cs>>4);
    (temp>=10)?(temp=temp-10+'A'):(temp+='0');
    if(m_ReceivedData[nCount-3]!=temp)
    {
        strCSError=QString("CheckSumError(0x%1)").arg(cs,2,16,QChar('0').toUpper());
        emit OnError(strCSError);
        strCSError=QString("Command:%1").arg(QString::fromLatin1(m_ReceivedData));
        emit OnError(strCSError);
        m_ReceivedData.clear();
        return false;
    }
    temp=(cs & 0xF);
    (temp>=10)?(temp=temp-10+'A'):(temp+='0');
    if(m_ReceivedData[nCount-2]!=temp)
    {
        strCSError=QString("CheckSumError(0x%1)").arg(cs,2,16,QChar('0').toUpper());
        emit OnError(strCSError);
        strCSError=QString("Command:%1").arg(QString::fromLatin1(m_ReceivedData));
        emit OnError(strCSError);
        m_ReceivedData.clear();
        return false;
    }

    if(m_ReceivedData[1]=='R')
    {
        if(m_ReceivedData.size()!=22)
        {
            strCSError=QString("DataSizeFailed(22/%1)").arg(m_ReceivedData.size());
            emit OnError(strCSError);
            m_ReceivedData.clear();
            return false;
        }

        type=m_ReceivedData[2];
        if(type=='0')
        {
            byTemp.clear();
            for(int i=0;i<2;i++)
                byTemp.push_back(m_ReceivedData[3+i]);
            Trans2ByteData(byTemp,temp);

            for(int i=0;i<4;i++)
                cDatas[i]=m_ReceivedData[5+i];

            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[11+i]);
            TransReal2Float(byTemp,fData);

            emit OnStatus(temp,cDatas[0],cDatas[1],cDatas[2],cDatas[3],fData);
            m_ReceivedData.clear();
            return true;
        }
        else if(type=='1')
        {
            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[3+i]);
            TransReal2Float(byTemp,fData);

            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[11+i]);
            TransReal2Float(byTemp,fTemp);

            emit OnPosition(fData,fTemp*m_unitMMV);
            m_ReceivedData.clear();
            return true;
        }
        else if(type=='2')
        {
            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[3+i]);
            TransReal2Float(byTemp,fData);

            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[11+i]);
            TransReal2Float(byTemp,fTemp);

            emit OnTempVoltage(fData,fTemp);
            m_ReceivedData.clear();
            return true;
        }
        else if(type=='3')
        {
            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[3+i]);
            TransReal2Float(byTemp,fData);

            byTemp.clear();
            for(int i=0;i<8;i++)
                byTemp.push_back(m_ReceivedData[11+i]);
            TransReal2Float(byTemp,fTemp);

            emit OnLVDTDiff(fData,fTemp);
            m_ReceivedData.clear();
            return true;
        }
        else
        {
            strCSError=QString("DataTypeFailed(%1)").arg(type);
            emit OnError(strCSError);
            m_ReceivedData.clear();
            return false;
        }

    }
    else if(m_ReceivedData[1]=='S' || m_ReceivedData[1]=='L' || m_ReceivedData[1]=='V' || m_ReceivedData[1]=='T')
    {
        if(m_ReceivedData.size()!=14)
        {
            strCSError=QString("DataSizeFailed(14/%1)").arg(m_ReceivedData.size());
            emit OnError(strCSError);
            m_ReceivedData.clear();
            return false;
        }
        m_ReceivedData.clear();
        return true;
    }
    else
    {
        QString strError=QString::fromLatin1(m_ReceivedData);
        strCSError=QString("CommandError(%1)").arg(strError);
        emit OnError(strCSError);
    }

    m_ReceivedData.clear();
    return false;
}

void SerialThread::InitCommands()
{
    int cs,temp;
    // Status
    CMD m_cmdStatus;
    m_cmdStatus.type='R';
    m_cmdStatus.commadn='0';
    m_cmdStatus.byData.push_back('$');
    m_cmdStatus.byData.push_back(m_cmdStatus.type);
    m_cmdStatus.byData.push_back(m_cmdStatus.commadn);
    for(int i=0;i<8;i++)
        m_cmdStatus.byData.push_back('0');
    cs=GetCheckSum(m_cmdStatus.byData,m_cmdStatus.byData.size());
    temp=(cs>>4);
    (temp>15)?(temp+='A'):(temp+='0');
    m_cmdStatus.byData.push_back(static_cast<char>(temp));
    temp=(cs & 0xF);
    (temp>15)?(temp+='A'):(temp+='0');
    m_cmdStatus.byData.push_back(static_cast<char>(temp));
    m_cmdStatus.byData.push_back('#');
    m_lstStatus.push_back(m_cmdStatus);

    // Position
    CMD m_cmdPosition;
    m_cmdPosition.type='R';
    m_cmdPosition.commadn='1';
    m_cmdPosition.byData.push_back('$');
    m_cmdPosition.byData.push_back(m_cmdPosition.type);
    m_cmdPosition.byData.push_back(m_cmdPosition.commadn);
    for(int i=0;i<8;i++)
        m_cmdPosition.byData.push_back('0');
    cs=GetCheckSum(m_cmdPosition.byData,m_cmdPosition.byData.size());
    temp=(cs>>4);
    (temp>15)?(temp+='A'):(temp+='0');
    m_cmdPosition.byData.push_back(static_cast<char>(temp));
    temp=(cs & 0xF);
    (temp>15)?(temp+='A'):(temp+='0');
    m_cmdPosition.byData.push_back(static_cast<char>(temp));
    m_cmdPosition.byData.push_back('#');
    m_lstStatus.push_back(m_cmdPosition);

    // 溫壓 ;
    CMD m_cmdTempVoltage;
    m_cmdTempVoltage.type='R';
    m_cmdTempVoltage.commadn='2';
    m_cmdTempVoltage.byData.push_back('$');
    m_cmdTempVoltage.byData.push_back(m_cmdTempVoltage.type);
    m_cmdTempVoltage.byData.push_back(m_cmdTempVoltage.commadn);
    for(int i=0;i<8;i++)
        m_cmdTempVoltage.byData.push_back('0');
    cs=GetCheckSum(m_cmdTempVoltage.byData,m_cmdTempVoltage.byData.size());
    temp=(cs>>4);
    (temp>15)?(temp+='A'):(temp+='0');
    m_cmdTempVoltage.byData.push_back(static_cast<char>(temp));
    temp=(cs & 0xF);
    (temp>15)?(temp+='A'):(temp+='0');
    m_cmdTempVoltage.byData.push_back(static_cast<char>(temp));
    m_cmdTempVoltage.byData.push_back('#');
    m_lstStatus.push_back(m_cmdTempVoltage);



    m_itCmd=m_lstStatus.begin();
}

void SerialThread::Trans2ByteData(QByteArray &in, char &out)
{
    unsigned int nTemp=0;
    char cTemp;
    if(in.size()!=2)
        return;

    cTemp=in[0];
    (cTemp>='A')?(nTemp=(cTemp-'A'+10)*16):(nTemp=(cTemp-'0')*16);
    cTemp=in[1];
    (cTemp>='A')?(nTemp+=(cTemp-'A'+10)):(nTemp+=(cTemp-'0'));

    out=static_cast<char>(nTemp);
}

void SerialThread::Trans2ShortData(QByteArray &in, short &out)
{
    unsigned long nData=0;
    unsigned int nTemp=0;
    char cTemp;
    if(in.size()!=4)
        return;
    for(int i=0;i<2;i++)
    {
        cTemp=in[2*i];
        (cTemp>='A')?(nTemp=(cTemp-'A')*16):(nTemp=(cTemp-'0')*16);
        cTemp=in[2*i+1];
        (cTemp>='A')?(nTemp+=(cTemp-'A')):(nTemp+=(cTemp-'0'));
        nTemp=(nTemp<<(8*i));
        nData+=nTemp;
    }
    out=static_cast<short>(nData);
}

/*
void SerialThread::TransReal2Float(QByteArray &in, float &out)
{
    unsigned long nData=0;
    unsigned int nTemp=0;
    char cTemp;
    if(in.size()!=8)
        return;
    for(int i=0;i<4;i++)
    {
        cTemp=in[2*i];
        (cTemp>='A')?(nTemp=(cTemp-'A')*16):(nTemp=(cTemp-'0')*16);
        cTemp=in[2*i+1];
        (cTemp>='A')?(nTemp+=(cTemp-'A')):(nTemp+=(cTemp-'0'));
        nTemp=(nTemp<<(8*i));
        nData+=nTemp;
    }
    float* pValue=(float*)(&nData);
    out=(*pValue);
}

void SerialThread::TransFloat2Byte(float in, QByteArray &out)
{
    unsigned long nTemp,nData;
    QString strTemp;
    int nOff;
    float* pDataIn=&in;
    unsigned long* pData=(unsigned long*)pDataIn;
    nData=(*pData);

    for(int i=0;i<4;i++)
    {
        nOff=(4*i);
        nTemp=nData & (0xFF<<nOff);
        strTemp=QString("%1").arg(nTemp>>nOff,2,16,QChar('0')).toUpper();
        QByteArray byteArray = strTemp.toLatin1();
        out.push_back(byteArray);
    }
}

void SerialThread::TransFloat2Byte(float in, QString &out)
{
    unsigned long nTemp,nData;
    QString strTemp;
    int nOff;
    float* pDataIn=&in;
    unsigned long* pData=(unsigned long*)pDataIn;
    nData=(*pData);

    for(int i=0;i<4;i++)
    {
        nOff=(8*i);
        nTemp=nData & (0xFF<<nOff);
        strTemp=QString("%1").arg(nTemp>>nOff,2,16,QChar('0')).toUpper();
        out+=strTemp;
    }
}
*/
void SerialThread::TransReal2Float(QByteArray &in, float &out)
{
    unsigned long nData[4],nValue=0;
    unsigned int nTemp;
    char cTemp;
    int nOff;
    if(in.size()!=8)
        return;
    for(int i=0;i<4;i++)
    {
        cTemp=in[2*i];
        (cTemp>='A')?(nTemp=(cTemp-'A'+10)*16):(nTemp=(cTemp-'0')*16);
        cTemp=in[2*i+1];
        (cTemp>='A')?(nTemp+=(cTemp-'A'+10)):(nTemp+=(cTemp-'0'));

        //nOff=(8*(3-i));
        nOff=(8*i);
        nData[i]=(nTemp<<nOff);
        nValue+=nData[i];
    }
    float* pValue=(float*)(&nValue);
    out=(*pValue);
}

void SerialThread::TransFloat2Byte(float in, QByteArray &out)
{
    float* floatPtr = new float(in);
    long* longPtr = reinterpret_cast<long*>(floatPtr);
    long nOff,nTemp,longValue = *longPtr;
    delete floatPtr;
    QString strTemp;
    out.clear();

    for(int i=0;i<4;i++)
    {
        nOff=(i<<3);
        nTemp=(0xFF<<nOff);
        nTemp=(longValue & nTemp)>>nOff;
        strTemp=QString("%1").arg(nTemp,2,16,QChar('0')).toUpper();
        QByteArray byteArray = strTemp.toLatin1();
        //out.push_front(byteArray);
        out.push_back(byteArray);
    }
}


void SerialThread::TransFloat2Byte(float in, QString &out)
{
    QByteArray byData;
    TransFloat2Byte(in,byData);
    out=QString::fromUtf8(byData);
}

int SerialThread::GetCheckSum(QByteArray datas, int count, QString& out)
{
    int ret=GetCheckSum(datas,count);
    QString strCS=QString("%1").arg(ret,2,16,QChar('0'));
    out=strCS.toUpper();
    return ret;
}

int SerialThread::GetCheckSum(QByteArray datas,int count)
{
    int ck;
    int nSize=datas.size();
    if(nSize<=0 || nSize<count)
        return -1;
    for(int i=0;i<count;i++)
    {
        if(i==0)
            ck = datas[0];
        else
            ck = ck ^ datas[i];
    }
    return ck;
}

bool SerialThread::CheckCheckSum(QByteArray datas)
{
    int nSize=datas.size();
    int cs=GetCheckSum(datas,nSize-3);
    if(cs<0)
        return false;

    int temp=datas[nSize-2];
    if(temp != (cs>>4))
        return false;

    temp=datas[nSize-1];
    if(temp != (cs & 0xF))
        return false;

    return true;
}

