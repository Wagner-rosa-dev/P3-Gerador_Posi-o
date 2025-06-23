#include "mainwindow.h" // Inclui o cabeçalho da classe MainWindow, que é a janela principal da aplicação.
#include <QSurfaceFormat> // Inclui QSurfaceFormat para configurar o formato da superfície de renderização OpenGL.
#include <QApplication> // Inclui QApplication, a classe que gerencia o loop de eventos da aplicação Qt.
#include <QDebug> // Inclui QDebug para mensagens de depuração (qInfo, qWarning, etc.).
#include "chunk.h" // Inclui o cabeçalho da classe chunk, necessário para registrar o tipo MeshData.
#include "speedcontroller.h"
#include "logger.h"

/**
 * @brief main
 * @param argc Número de argumentos da linha de comando.
 * @param argv Vetor de strings de argumentos da linha de comando.
 * @return Código de saída da aplicação.
 *
 * Esta é a função principal (entry point) da aplicação.
 * Ela configura o ambiente Qt, define o formato da superfície OpenGL,
 * cria e exibe a janela principal e inicia o loop de eventos da aplicação.
 */
int main(int argc, char *argv[])
{
    // Cria uma instância da QApplication.
    // Esta linha é essencial para qualquer aplicação Qt, pois ela inicializa o sistema de eventos
    // e gerencia os recursos da aplicação.
    QApplication a(argc, argv);

    //Configuração do logger
    //Voce pode alterar essas linhas para controlar o comportamento do log:
    //Nivel minimo de log:
    //      Debug: para ver TUDO (ideal para depuração
    //      Info: Para ver informações gerais e importantes
    //      Warning/Erro/Critical: Para ver apenas problemas
    Logger::getInstance().setMinLevel(Debug);

    //Log para arquivo:
    //  true: Habilita o salvamento em "app_log.txt" (ou o caminho que voce especificar
    //  false: Desabilita o salvamente em arquivo
    Logger::getInstance().setLogToFile(true, "gps_monitor_log.txt"); //Salve em um arquivo especifico no diretorio de aplicação


    // Registra o tipo customizado 'chunk::MeshData' no sistema de metadados do Qt.
    // Isso é crucial para que o Qt possa passar objetos desse tipo através de conexões de sinais e slots
    // que cruzam limites de thread (Qt::QueuedConnection), como é o caso entre ChunkWorker e TerrainManager.
    qRegisterMetaType<chunk::MeshData>("chunk::MeshData");
    qRegisterMetaType<GpsData>("GpsData"); // registrar o noovo metatype

    // Cria e configura um objeto QSurfaceFormat.
    // O QSurfaceFormat define as propriedades desejadas para a superfície de renderização OpenGL,
    // como a versão do OpenGL/OpenGLES, tamanho do buffer de profundidade, etc.
    QSurfaceFormat format;
    // Define o tipo de renderização como OpenGLES.
    // Isso é importante para dispositivos embarcados como o Toradex i.MX8MP.
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    // Define a versão do OpenGL ES a ser utilizada (3.0).
    format.setVersion(3, 0);
    // Define o tamanho do buffer de profundidade em 24 bits.
    // O buffer de profundidade é usado para determinar quais fragmentos estão mais próximos da câmera,
    // garantindo que objetos mais próximos cubram os mais distantes.
    format.setDepthBufferSize(24);
    // Define o tamanho do buffer de estêncil em 8 bits.
    // O buffer de estêncil pode ser usado para efeitos especiais, como reflexos ou sombras.
    format.setStencilBufferSize(8);
    // Define o formato padrão para todas as novas superfícies OpenGL criadas na aplicação.
    QSurfaceFormat::setDefaultFormat(format);

    // Imprime informações de depuração sobre o formato da superfície configurado.
    MY_LOG_INFO("OpenGL", QString("Configurando QSurfaceFormat para OPENGL ES %1.%2")
                              .arg(format.majorVersion()).arg(format.minorVersion()));
    if (format.renderableType() == QSurfaceFormat::OpenGLES) {
        MY_LOG_INFO("OpenGL", "Tipo de renderização OpenGLES");
    } else {
        MY_LOG_WARNING("OpenGL", "Tipo de renderização Não é OpenGLES. Verifique a Configuração");
    }

    // Cria uma instância da janela principal da aplicação.
    MainWindow w;
    // Exibe a janela principal em tela cheia.
    w.showFullScreen();

    // Inicia o loop de eventos da aplicação Qt.
    // Esta linha passa o controle para o Qt, que então gerencia a interface do usuário,
    // processa eventos (cliques, teclado, timers, etc.) e mantém a aplicação em execução
    // até que QCoreApplication::exit() seja chamado ou a última janela seja fechada.
    return a.exec();
}
