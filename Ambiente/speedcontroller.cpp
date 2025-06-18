#include "speedcontroller.h" // Inclui o cabeçalho da classe SpeedController.
#include <QDebug>            // Para mensagens de depuração.
#include <QStringList>


double convertNmeaToDecimal(const QString& nmeaValue, const QString& hemisphere) {
    if (nmeaValue.isEmpty()) return 0.0;

    bool ok;
    double value = nmeaValue.toDouble(&ok);
    if (!ok) return 0.0;

    int degrees = static_cast<int>(value / 100.0);
    double minutes = value - (degrees * 100.0);

    double decimalDegrees = degrees + (minutes / 60.0);

    if (hemisphere == "S" || hemisphere == "W") {
        decimalDegrees *= -1.0;
    }
    return decimalDegrees;
}

/**
 * @brief Construtor da classe SpeedController.
 * @param parent O QObject pai deste controlador.
 *
 * Inicializa o objeto QSerialPort e conecta seus sinais `readyRead` e `errorOccurred`
 * aos slots internos correspondentes para lidar com dados recebidos e erros.
 */
SpeedController::SpeedController(QObject *parent) : QObject(parent),
    m_consecutiveInvalidFixes(0)

{
    m_serialPort = new QSerialPort(this); // Cria uma nova instância de QSerialPort.
    // Conecta o sinal `readyRead` (emitido quando há novos dados para ler) ao slot `handleReadyRead`.
    connect(m_serialPort, &QSerialPort::readyRead, this, &SpeedController::handleReadyRead);
    // Conecta o sinal `errorOccurred` (emitido quando ocorre um erro na porta serial) ao slot `handleError`.
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SpeedController::handleError);
}

/**
 * @brief Destrutor da classe SpeedController.
 *
 * Garante que a porta serial seja fechada se estiver aberta, liberando o recurso.
 */
SpeedController::~SpeedController()
{
    if (m_serialPort->isOpen()) { // Verifica se a porta serial está aberta.
        m_serialPort->close(); // Fecha a porta serial.
    }
}

/**
 * @brief Inicia a escuta de dados em uma porta serial específica.
 * @param portName O nome da porta serial a ser aberta (ex: "/dev/ttyUSB0", "COM1").
 *
 * Configura os parâmetros da porta serial (baud rate, data bits, paridade, stop bits, flow control)
 * e tenta abrir a porta para leitura. Registra mensagens de sucesso ou falha.
 */
void SpeedController::startListening(const QString &portName)
{
    m_serialPort->setPortName(portName); // Define o nome da porta serial.
    m_serialPort->setBaudRate(QSerialPort::Baud4800); // Define a taxa de transmissão para 115200 bps (compatível com ESP32).
    m_serialPort->setDataBits(QSerialPort::Data8); // Define 8 bits de dados por quadro.
    m_serialPort->setParity(QSerialPort::NoParity); // Define sem paridade.
    m_serialPort->setStopBits(QSerialPort::OneStop); // Define 1 stop bit.
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl); // Define sem controle de fluxo.

    if (m_serialPort->open(QIODevice::ReadOnly)) { // Tenta abrir a porta serial em modo somente leitura.
        qInfo() << "Controlador de velocidade conectado na porta:" << portName;
    } else {
        qWarning() << "Não foi possivel abrir a porta" << portName << ":" << m_serialPort->errorString();
    }
}

/**
 * @brief Manipula dados prontos para leitura na porta serial.
 *
 * Este slot é chamado sempre que há dados disponíveis na porta serial.
 * Ele lê as linhas, as divide por vírgula e tenta converter as partes
 * em valores de velocidade (float) e direção (int).
 */
void SpeedController::handleReadyRead()
{
    //adiciona os dados recebidos ao buffer
    m_serialBuffer.append(m_serialPort->readAll());

    //Processa o buffer linha por linha
    while (m_serialBuffer.contains('\n')) {
        int newlineIndex = m_serialBuffer.indexOf('\n');
        QByteArray line = m_serialBuffer.left(newlineIndex).trimmed();
        m_serialBuffer.remove(0, newlineIndex + 1);

        if (line.isEmpty()) continue;

        //tenta parsear a linha como uma mensagem NMEA
        QString nmeaSentence = QString::fromLatin1(line);
        QStringList parts = nmeaSentence.split(',');

        GpsData currentGpsData;
        currentGpsData.isValid = false; // por padrao, os dados nao soa validos

        // --- Parsing de mensagem NEMA ---
        if (parts.size() > 8) {
            QString setenceType = parts[0];

            if (setenceType.startsWith("$GPRMC")) { //
                if (parts.size() >= 13) {
                    QString status = parts[2]; // A = active, v = void

                    if (status == "A") { // dados validos
                        currentGpsData.isValid = true;
                        currentGpsData.latitude = convertNmeaToDecimal(parts[3], parts[4]);
                        currentGpsData.longitude = convertNmeaToDecimal(parts[5], parts[6]);
                        currentGpsData.speedKnots = parts[7].toFloat(); //velocidade em nos
                        currentGpsData.courseOverGround = parts[8].toFloat(); // rumo em graus

                         //timestamp(se necessario, pode ser mais complexo com data
                        //Qtime nmeaTime = QTime::fromString(parts[1].left(6), "hhmmss");
                        currentGpsData.timestamp = QDateTime::currentDateTime(); //tempo atual da placa por simplicidade
                    }
                }
            } else if (setenceType.startsWith("$GPGGA")) {
                if (parts.size() >= 15) {
                    int fixQuality = parts[6].toInt(); // 0 = no fix, 1 = gps Fix, 2 = FGPS Fix
                    currentGpsData.fixQuality = fixQuality;
                    currentGpsData.numSatellites = parts[7].toInt();

                    if (fixQuality >= 1) {//temos uma fixação GPS
                        currentGpsData.altitude = parts[9].toFloat(); // altitude em metros
                        // se GPRMC ao validou, GPGGA pode validar a posição
                        if (!currentGpsData.isValid) {
                            currentGpsData.isValid = true;
                            currentGpsData.latitude = convertNmeaToDecimal(parts[2], parts[3]);
                            currentGpsData.longitude = convertNmeaToDecimal(parts[4], parts[5]);
                            currentGpsData.timestamp = QDateTime::currentDateTime();
                        }
                    }
                }
            }
        }

        //Logica de contenção de spam emissão de sinais
        if (currentGpsData.isValid) {
            m_consecutiveInvalidFixes = 0; // reseta o contador
            //qInfo() << "GPS Data:" << currentGpsData.latitude << currentGpsData.longitude <<currenGpsData.speedKnots << currentGpsData.courseOverGround << currentGpsData.altitude;
            emit gpsDataUpdate(currentGpsData);

        } else {
            //qWarning() << "Invalid GPS data received or no recognizes NMEA setence:" << nmeaSentence;
        }
    }
}

/**
 * @brief Manipula erros ocorridos na porta serial.
 * @param error O código do erro ocorrido.
 *
 * Se um erro diferente de `NoError` ocorrer, uma mensagem de aviso é registrada.
 */
void SpeedController::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) { // Se o erro não for `NoError`.
        qWarning() << "Ocorreu um erro na prta serial:" << m_serialPort->errorString();
        // se o erro for serio, podemos resetar o contador de falhas ou fechar a porta
        m_consecutiveInvalidFixes = 0;
    }
}
