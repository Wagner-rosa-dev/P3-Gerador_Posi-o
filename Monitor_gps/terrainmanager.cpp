#include "terrainmanager.h"
#include <QDebug>
#include <cmath>


terrainmanager::terrainmanager() :
    m_centerChunkX(0),
    m_centerChunkZ(0),
    m_lineQuadVaoRef(nullptr),
    m_lineQuadVboRef(nullptr)
{}

void terrainmanager::init(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs) {
    m_lineQuadVaoRef = lineQuadVao;
    m_lineQuadVboRef = lineQuadVbo;

    m_chunks.resize(GRID_SIZE);
    for (int i = 0; i < GRID_SIZE; ++i) {
        m_chunks[i].resize(GRID_SIZE);
    }

    //inicia a grade centrada em (0, 0)
    recenterGrid(0, 0, terrainShaderProgram, glFuncs);
}

void terrainmanager::update(const QVector3D& cameraPos, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs) {
    // Verifica se o centro da grade precisa mudar (logica de terreno infinito)
    int cameraChunkX = static_cast<int>(std::floor(cameraPos.x() / CHUNK_SIZE));
    int cameraChunkZ = static_cast<int>(std::floor(cameraPos.z() / CHUNK_SIZE));

    if (cameraChunkX != m_centerChunkX || cameraChunkZ != m_centerChunkZ) {
        recenterGrid(cameraChunkX, cameraChunkZ, terrainShaderProgram, glFuncs);
    }

    //atualiza o nivel de detalhe (LOD) dos chunks existentes
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j){
            chunk& currentChunk = m_chunks[i][j];
            float distanceToChunk = cameraPos.distanceToPoint(currentChunk.getCenterPosition());

            int desiredLOD = (distanceToChunk > LOD_DISTANCE_THRESHOLD) ? 1 : 0;
            if (currentChunk.getLOD() != desiredLOD) {
                int oldRes = currentChunk.getLOD() == 0 ? HIGH_RES : LOW_RES;
                currentChunk.setLOD(desiredLOD);
                int newRes = currentChunk.getLOD() == 0 ? HIGH_RES : LOW_RES;

                if (oldRes != newRes) {
                    currentChunk.generateMesh(newRes, terrainShaderProgram, glFuncs);
                }
            }
        }
    }
}

void terrainmanager::recenterGrid(int newCenterX, int newCenterZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions* glFuncs){
    qInfo() << "Recentering grid to:" << newCenterX << "," << newCenterZ;
    m_centerChunkX = newCenterX;
    m_centerChunkZ = newCenterZ;

    int halfGrid = GRID_SIZE / 2;

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            int chunkX = m_centerChunkX - halfGrid + i;
            int chunkZ = m_centerChunkZ - halfGrid + j;
            //Recicla o chunk na posição [i][j] da nossa matriz a nova coordenada
            m_chunks[i][j].recycle(chunkX, chunkZ, terrainShaderProgram, glFuncs);
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
