#ifndef HTRIMMOTOR_H
#define HTRIMMOTOR_H

#include <QObject>
#include <QSerialPortInfo>

class HTrimMotor : public QObject
{
    Q_OBJECT
public:
    explicit HTrimMotor(QObject *parent = nullptr);






signals:
    void OnEnumComPort(QStringList);

public slots:


public:
    bool EnumPorts();

private:
    QList<QSerialPortInfo> m_serialPorts;

};

#endif // HTRIMMOTOR_H
