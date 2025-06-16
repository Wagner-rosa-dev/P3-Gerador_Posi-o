#include "terrainmanager.h"
#include "chunkworker.h"
#include <QDebug>
#include <cmath>

terrainmanager::terrainmanager() :
    QObject(nullptr),
    m_centerChunkX(0),
    m_centerChunkZ(0),
    m_lineQuadVaoRef(nullptr),
    m_lineQuadVboRef(nullptr),
    m_glFuncsRef(nullptr)
{
    //configuração da thread e do worker
    m_workerThread = new QThread(this); //crai a thread
    m_worker = new ChunkWorker();

    //move o objeto trabalhador para a nova thread, a partir daqui
    //todos os slots do m_worker serao executados na m_workerthread
    m_worker->moveToThread(m_workerThread);

    //conecta o nosso pedido (sinal) ao gatilho do trabalhor (slot)
    connect(this, &terrainmanager::requestMeshGeneration, m_worker, &ChunkWorker::generateChunkMesh);

    //conecta o anuncio do trabalhador (sinal) ao receptor (slot)
    connect(m_worker, &ChunkWorker::meshReady, this, &terrainmanager::onMeshReady);

    //inicia a thread, ela ficara em espera, aguardando por sinais
    m_workerThread->start();
}

terrainmanager::~terrainmanager()
{
    m_workerThread->quit(); // pede para a thread terminar seu loop de eventos
    m_workerThread->wait(); // Espera a thread finalizar de verdade
    delete m_worker; // delete o objeto trabalhador
}

void terrainmanager::init(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs) {
    m_lineQuadVaoRef = lineQuadVao;
    m_lineQuadVboRef = lineQuadVbo;
    m_glFuncsRef = glFuncs;

    m_chunks.resize(GRID_SIZE);
    for (int i = 0; i < GRID_SIZE; ++i) {
        m_chunks[i].resize(GRID_SIZE);
    }
    //inicia a grade centrada em (0, 0)
    recenterGrid(0, 0);
}

void terrainmanager::update(const QVector3D& cameraPos) {
    // Verifica se o centro da grade precisa mudar (logica de terreno infinito)
    int cameraChunkX = static_cast<int>(std::floor(cameraPos.x() / CHUNK_SIZE));
    int cameraChunkZ = static_cast<int>(std::floor(cameraPos.z() / CHUNK_SIZE));

    if (cameraChunkX != m_centerChunkX || cameraChunkZ != m_centerChunkZ) {
        recenterGrid(cameraChunkX, cameraChunkZ);
    }
    //atualiza o nivel de detalhe (LOD) dos chunks existentes
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j){
            chunk& currentChunk = m_chunks[i][j];
            int currentLOD = currentChunk.getLOD();
            int desiredLOD = currentLOD;

            float distanceToChunk = cameraPos.distanceToPoint(currentChunk.getCenterPosition());

            //logica de histerese
            if (currentLOD == 1 && distanceToChunk < LOD_DISTANCE_THRESHOLD - LOD_HYSTERESIS_BUFFER) {
                //se o chunk esta em baixa resolução, ele so muda para alta
                //se entrar bem na zona de alta resolução
                desiredLOD = 0;
            } else if (currentLOD == 0 && distanceToChunk > LOD_DISTANCE_THRESHOLD + LOD_HYSTERESIS_BUFFER) {
                //se o chunk esta em alta resolução, ele so muda para baixa
                //se sair bem da zona de alta resolução
                desiredLOD = 1;
            }

            if (currentLOD != desiredLOD) {
                currentChunk.setLOD(desiredLOD);
                int newRes = (desiredLOD == 0) ? HIGH_RES : LOW_RES;
                emit requestMeshGeneration(currentChunk.chunkGridX(), currentChunk.chunkGridZ(), newRes);
            }
        }
    }
}

void terrainmanager::recenterGrid(int newCenterX, int newCenterZ){
    qInfo() << "Recentering grid to:" << newCenterX << "," << newCenterZ;
    m_centerChunkX = newCenterX;
    m_centerChunkZ = newCenterZ;

    int halfGrid = GRID_SIZE / 2;

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            int chunkX = m_centerChunkX - halfGrid + i;
            int chunkZ = m_centerChunkZ - halfGrid + j;
            //Recicla o chunk na posição [i][j] da nossa matriz a nova coordenada
            m_chunks[i][j].recycle(chunkX, chunkZ);

            //dispara o sinal para gerar a malha deste chunk em segundo plano
            //a logica de lod pode ser mais complexa aqui
            m_chunks[i][j].setLOD(1); //começa com baixa resolução
            emit requestMeshGeneration(chunkX, chunkZ, LOW_RES);
        }
    }
}

void terrainmanager::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions *glFuncs) {
    // Se o shader de terreno foi passado, desenhamos o terreno.
    if (terrainShaderProgram) {
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                m_chunks[i][j].render(terrainShaderProgram, glFuncs);
            }
        }
    }
    // Se o shader de linha foi passado, desenhamos as bordas.
    if (lineShaderProgram) {
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                m_chunks[i][j].renderBorders(lineShaderProgram, glFuncs, m_lineQuadVaoRef, m_lineQuadVboRef);
            }
        }
    }
}

void terrainmanager::onMeshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData)
{
    //qInfo() << "main thread: recebido malha pronta para chunk" << chunkX << "," << chunkZ;

    //calcula a posição do chunk na nossa grade interna
    int grid_i = (chunkX - m_centerChunkX) + GRID_SIZE / 2;
    int grid_j = (chunkZ - m_centerChunkZ) + GRID_SIZE / 2;

    //Verifica se o chunk ainda pertence a grade atual(ele pode ter saido de vista)
    if(grid_i >= 0 && grid_i < GRID_SIZE && grid_j >= 0 && grid_j < GRID_SIZE) {
        chunk& targetChunk = m_chunks[grid_i][grid_j];
        // apenas armazena os dados, nao faz upload para a gpu aqui
        targetChunk.setPendingMeshData(meshData);
    }
}
