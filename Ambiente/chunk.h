#ifndef CHUNK_H
#define CHUNK_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QMatrix4x4>
#include <vector>
#include <utility>
#include <memory>
#include <QTimer>
#include <QKeyEvent>

const int CHUNK_SIZE = 32;
const int HIGH_RES = 65;
const int LOW_RES = 17;

struct Vertex {
    QVector3D position;
    QVector3D normal;
};

class chunk {
public:
    //struct para transportar os dados da malha entre os threads
    struct MeshData {
        int chunkGridX;
        int chunkGridZ;
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        int resolution;
    };

    chunk();
    ~chunk();
    // ----- semantica de movimento e copia ------
    //construtor de movimento
    chunk(chunk&& other) noexcept(true);
    //Operador de atribuição por movimento
    chunk& operator=(chunk&& other) noexcept(true);
    //Impedir copias(importante apra classes com rescursos opengl gerenciados)
    chunk(const chunk& other) = delete;
    chunk& operator=(const chunk& other) = delete;

    void init(int cX, int cZ, QOpenGLFunctions *glFuncs);
    void recycle(int cX, int cZ);

    static MeshData generateMeshData(int cX, int cZ, int resolution);

    void uploadMeshData(const MeshData& data, QOpenGLFunctions* glFuncs);

    //void generateMesh(int resolution, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs);
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs);
    void renderBorders(QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions* glFuncs, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo);
    void setLOD(int lodLevel);
    int getLOD() const {return m_currentLOD; }
    QVector3D getCenterPosition() const;
    QMatrix4x4 modelMatrix() const { return m_modelMatrix; }

    int chunkGridX() const { return m_chunkGridX; }
    int chunkGridZ() const { return m_chunkGridZ; }

    void setPendingMeshData(const MeshData& data);
private:
    int m_chunkGridX;
    int m_chunkGridZ;
    int m_indexCount;
    int m_vertexCount;
    int m_currentResolution;
    int m_currentLOD;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    std::unique_ptr<QOpenGLBuffer> m_vbo;
    std::unique_ptr<QOpenGLBuffer> m_ebo;
    QMatrix4x4 m_modelMatrix;

    bool m_hasPendingMesh;
    MeshData m_pendingMeshData;
};
#endif // CHUNK_H
