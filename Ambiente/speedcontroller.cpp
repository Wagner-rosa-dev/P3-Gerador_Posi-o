#include "speedcontroller.h"
#include <QDebug>

SpeedController::SpeedController(QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::readyRead, this, &SpeedController::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SpeedController::handleError);
}

SpeedController::~SpeedController()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

void SpeedController::startListening(const QString &portName)
{
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud115200); //Mesma velocidade do ESP32
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadOnly)) {
        qInfo() << "Controlador de velocidade conectado na porta:" << portName;
    } else {
        qWarning() << "Não foi possivel abrir a porta" << portName << ":" << m_serialPort->errorString();
    }
}

void SpeedController::handleReadyRead()
{
    //Le todo os dados disponiveis, que podem conter multiplas linhas
    while(m_serialPort->canReadLine()) {
        QByteArray line = m_serialPort->readLine().trimmed(); //.trimmed() remove \n, \r etc
        if (!line.isEmpty()) {
            bool ok;
            float speed = line.toFloat(&ok);
            if (ok) {
                emit speedUpdate(speed); //emite o sinal com o novo valor
            }
        }
    }
}

void SpeedController::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        qWarning() << "Ocorreu um erro na prta serial:" << m_serialPort->errorString();
    }
}
