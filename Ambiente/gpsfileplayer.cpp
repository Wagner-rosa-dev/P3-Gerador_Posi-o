// Ambiente/gpsfileplayer.cpp
#include "gpsfileplayer.h"
#include "logger.h" // Para as funções de log (MY_LOG_INFO, MY_LOG_DEBUG, etc.)
#include <QStringList> // Para dividir as linhas NMEA

// --- FUNÇÃO AUXILIAR: convertNmeaToDecimal ---
// Esta função é movida de speedcontroller.cpp para ser reutilizada aqui.
// É uma função global (não um método de classe), por isso não está dentro de GpsFilePlayer::.
namespace {
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
}
// ---------------------------------------------


// Construtor: GpsFilePlayer
// Descrição: Inicializa os membros da classe e conecta o timer.
GpsFilePlayer::GpsFilePlayer(QObject *parent) :
    QObject(parent),
    m_file(nullptr),
    m_textStream(nullptr)
{
    // Conecta o sinal 'timeout' do timer ao slot 'processNextLine'.
    // Isso fará com que processNextLine seja chamado em intervalos regulares.
    connect(&m_playbackTimer, &QTimer::timeout, this, &GpsFilePlayer::processNextLine);
}

// Destrutor: ~GpsFilePlayer
// Descrição: Garante que o arquivo seja fechado e os recursos liberados.
GpsFilePlayer::~GpsFilePlayer()
{
    stopPlayback(); // Garante que tudo seja parado e limpo
}

// Slot: startPlayback
// Descrição: Inicia a reprodução dos dados GPS do arquivo.
// Parâmetros:
//   - filePath: O caminho para o arquivo de log NMEA.
//   - intervalMs: O intervalo de tempo (em ms) entre o envio de cada linha de dados.
void GpsFilePlayer::startPlayback(const QString &filePath, int intervalMs)
{
    stopPlayback(); // Para qualquer reprodução anterior

    m_file = new QFile(filePath);
    if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        MY_LOG_ERROR("GpsFilePlayer", QString("Não foi possível abrir o arquivo de log GPS: %1. Erro: %2")
                                          .arg(filePath)
                                          .arg(m_file->errorString()));
        delete m_file;
        m_file = nullptr;
        return;
    }

    m_textStream = new QTextStream(m_file);
    m_playbackTimer.start(intervalMs); // Inicia o timer com o intervalo desejado

    MY_LOG_INFO("GpsFilePlayer", QString("Iniciando reprodução do arquivo: %1 a cada %2 ms")
                                     .arg(filePath)
                                     .arg(intervalMs));
}

// Slot: stopPlayback
// Descrição: Para a reprodução dos dados do arquivo.
void GpsFilePlayer::stopPlayback()
{
    if (m_playbackTimer.isActive()) {
        m_playbackTimer.stop();
        MY_LOG_INFO("GpsFilePlayer", "Reprodução do arquivo GPS parada.");
    }

    if (m_textStream) {
        delete m_textStream;
        m_textStream = nullptr;
    }

    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

// Slot Privado: processNextLine
// Descrição: Lê a próxima linha do arquivo e processa a mensagem NMEA.
void GpsFilePlayer::processNextLine()
{
    if (!m_textStream || m_textStream->atEnd()) {
        MY_LOG_INFO("GpsFilePlayer", "Fim do arquivo de log GPS. Parando reprodução.");
        stopPlayback();
        emit playbackFinished(); // Sinaliza que a reprodução terminou
        return;
    }

    QString line = m_textStream->readLine().trimmed();
    if (line.isEmpty()) {
        // Se a linha estiver vazia, tente a próxima imediatamente para não atrasar
        QTimer::singleShot(0, this, &GpsFilePlayer::processNextLine);
        return;
    }

    MY_LOG_DEBUG("GpsFilePlayer", QString("Lendo linha: %1").arg(line));

    // Lógica de Parsing NMEA (adaptada de SpeedController::handleReadyRead)
    QStringList parts = line.split(',');

    GpsData currentGpsData;
    currentGpsData.isValid = false; // Por padrão, os dados não são válidos

    if (parts.size() > 8) {
        QString sentenceHeader = parts[0]; // Ex: "$GPRMC" ou "$GNRMC"

        if (sentenceHeader.endsWith("RMC")) { // Verifica se termina com "RMC" (para GPRMC, GNRMC, etc.)
            if (parts.size() >= 13) {
                QString status = parts[2]; // A = active, V = void

                if (status == "A") { // Dados válidos
                    currentGpsData.isValid = true;
                    currentGpsData.latitude = convertNmeaToDecimal(parts[3], parts[4]);
                    currentGpsData.longitude = convertNmeaToDecimal(parts[5], parts[6]);
                    currentGpsData.speedKnots = parts[7].toFloat(); // Velocidade em nós
                    currentGpsData.courseOverGround = parts[8].toFloat(); // Rumo em graus

                    // Usamos o tempo atual da placa para o timestamp, similar ao SpeedController
                    currentGpsData.timestamp = QDateTime::currentDateTime();

                    MY_LOG_DEBUG("GpsFilePlayer_Parsed", QString("RMC Parseado - Lat:%1 Lon:%2 Vel(nos):%3 Rumo:%4")
                                                             .arg(currentGpsData.latitude, 0, 'f', 6)
                                                             .arg(currentGpsData.longitude, 0, 'f', 6)
                                                             .arg(currentGpsData.speedKnots, 0, 'f', 2)
                                                             .arg(currentGpsData.courseOverGround, 0, 'f', 2));
                }
            }
        } else if (sentenceHeader.endsWith("GGA")) { // Verifica se termina com "GGA" (para GPGGA, GNGGA, etc.)
            if (parts.size() >= 15) {
                int fixQuality = parts[6].toInt(); // 0 = no fix, 1 = GPS Fix, 2 = DGPS Fix
                currentGpsData.fixQuality = fixQuality;
                currentGpsData.numSatellites = parts[7].toInt();

                if (fixQuality >= 1) { // Temos uma fixação GPS
                    currentGpsData.altitude = parts[9].toFloat(); // Altitude em metros
                    // Se RMC não validou, GGA pode validar a posição (mas RMC é preferido para velocidade/rumo)
                    if (!currentGpsData.isValid) {
                        currentGpsData.isValid = true;
                        currentGpsData.latitude = convertNmeaToDecimal(parts[2], parts[3]);
                        currentGpsData.longitude = convertNmeaToDecimal(parts[4], parts[5]);
                        currentGpsData.timestamp = QDateTime::currentDateTime();
                    }
                    MY_LOG_DEBUG("GpsFilePlayer_Parsed", QString("GGA Parseado - Alt:%1 Fix:%2 Sats:%3")
                                                             .arg(currentGpsData.altitude, 0, 'f', 2)
                                                             .arg(currentGpsData.fixQuality)
                                                             .arg(currentGpsData.numSatellites));
                }
            }
        }
    }

    if (currentGpsData.isValid) {
        emit gpsDataUpdate(currentGpsData); // Emite o sinal com os dados processados
    } else {
        MY_LOG_WARNING("GpsFilePlayer_Parsed", QString("Dados GPS inválidos ou sentença NMEA não reconhecida/válida: %1").arg(line));
        // Se a linha não for válida, processa a próxima imediatamente para não atrasar o playback
        QTimer::singleShot(0, this, &GpsFilePlayer::processNextLine);
    }
}
