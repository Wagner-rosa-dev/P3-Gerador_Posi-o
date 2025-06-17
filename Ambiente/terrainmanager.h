#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include "chunk.h"
#include <vector>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QThread>

class ChunkWorker;
struct WorldConfig;

class terrainmanager : public QObject { Q_OBJECT

public:

    terrainmanager();
    ~terrainmanager();



    void init(const WorldConfig* config, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs);
    void update(const QVector3D& cameraPos);
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions *glFuncs);


private slots:
    //slota para receber a malha pronta do trabalhador
    void onMeshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData);

private:
    void recenterGrid(int newCenterX, int newCenterZ);

    const WorldConfig* m_config;
    std::vector<std::vector<chunk>> m_chunks;
    int m_centerChunkX;
    int m_centerChunkZ;
    QOpenGLVertexArrayObject* m_lineQuadVaoRef;
    QOpenGLBuffer* m_lineQuadVboRef;
    QOpenGLFunctions* m_glFuncsRef;


    const float LOD_HYSTERESIS_BUFFER = 5.0f;


};

#endif // TERRAINMANAGER_H
