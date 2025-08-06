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

    QStringList parts = line.split(',');
    if (parts.isEmpty()) return;
    QString sentenceHeader = parts[0];

    // A sentença RMC marca o FIM de uma época e o INÍCIO da próxima.
    if (sentenceHeader.endsWith("RMC")) {
        // 1. A época anterior acabou. Se coletamos dados válidos, EMITA o sinal agora.
        if (m_buildingGpsData.isValid) {
            emit gpsDataUpdate(m_buildingGpsData);
        }

        // 2. Comece uma nova época de coleta, limpando os dados antigos.
        m_buildingGpsData = GpsData();

        // 3. Processe a sentença RMC atual para a *nova* época.
        if (parts.size() >= 13 && parts[2] == "A") {
            m_buildingGpsData.isValid = true;
            m_buildingGpsData.latitude = convertNmeaToDecimal(parts[3], parts[4]);
            m_buildingGpsData.longitude = convertNmeaToDecimal(parts[5], parts[6]);
            m_buildingGpsData.speedKnots = parts[7].toFloat();
            m_buildingGpsData.courseOverGround = parts[8].toFloat();
            // Adicione a extração de data/hora aqui se desejar um timestamp preciso
        }
    } else if (m_buildingGpsData.isValid) {
        // Se a época atual é válida, adicione informações de outras sentenças.
        if (sentenceHeader.endsWith("GGA") && parts.size() >= 9) {
            m_buildingGpsData.fixQuality = parts[6].toInt();
            m_buildingGpsData.numSatellites = parts[7].toInt();
            m_buildingGpsData.hdop = parts[8].toFloat();
            m_buildingGpsData.altitude = parts[9].toFloat();
        } else if (sentenceHeader.endsWith("GSA") && parts.size() >= 18) {
            m_buildingGpsData.usedSatellites.clear();
            for (int i = 3; i <= 14; ++i) {
                if (!parts[i].isEmpty()) {
                    m_buildingGpsData.usedSatellites.append(parts[i].toInt());
                }
            }
            m_buildingGpsData.gsa_hdop = parts[16].toFloat();
        } else if (sentenceHeader.endsWith("GSV") && parts.size() >= 8) {
            // A GSV pode vir em múltiplos pacotes. Esta lógica simples pega todos.
            for (int i = 4; i < parts.size() - 3; i += 4) {
                int satId = parts[i].toInt();
                int snr = parts[i+3].isEmpty() ? 0 : parts[i+3].toInt();
                if (satId > 0) {
                    m_buildingGpsData.satelliteSnr[satId] = snr;
                }
            }
        }
    }
}
