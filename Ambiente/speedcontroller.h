#ifndef SPEEDCONTROLLER_H
#define SPEEDCONTROLLER_H

#include <QObject>           // Classe base para o sistema de sinais/slots do Qt.
#include <QtSerialPort/QSerialPort> // Classe para comunicação com portas seriais.

// Classe: SpeedController
// Descrição: Esta classe é responsável por ler dados de velocidade e direção
//            recebidos através de uma porta serial (e.g., de um microcontrolador como ESP32).
//            Ela parseia os dados e emite sinais para outras partes da aplicação,
//            permitindo que a velocidade e direção do trator sejam controladas externamente.
class SpeedController : public QObject
{
    Q_OBJECT // Macro necessária para classes que usam sinais e slots do Qt.

public:
    // Construtor: SpeedController
    // Descrição: Inicializa o controlador de velocidade, configurando o objeto QSerialPort
    //            e conectando os slots internos para leitura e tratamento de erros.
    // Parâmetros:
    //   - parent: O objeto pai (QObject) deste controlador, para gerenciamento de memória.
    explicit SpeedController(QObject *parent = nullptr);

    // Destrutor: ~SpeedController
    // Descrição: Fecha a porta serial se estiver aberta e libera os recursos.
    ~SpeedController();

public slots:
    // Slot: startListening
    // Descrição: Inicia a comunicação e escuta de dados na porta serial especificada.
    //            Configura os parâmetros da porta (baud rate, data bits, etc.).
    // Parâmetros:
    //   - portName: O nome da porta serial a ser aberta (e.g., "/dev/ttyUSB0" no Linux, "COMx" no Windows).
    void startListening(const QString &portName);

signals:
    // Sinal: speedUpdate
    // Descrição: Emitido sempre que uma nova e válida leitura de velocidade é recebida da porta serial.
    // Parâmetros:
    //   - newSpeed: O valor da velocidade recém-lida (tipo float).
    void speedUpdate(float newSpeed);

    // Sinal: steeringUpdate
    // Descrição: Emitido sempre que um novo e válido valor de direção (esterçamento) é recebido da porta serial.
    // Parâmetros:
    //   - steeringValue: O valor da direção recém-lida (tipo int).
    void steeringUpdate(int steeringValue);

private slots:
    // Slot Privado: handleReadyRead
    // Descrição: Slot acionado pelo sinal `readyRead` do QSerialPort.
    //            Responsável por ler os dados disponíveis na porta serial,
    //            separá-los (velocidade e direção) e emitir os sinais `speedUpdate` e `steeringUpdate`.
    void handleReadyRead();

    // Slot Privado: handleError
    // Descrição: Slot acionado pelo sinal `errorOccurred` do QSerialPort.
    //            Lida com erros que podem ocorrer durante a comunicação serial,
    //            registrando mensagens de aviso.
    // Parâmetros:
    //   - error: O código do erro ocorrido na porta serial.
    void handleError(QSerialPort::SerialPortError error);

private:
    // Membro: m_serialPort
    // Tipo: QSerialPort*
    // Descrição: Ponteiro para o objeto QSerialPort que gerencia a comunicação serial.
    QSerialPort *m_serialPort;
};

#endif // SPEEDCONTROLLER_H
