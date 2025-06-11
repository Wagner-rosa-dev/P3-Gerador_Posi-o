#include "chunk.h"
#include "noiseutils.h"
#include <QDebug>



chunk::chunk() :
    m_chunkGridX(0),
    m_chunkGridZ(0),
    // m_vao, m_vbo, m_ebo sao inicializados por seus proprios construtores padrao
    m_indexCount(0),
    m_vertexCount(0),
    //m_modelmatrix e minicializada como identidade por padrao
    m_currentResolution(0),
    m_currentLOD(-1)
{}

chunk::~chunk() {

}

chunk::chunk(chunk&& other) noexcept(true)
    : m_chunkGridX(other.m_chunkGridX),
    m_chunkGridZ(other.m_chunkGridZ),
    m_vao(std::move(other.m_vao)),         // Move a propriedade do unique_ptr
    m_vbo(std::move(other.m_vbo)),         // Move a propriedade do unique_ptr
    m_ebo(std::move(other.m_ebo)),         // Move a propriedade do unique_ptr
    m_indexCount(other.m_indexCount),
    m_vertexCount(other.m_vertexCount),
    m_modelMatrix(std::move(other.m_modelMatrix)), // QMatrix4x4 também suporta movimento
    m_currentResolution(other.m_currentResolution),
    m_currentLOD(other.m_currentLOD)
{
    // Deixa o objeto 'other' em um estado válido, mas "vazio" ou resetado,
    // para que seu destrutor não tente liberar recursos que foram movidos.
    other.m_chunkGridX = 0; // Opcional para tipos simples, mas boa prática
    other.m_chunkGridZ = 0; // Opcional
    other.m_indexCount = 0;
    other.m_vertexCount = 0;
    other.m_currentResolution = 0;
    other.m_currentLOD = -1;
    // other.m_modelMatrix.setToIdentity(); // Opcional, já que foi movida

    // Os objetos m_vao, m_vbo, m_ebo em 'other' agora estão em um estado "movido de"
    // (geralmente inválido para uso, mas seguro para destruição).
    // qInfo() << "Chunk Move Constructed";
}

// Operador de Atribuição por Movimento
chunk& chunk::operator=(chunk&& other) noexcept(true) {
    if (this != &other) { // Proteção contra auto-atribuição (ex: c = std::move(c);)
        // Liberar recursos existentes deste objeto (this)
        // Se m_vao, m_vbo, m_ebo estivessem ativos, seus destrutores seriam chamados
        // quando são sobrescritos pelas operações de movimento abaixo.
        // Se você quiser ser explícito sobre a liberação antes da atribuição:
        //if (m_vao.isCreated()) m_vao.destroy();
        //if (m_vbo.isCreated()) m_vbo.destroy();
        //if (m_ebo.isCreated()) m_ebo.destroy();

        // Mover os dados de 'other' para 'this'
        m_chunkGridX = other.m_chunkGridX;
        m_chunkGridZ = other.m_chunkGridZ;
        m_vao = std::move(other.m_vao);
        m_vbo = std::move(other.m_vbo);
        m_ebo = std::move(other.m_ebo);
        m_indexCount = other.m_indexCount;
        m_vertexCount = other.m_vertexCount;
        m_modelMatrix = std::move(other.m_modelMatrix);
        m_currentResolution = other.m_currentResolution;
        m_currentLOD = other.m_currentLOD;

        // Resetar o objeto 'other'
        other.m_chunkGridX = 0;
        other.m_chunkGridZ = 0;
        other.m_indexCount = 0;
        other.m_vertexCount = 0;
        other.m_currentResolution = 0;
        other.m_currentLOD = -1;
        // other.m_modelMatrix.setToIdentity();
    }
    // qInfo() << "Chunk Move Assigned";
    return *this;
}

void chunk::init(int cX, int cZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs) {
    m_chunkGridX = cX;
    m_chunkGridZ = cZ;
    float worldX = static_cast<float>(m_chunkGridX * CHUNK_SIZE);
    float worldZ = static_cast<float>(m_chunkGridZ * CHUNK_SIZE);
    m_modelMatrix.setToIdentity();
    m_modelMatrix.translate(worldX, 0.0f, worldZ);
    setLOD(0); //Define o LOD inicial, que definira o m_currentResolution
    generateMesh(m_currentResolution, terrainShaderProgram, glFuncs);
}

