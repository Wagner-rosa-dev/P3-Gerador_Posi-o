#include "chunkworker.h"
#include <QDebug>
#include "terrainmanager.h"

ChunkWorker::ChunkWorker(int chunkX, int chunkZ, int resolution, const WorldConfig* config, terrainmanager* manager):
    m_chunkX(chunkX),
    m_chunkZ(chunkZ),
    m_resolution(resolution),
    m_config(config),
    m_manager(manager)
{
    //a auto-destruição garante que o QRunnable seja deletado apos a execução
    setAutoDelete(true);
}

void ChunkWorker::run()
{
    //o mesmo calculo pesado de antes
    chunk::MeshData generateData = chunk::generateMeshData(m_chunkX, m_chunkZ, m_resolution, m_config->chunkSize);

    //devolve o resultado para a thread principal de forma segura
    //usamos o QMetaOBject::invokeMethod para chamar um slot do terrainmanager
    //a partir desta thread
    QMetaObject::invokeMethod(m_manager, "onMeshReady", Qt::QueuedConnection,
                              Q_ARG(int, m_chunkX),
                              Q_ARG(int, m_chunkZ),
                              Q_ARG(chunk::MeshData, generateData));
}

