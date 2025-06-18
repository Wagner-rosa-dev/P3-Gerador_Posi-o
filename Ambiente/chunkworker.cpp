#include "chunkworker.h" // Inclui o cabeçalho da classe ChunkWorker.
#include <QDebug>          // Para mensagens de depuração.
#include "terrainmanager.h" // Inclui o cabeçalho da classe terrainmanager.

/**
 * @brief Construtor da classe ChunkWorker.
 * @param chunkX Coordenada X do chunk a ser gerado.
 * @param chunkZ Coordenada Z do chunk a ser gerado.
 * @param resolution Resolução da malha a ser gerada.
 * @param config Ponteiro para a configuração do mundo.
 * @param manager Ponteiro para o terrainmanager que solicitou o trabalho.
 *
 * Inicializa os membros da classe com os parâmetros fornecidos e define
 * `setAutoDelete(true)` para que o objeto seja automaticamente liberado
 * pelo QThreadPool após a execução de `run()`.
 */
ChunkWorker::ChunkWorker(int chunkX, int chunkZ, int resolution, const WorldConfig* config, terrainmanager* manager):
    m_chunkX(chunkX), // Inicializa a coordenada X do chunk.
    m_chunkZ(chunkZ), // Inicializa a coordenada Z do chunk.
    m_resolution(resolution), // Inicializa a resolução da malha.
    m_config(config), // Inicializa o ponteiro para a configuração do mundo.
    m_manager(manager) // Inicializa o ponteiro para o terrainmanager.
{
    // A auto-destruição garante que o QRunnable seja deletado após a execução pelo QThreadPool.
    setAutoDelete(true);
}

/**
 * @brief O método principal de execução do ChunkWorker.
 *
 * Este método é executado em uma thread separada pelo QThreadPool.
 * Ele realiza o cálculo pesado de geração dos dados da malha do chunk
 * e, em seguida, envia esses dados de volta para a thread principal (via terrainmanager)
 * para que possam ser uploaded na GPU de forma segura.
 */
void ChunkWorker::run()
{
    // Realiza o cálculo pesado da malha do chunk.
    // A função `chunk::generateMeshData` é estática e não depende do estado de um objeto `chunk` específico,
    // o que a torna segura para ser chamada de uma thread de worker.
    chunk::MeshData generateData = chunk::generateMeshData(m_chunkX, m_chunkZ, m_resolution, m_config->chunkSize);

    // Devolve o resultado para a thread principal de forma segura.
    // Usamos `QMetaObject::invokeMethod` para chamar o slot `onMeshReady` do `m_manager`
    // a partir desta thread de worker.
    // `Qt::QueuedConnection` garante que a chamada do slot será enfileirada e executada
    // na thread onde `m_manager` vive (a thread principal), o que é essencial para
    // operações OpenGL (que precisam ser no contexto da thread principal).
    QMetaObject::invokeMethod(m_manager, "onMeshReady", Qt::QueuedConnection,
                              Q_ARG(int, m_chunkX),
                              Q_ARG(int, m_chunkZ),
                              Q_ARG(chunk::MeshData, generateData));
}
