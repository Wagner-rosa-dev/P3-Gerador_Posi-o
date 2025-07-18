// Ambiente/gpsfileplayer.cpp
#include "gpsfileplayer.h"
#include "logger.h" // Para as funções de log (MY_LOG_INFO, MY_LOG_DEBUG, etc.)
#include <QStringList> // Para dividir as linhas NMEA
#include <memory>

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

    int calculateNmeaChecksum(const QString& sentence) {
        int checksum = 0;
        for (QChar c : sentence) {
            checksum ^= c.toLatin1();
        }
        return checksum;
    }
}
// ---------------------------------------------


// Construtor: GpsFilePlayer
// Descrição: Inicializa os membros da classe e conecta o timer.
GpsFilePlayer::GpsFilePlayer(QObject *parent) :
    QObject(parent)
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

    m_file = std::make_unique<QFile>(filePath);
    if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        MY_LOG_ERROR("GpsFilePlayer", QString("Não foi possível abrir o arquivo de log GPS: %1. Erro: %2")
                                          .arg(filePath)
                                          .arg(m_file->errorString()));
        m_file.reset();
        return;
    }

    m_textStream = std::make_unique<QTextStream>(m_file.get());
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
        m_textStream = nullptr;
    }

    if (m_file) {
        m_file->close();
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


    GpsData currentGpsData;
    currentGpsData.isValid = false; // Por padrão, os dados não são válidos

    //validação de checksum NMEA

    int checksumIndex = line.lastIndexOf('*');
    if (checksumIndex == -1 || checksumIndex + 3 > line.length()) { //verifica se * existe e ha 2 digitos após ele
        MY_LOG_WARNING("GpsFilePlayer_parse", QString("Sentença NMEA invalida (checksum ausente ou incompleto): %1").arg(line));
        return;
    }

    QString nmeaMessage = line.mid(1, checksumIndex - 1); //Extrai a mensagem entre $ e *
    QString receivedChecksumStr = line.mid(checksumIndex + 1, 2); //extrai o checksum recebido
    bool ok;
    int receivedChecksum = receivedChecksumStr.toInt(&ok, 16); //Converte para inteiro hexadeciaml

    if (!ok || calculateNmeaChecksum(nmeaMessage) != receivedChecksum) {
        MY_LOG_WARNING("GpsFilePlayer_Parse", QString("Checksum NMEA invalido para sentença: %1. Recebido: %2, Calculado: %3")
                                                  .arg(line)
                                                  .arg(receivedChecksumStr)
                                                  .arg(calculateNmeaChecksum(nmeaMessage), 2, 16, QChar('0')));

        return;
    }

    // Lógica de Parsing NMEA (adaptada de SpeedController::handleReadyRead)
    QStringList parts = line.split(',');

    if (parts.size() > 0) {
        QString sentenceHeader = parts[0]; // Ex: "$GPRMC" ou "$GNRMC"

        QDateTime messageDateTime = QDateTime::currentDateTime();

        if (parts.size() > 1 && !parts[1].isEmpty()) {
            QTime nmeaTime = QTime::fromString(parts[1].left(6), "hhmmss.zz");
            if (nmeaTime.isValid()) {
                messageDateTime.setTime(nmeaTime);
            }
        }

        if (sentenceHeader.endsWith("RMC") && parts.size() >= 10 && !parts[9].isEmpty()) {
            QDate nmeaDate = QDate::fromString(parts[9], "ddMMyy");
            if(nmeaDate.isValid()) {
                messageDateTime.setDate(nmeaDate);
            }
        }
        currentGpsData.timestamp = messageDateTime;

        if (sentenceHeader.endsWith("RMC")) { // Verifica se termina com "RMC" (para GPRMC, GNRMC, etc.)
            if (parts.size() >= 11) {

                QString status = parts[2]; // A = active, V = void

                if (status == "A") { // Dados válidos
                    currentGpsData.isValid = true;
                    currentGpsData.latitude = convertNmeaToDecimal(parts[3], parts[4]);
                    currentGpsData.longitude = convertNmeaToDecimal(parts[5], parts[6]);
                    currentGpsData.speedKnots = parts[7].toFloat(); // Velocidade em nós
                    currentGpsData.courseOverGround = parts[8].toFloat(); // Rumo em graus

                    MY_LOG_DEBUG("GpsFilePlayer_Parsed", QString("RMC Parseado - Lat:%1 Lon:%2 Vel(nos):%3 Rumo:%4")
                                                             .arg(currentGpsData.latitude, 0, 'f', 6)
                                                             .arg(currentGpsData.longitude, 0, 'f', 6)
                                                             .arg(currentGpsData.speedKnots, 0, 'f', 2)
                                                             .arg(currentGpsData.courseOverGround, 0, 'f', 2));
                }
            }
        } else if (sentenceHeader.endsWith("GGA")) { // Verifica se termina com "GGA" (para GPGGA, GNGGA, etc.)
            if (parts.size() >= 14) {
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
    }
}
