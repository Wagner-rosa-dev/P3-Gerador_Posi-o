#ifndef SPEEDCONTROLLER_H
#define SPEEDCONTROLLER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>

class SpeedController : public QObject
{
    Q_OBJECT
public:
    explicit SpeedController(QObject *parent = nullptr);
    ~SpeedController();

public slots:
    //Inicia a escuta em uma porta serial especifica
    void startListening(const QString &portName);

signals:
    //Sinal emitido sempre que uma nova velocidade valida é recebida
    void speedUpdate(float newSpeed);
    void steeringUpdate(int steeringValue);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
};

#endif // SPEEDCONTROLLER_H
