#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include "chunk.h"
#include <vector>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

class terrainmanager{
public:
    terrainmanager();

    static const int GRID_SIZE = 9; // Uma grade 9x9 como exemplo

    void init(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs);
    void update(const QVector3D& cameraPos, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs);
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions *glFuncs);

private:


    void recenterGrid(int newCenterX, int newCenterZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions* glFuncs);

    std::vector<std::vector<chunk>> m_chunks;


    int m_centerChunkX;
    int m_centerChunkZ;

    QOpenGLVertexArrayObject* m_lineQuadVaoRef;
    QOpenGLBuffer* m_lineQuadVboRef;

    const float LOD_DISTANCE_THRESHOLD = CHUNK_SIZE * 2.5f;
};

#endif // TERRAINMANAGER_H
