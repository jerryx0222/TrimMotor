#include "htrimmotor.h"

HTrimMotor::HTrimMotor(QObject *parent) : QObject(parent)
{

}

bool HTrimMotor::EnumPorts()
{
    QStringList lstCom;
    m_serialPorts = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &serialPort : m_serialPorts)
    {
        lstCom.push_back(serialPort.portName());
    }
    if(m_serialPorts.size()>0)
    {
        emit OnEnumComPort(lstCom);
        return true;
    }
    return false;
}
