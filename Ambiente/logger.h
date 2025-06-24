#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>       // Para a saida padrão de debug(console)
#include <QDateTime>    // Para adicionar timestamp aos logs
#include <QString>      // Para manipular strings
#include <QFile>        // Para salvar logs em arquivos
#include <QTextStream>  // Para escrever texto em arquivo
#include <QMutex>       // Para garantir segurança em thread ao logar (importante)

//Enumeração dos niveis de log
//Quanto maior o nivel, mais grave a mensagem
enum LogLevel {
    Debug,      // Informações detalhadas para depuração
    Info,       // Mensagens informativas sobre o fluxo da aplicação
    Warning,    // Situação que podem indicar um problema, mas não um erro fatal
    Error,      // Erros que afetam a funcionalidade, mas a aplicação pode continuar
    Critical    // Erros graves que podem levar a falha da aplicação
};

//Classe Logger: Implementa o padrão Singleton para o acesso global
//Permite centrilizar a configuração e o gerenciamento dos logs
class Logger
{
public:
    //Retorna a unica instancia da classe logger (Singleton)
    //Thread-safe a partir do C++11, garantido pela inicialização estatica local
    static Logger& getInstance();

    //Define o nivel minimo de log a ser exibido
    //Mensagens com nivel menor que o minimo serão logadas
    void setMinLevel(LogLevel level);

    //Habilita ou desabilita o log para um arquivo
    //se "enable" for true, os logs serão salvos no "filePath"
    void setLogToFile(bool enable, const QString& filePath = "");

    //Metodo principal para logar uma mensagem
    //Recebe o nivel, categoria, mensagem, informações do arquivo/linha/função
    void log(LogLevel level, const QString& category, const QString& message, const char* file, int line, const char* function);

private:
    //Construtor privado para garantir que apenas a propria classe possa criar instancia (Singleton)
    Logger();
    //Destrutor privado. Fecha o arquivo de log se estiver aberto
    ~Logger();

    //Impede a cópia e atribuição de objetos Logger para manter o Singleton
    Q_DISABLE_COPY(Logger)

    LogLevel m_minLevel;        //Nivel minimo de log configurado
    QFile* m_logFile;           //Ponteiro para o arquivo de log
    QTextStream* m_logStream;   //Stream para escrita no arquivo de log
    bool m_logToFileEnabled;    //Flag para indicar se o log para o arquivo esta ativo
    QMutex m_mutex;              //Mutex para garantir segurança de thread ao acessar recursos de log

    //Retorna a string correspondente ao nivel de log
    QString levelToString(LogLevel level) const;
};

//-- Macros de log para uso facil no codigo--
//Elas automaticamente passam o nivel, categoria e informações de contexto (arquivo, linha função

//Log de depuração: mensagens detalhadas
#define MY_LOG_DEBUG(category, msg) \
    Logger::getInstance().log(Debug, category, msg, __FILE__, __LINE__, __FUNCTION__)

//Log de informação: Mensagens gerais de execução
#define MY_LOG_INFO(category, msg) \
    Logger::getInstance().log(Info, category, msg, __FILE__, __LINE__, __FUNCTION__)

//Log de aviso: Potenciais problemas
#define MY_LOG_WARNING(category, msg) \
    Logger::getInstance().log(Warning, category, msg, __FILE__, __LINE__, __FUNCTION__)

//Log de erro: para erros de execução
#define MY_LOG_ERROR(category, msg) \
    Logger::getInstance().log(Error, category, msg, __FILE__, __LINE__, __FUNCTION__)

//Log critico: erros graves
#define MY_LOG_CRITICAL(category, msg) \
    Logger::getInstance().log(Critical, category, msg, __FILE__, __LINE__, __FUNCTION__)

#endif // LOGGER_H
