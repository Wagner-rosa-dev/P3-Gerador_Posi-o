#include "logger.h"
#include <QCoreApplication> //Para QCoreApplication::applicationDirPath() ou nome do app

//Implementação do padrão Singleton para obter a unica instancia do Logger.
//Isso garante que haja apenas um objeto Logger em toda a aplicação
Logger& Logger::getInstance() {
    static Logger instance; //Garante que a instancia é criada uma vez e destruida no final do programa
    return instance;
}

//Construtor privado do Logger
//Inicializa os membros e configura o log para o console por padrão
Logger::Logger()
    : m_minLevel(Debug), //Por padrão, logs de debug para cima são exibidos
      m_logFile(nullptr),
      m_logStream(nullptr),
      m_logToFileEnabled(false)
{

}

//Destrutor do Logger
//Garante que o arquivo de log seja fechado corretamente
Logger::~Logger() {
    if (m_logStream) {
        m_logStream->flush(); //Garante que todos os dados pendentes sejam escritos
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

//Defini o nivel minimo de log a ser exibido
void Logger::setMinLevel(LogLevel level) {
    QMutexLocker locker(&m_mutex); //Protege o acesso multi-thread
    m_minLevel = level;
}

//Habilita/desabilita o log para um arquivo
void Logger::setLogToFile(bool enable, const QString& filePath) {
    QMutexLocker locker (&m_mutex); //Protege o acesso multi-thread

    if (m_logToFileEnabled == enable && (enable == false || (m_logFile && m_logFile->fileName() == filePath))) {
        //nada a fazer se o estado ja é o desejado
        return;
    }

    //Fecha o arquivo de log existente se houver
    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_logToFileEnabled = enable;
    if (enable) {
        //se o filePath estiver, usa um nome padrão
        QString finalPath = filePath.isEmpty() ? QCoreApplication::applicationDirPath() + "/app_log.txt" : filePath;
        m_logFile = new QFile(finalPath);
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            m_logStream = new QTextStream(m_logFile);
            qDebug() << "Logging para arquivo habilitado:" << finalPath;
        } else {
            qCritical() << "Erro ao abrir arquivo de log para escrita:" << m_logFile->errorString();
            m_logToFileEnabled = false; // Desabilita se falhou ao abrir
        }
    }
}

//Converte o nivel de log para uma string.
QString Logger::levelToString(LogLevel level) const {
    switch (level) {
        case Debug: return "DEBUG";
        case Info: return "INFO";
        case Warning: return "WARNING";
        case Error: return "ERROR";
        case Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

//Metodo principal para logar mensagens
//Formata e envia a mensagem para o console e, opcionalmente, para um arquivo
void Logger::log(LogLevel level, const QString& category, const QString& message, const char* file, int line, const char* function) {
    QMutexLocker locker(&m_mutex); //Protege o acesso multi-thread ao recurso de log

    if (level < m_minLevel) {
        return; // nao loga se o nivel for menor que o minimo configurado
    }

    //Formata a string de log: [TIMESTAMP] [NIVEL] [CATEGORIA] [ARQUIVO:LINHA::FUNÇÂO] - MENSAGEM
    QString logEntry = QString("[%1] [%2] [%3] [%4:%5::%6] - %7")
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                            .arg(levelToString(level))
                            .arg(category)
                            .arg(QFile(file).fileName()) //Apenas o nome do arquivo, não o caminhho completo
                            .arg(line)
                            .arg(function)
                            .arg(message);

//Envia para o console (QDebug)
//Usamos qCritical/qWarning/qInfo para que as mensagens apareçam na cor correta no Qt Creator
    if (level == Critical) {
        qCritical() << logEntry;
    } else if (level == Error) {
            qCritical() << logEntry; //qCritical para error graves
    } else if (level == Warning) {
        qWarning() << logEntry;
    } else {
        qInfo() << logEntry; // qInfo para Debug e Info
    }

    //Envia para o arquivo de log, se habilitando
    if (m_logToFileEnabled && m_logStream) {
        *m_logStream << logEntry << "\n";
        m_logStream->flush(); //Garante que a mensagem seja escrita imediatamente
    }
}
