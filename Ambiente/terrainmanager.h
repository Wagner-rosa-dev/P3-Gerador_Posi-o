#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include "chunk.h"
#include <vector>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QThread>

class ChunkWorker;

class terrainmanager : public QObject { Q_OBJECT

public:

    terrainmanager();
    ~terrainmanager();

    static const int GRID_SIZE = 9; // Uma grade 9x9 como exemplo

    void init(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs);
    void update(const QVector3D& cameraPos);
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions *glFuncs);

signals:
    //sinal para pedir ao trabalhor que here uma nova malha
    void requestMeshGeneration(int chunkX, int chunkZ, int resolution);

private slots:
    //slota para receber a malha pronta do trabalhador
    void onMeshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData);

private:
    void recenterGrid(int newCenterX, int newCenterZ);

    std::vector<std::vector<chunk>> m_chunks;
    int m_centerChunkX;
    int m_centerChunkZ;
    QOpenGLVertexArrayObject* m_lineQuadVaoRef;
    QOpenGLBuffer* m_lineQuadVboRef;
    QOpenGLFunctions* m_glFuncsRef;

    const float LOD_DISTANCE_THRESHOLD = CHUNK_SIZE * 2.5f;
    const float LOD_HYSTERESIS_BUFFER = 5.0f;

    //variaveis para gerenciar a thread
    QThread* m_workerThread;
    ChunkWorker* m_worker;
};

#endif // TERRAINMANAGER_H
