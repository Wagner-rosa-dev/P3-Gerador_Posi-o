#include "terrainmanager.h"
#include "chunkworker.h"
#include <QDebug>
#include <cmath>
#include <QThreadPool>
#include "worldconfig.h"

terrainmanager::terrainmanager() :
    QObject(nullptr),
    m_centerChunkX(0),
    m_centerChunkZ(0),
    m_lineQuadVaoRef(nullptr),
    m_lineQuadVboRef(nullptr),
    m_glFuncsRef(nullptr)
{
    //definimos o numero maximo de threads que queremos usar para gerar chunk
    // deixamos 1 nucleo livre para a thread princiapl e o sistema operacional
    QThreadPool::globalInstance()->setMaxThreadCount(3);
}

terrainmanager::~terrainmanager()
{
    //espera todos os trabalhos na piscina terminarem antes de fechar
    QThreadPool::globalInstance()->waitForDone();
}

void terrainmanager::init(const WorldConfig* config, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs) {
    m_config = config; // armazena o ponteiro para configuração
    m_lineQuadVaoRef = lineQuadVao;
    m_lineQuadVboRef = lineQuadVbo;
    m_glFuncsRef = glFuncs;

    m_chunks.resize(m_config->gridRenderSize);
    for (int i = 0; i < m_config->gridRenderSize; ++i) {
        m_chunks[i].resize(m_config->gridRenderSize);
    }
    //inicia a grade centrada em (0, 0)
    recenterGrid(0, 0);
}

void terrainmanager::update(const QVector3D& cameraPos) {
    // Verifica se o centro da grade precisa mudar (logica de terreno infinito)
    int cameraChunkX = static_cast<int>(std::floor(cameraPos.x() / m_config->chunkSize));
    int cameraChunkZ = static_cast<int>(std::floor(cameraPos.z() / m_config->chunkSize));

    if (cameraChunkX != m_centerChunkX || cameraChunkZ != m_centerChunkZ) {
        recenterGrid(cameraChunkX, cameraChunkZ);
    }
    //atualiza o nivel de detalhe (LOD) dos chunks existentes
    for (int i = 0; i < m_config->gridRenderSize; ++i) {
        for (int j = 0; j < m_config->gridRenderSize; ++j){
            chunk& currentChunk = m_chunks[i][j];
            int currentLOD = currentChunk.getLOD();
            int desiredLOD = currentLOD;

            float distanceToChunk = cameraPos.distanceToPoint(currentChunk.getCenterPosition(m_config->chunkSize));

            //logica de histerese
            if (currentLOD == 1 && distanceToChunk < m_config->lodDistanceThreshold - LOD_HYSTERESIS_BUFFER) {
                //se o chunk esta em baixa resolução, ele so muda para alta
                //se entrar bem na zona de alta resolução
                desiredLOD = 0;
            } else if (currentLOD == 0 && distanceToChunk > m_config->lodDistanceThreshold + LOD_HYSTERESIS_BUFFER) {
                //se o chunk esta em alta resolução, ele so muda para baixa
                //se sair bem da zona de alta resolução
                desiredLOD = 1;
            }

            if (currentLOD != desiredLOD) {
                currentChunk.setLOD(desiredLOD);
                int newRes = (desiredLOD == 0) ? m_config->highRes : m_config->lowRes;
                //cria um novo trabalho (QRunnable)
                ChunkWorker* worker = new ChunkWorker(currentChunk.chunkGridX(), currentChunk.chunkGridZ(), newRes, m_config, this);
                //submete o traalho a psicina de threads. o qt cuida do resto
                QThreadPool::globalInstance()->start(worker);
            }
        }
    }
}

void terrainmanager::recenterGrid(int newCenterX, int newCenterZ){
    qInfo() << "Recentering grid to:" << newCenterX << "," << newCenterZ;
    m_centerChunkX = newCenterX;
    m_centerChunkZ = newCenterZ;

    int halfGrid = m_config->gridRenderSize / 2;

    for (int i = 0; i < m_config->gridRenderSize; ++i) {
        for (int j = 0; j < m_config->gridRenderSize; ++j) {
            int chunkX = m_centerChunkX - halfGrid + i;
            int chunkZ = m_centerChunkZ - halfGrid + j;
            //Recicla o chunk na posição [i][j] da nossa matriz a nova coordenada
            m_chunks[i][j].recycle(chunkX, chunkZ, m_config->chunkSize);

            //dispara o sinal para gerar a malha deste chunk em segundo plano
            //a logica de lod pode ser mais complexa aqui
            m_chunks[i][j].setLOD(1); //começa com baixa resolução
            ChunkWorker* worker = new ChunkWorker(chunkX, chunkZ, m_config->lowRes, m_config, this);
            QThreadPool::globalInstance()->start(worker);
        }
    }
}

void terrainmanager::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions *glFuncs) {
    // Se o shader de terreno foi passado, desenhamos o terreno.
    if (terrainShaderProgram) {
        for (int i = 0; i < m_config->gridRenderSize; ++i) {
            for (int j = 0; j < m_config->gridRenderSize; ++j) {
                m_chunks[i][j].render(terrainShaderProgram, glFuncs);
            }
        }
    }
    // Se o shader de linha foi passado, desenhamos as bordas.
    if (lineShaderProgram) {
        for (int i = 0; i < m_config->gridRenderSize; ++i) {
            for (int j = 0; j < m_config->gridRenderSize; ++j) {
                m_chunks[i][j].renderBorders(lineShaderProgram, glFuncs, m_lineQuadVaoRef, m_lineQuadVboRef);
            }
        }
    }
}

void terrainmanager::onMeshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData)
{
    //qInfo() << "main thread: recebido malha pronta para chunk" << chunkX << "," << chunkZ;

    //calcula a posição do chunk na nossa grade interna
    int grid_i = (chunkX - m_centerChunkX) + m_config->gridRenderSize / 2;
    int grid_j = (chunkZ - m_centerChunkZ) + m_config->gridRenderSize / 2;

    //Verifica se o chunk ainda pertence a grade atual(ele pode ter saido de vista)
    if(grid_i >= 0 && grid_i < m_config->gridRenderSize && grid_j >= 0 && grid_j < m_config->gridRenderSize) {
        chunk& targetChunk = m_chunks[grid_i][grid_j];
        // apenas armazena os dados, nao faz upload para a gpu aqui
        targetChunk.setPendingMeshData(meshData);
    }
}