void chunk::generateMesh(int resolution, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs) {
    if(!glFuncs) {
        qWarning() << "generateMesh called with null QOpenGlFunctions pointer.";
        return;
    }



    // 1. GERAÇÃO DOS DADOS (como antes)
    m_vao.reset(); m_vbo.reset(); m_ebo.reset();
    m_currentResolution = resolution;
    if (resolution <= 1) return;


    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(resolution) * static_cast<size_t>(resolution));
    std::vector<GLuint> indices;
    indices.reserve(static_cast<size_t>(resolution - 1) * static_cast<size_t>(resolution - 1) * 6);
    float step = static_cast<float>(CHUNK_SIZE) / (resolution - 1);

    for (int r = 0; r < resolution; ++r) {
        for (int c = 0; c < resolution; ++c) {
            Vertex v;
            float localX = c * step;
            float localZ = r * step;
            float noise_coord_x = (static_cast<float>(m_chunkGridX * CHUNK_SIZE)) + localX;
            float noise_coord_z = (static_cast<float>(m_chunkGridZ * CHUNK_SIZE)) + localZ;
            v.position = QVector3D(localX, NoiseUtils::getHeight(noise_coord_x, noise_coord_z), localZ);
            float offset_norm = 0.1f;
            float hL = NoiseUtils::getHeight(noise_coord_x - offset_norm, noise_coord_z);
            float hR = NoiseUtils::getHeight(noise_coord_x + offset_norm, noise_coord_z);
            float hD = NoiseUtils::getHeight(noise_coord_x, noise_coord_z - offset_norm);
            float hU = NoiseUtils::getHeight(noise_coord_x, noise_coord_z + offset_norm);
            v.normal = QVector3D(hL - hR, 2.0f * offset_norm, hD - hU).normalized();
            vertices.push_back(v);
        }
    }

    for (int r = 0; r < resolution - 1; ++r) {
        for (int c = 0; c < resolution - 1; ++c) {
            GLuint topLeft = static_cast<GLuint>(r * resolution + c);
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = static_cast<GLuint>((r + 1) * resolution + c);
            GLuint bottomRight = bottomLeft + 1;
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    m_vertexCount = static_cast<int>(vertices.size());
    m_indexCount = static_cast<int>(indices.size());
    if (m_indexCount == 0) return;

    // 2. CONFIGURAÇÃO DOS BUFFERS OPENGL (A PARTE CORRIGIDA)
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    m_vao->create();
    m_vao->bind(); // <<-- VAO vinculado

    // Configura o VBO (dados dos vértices)
    m_vbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(vertices.data(), m_vertexCount * sizeof(Vertex));

    // Configura os atributos do vértice.
    // Isso diz ao VAO como interpretar os dados do VBO que acabamos de vincular.
    glFuncs->glEnableVertexAttribArray(0); // location = 0 (position)
    glFuncs->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glFuncs->glEnableVertexAttribArray(1); // location = 1 (normal)
    glFuncs->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Configura o EBO (índices)
    // O VAO também "lembra" qual EBO está vinculado enquanto ele (o VAO) está ativo.
    m_ebo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
    m_ebo->create();
    m_ebo->bind();
    m_ebo->allocate(indices.data(), m_indexCount * sizeof(GLuint));

    // 3. LIBERA OS VÍNCULOS (BOA PRÁTICA)
    m_vao->release(); // Libera o VAO primeiro

    // Opcional, mas limpo: desvincular os outros buffers depois que o VAO já guardou o estado.
    m_vbo->release();
    m_ebo->release();
}

void chunk::setLOD(int lodLevel) {
    m_currentLOD = lodLevel;
    if (lodLevel == 0) {
        m_currentResolution = HIGH_RES;
    } else {
        m_currentResolution = LOW_RES;
    }
}

QVector3D chunk::getCenterPosition() const {
    float worldX = (static_cast<float>(m_chunkGridX) + 0.5f) * CHUNK_SIZE;
    float worldZ = (static_cast<float>(m_chunkGridZ) + 0.5f) * CHUNK_SIZE;
    return QVector3D(worldX, NoiseUtils::getHeight(worldX, worldZ), worldZ);
}

void chunk::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs) {
    if (m_indexCount == 0 || !m_vao || !m_vao->isCreated()) { return; }

    terrainShaderProgram->setUniformValue("modelMatrix", m_modelMatrix);
    m_vao->bind();
    glFuncs->glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    m_vao->release();
}

void chunk::renderBorders(QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions* glFuncs, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo) {
    if (!lineQuadVao || !lineQuadVao->isCreated()) return;
    lineShaderProgram->setUniformValue("modelMatrix", m_modelMatrix);
    lineQuadVao->bind();
    glFuncs->glDrawArrays(GL_LINE_LOOP, 0, 4);
    lineQuadVao->release();
}

void chunk::recycle(int cX,int cZ, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs) {
    //Esta função reutiliza o chunk em uma nova posição
    m_chunkGridX = cX;
    m_chunkGridZ = cZ;
    float worldX = static_cast<float>(m_chunkGridX * CHUNK_SIZE);
    float worldZ = static_cast<float>(m_chunkGridZ * CHUNK_SIZE);
    m_modelMatrix.setToIdentity();
    m_modelMatrix.translate(worldX, 0.0f, worldZ);

    //Força o LOD a ser reavaliado. poderiamos simplesmente gerar com alta resolução
    //ou manter o LOD anterior para evit "pop-in" de geometria. vamos gerar com a resolução atual.
    if(m_currentResolution == 0) {
        setLOD(0);
    }

    generateMesh(m_currentResolution, terrainShaderProgram, glFuncs);
}
