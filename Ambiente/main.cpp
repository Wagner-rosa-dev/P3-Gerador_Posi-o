#include "mainwindow.h"
#include <QSurfaceFormat>
#include <QApplication>
#include <QDebug>
#include "chunk.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<chunk::MeshData>("chunk::MeshData");

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setVersion(3, 0);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    qInfo() << "Configurando QSurfaceFomat para OPENGL ES" << format.majorVersion() << "." << format.minorVersion();
    if (format.renderableType() == QSurfaceFormat::OpenGLES) {
        qInfo() << "Tipo de renderização OpenGLES";
    } else {
        qWarning() << "Tipo de renderização NÂO é OpenGLE. Verifique a Configurção";
    }
    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
#include "mainwindow.h" // Inclui o cabeçalho da classe MainWindow, que é a janela principal da aplicação.
#include <QSurfaceFormat> // Inclui QSurfaceFormat para configurar o formato da superfície de renderização OpenGL.
#include <QApplication> // Inclui QApplication, a classe que gerencia o loop de eventos da aplicação Qt.
#include <QDebug> // Inclui QDebug para mensagens de depuração (qInfo, qWarning, etc.).
#include "chunk.h" // Inclui o cabeçalho da classe chunk, necessário para registrar o tipo MeshData.

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

    // Registra o tipo customizado 'chunk::MeshData' no sistema de metadados do Qt.
    // Isso é crucial para que o Qt possa passar objetos desse tipo através de conexões de sinais e slots
    // que cruzam limites de thread (Qt::QueuedConnection), como é o caso entre ChunkWorker e TerrainManager.
    qRegisterMetaType<chunk::MeshData>("chunk::MeshData");

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
    qInfo() << "Configurando QSurfaceFomat para OPENGL ES" << format.majorVersion() << "." << format.minorVersion();
    // Verifica se o tipo de renderização configurado é realmente OpenGLES.
    if (format.renderableType() == QSurfaceFormat::OpenGLES) {
        qInfo() << "Tipo de renderização OpenGLES";
    } else {
        qWarning() << "Tipo de renderização NÂO é OpenGLE. Verifique a Configurção";
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
