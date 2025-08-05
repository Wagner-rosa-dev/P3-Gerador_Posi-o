#include "terraingrid.h"
#include "worldconfig.h" // Inclui WorldConfig para acessar parâmetros como gridSquareSize.
#include "logger.h"      // Para mensagens de depuração (MY_LOG_INFO, MY_LOG_WARNING).
#include <QtMath> //para QFloor
#include <QColor>

/**
 * @brief Construtor da classe TerrainGrid.
 *
 * Inicializa os membros da classe para um estado padrão.
 */
TerrainGrid::TerrainGrid() :
    m_vertexCount(0),
    m_glFuncsRef(nullptr),
    m_config(nullptr),
    m_lastCenterGridX(0),
    m_lastCenterGridZ(0)
{}

/**
 * @brief Destrutor da classe TerrainGrid.
 *
 * Garante que os recursos OpenGL (VAO, VBO) sejam liberados quando o objeto é destruído.
 * Os objetos QOpenGLVertexArrayObject e QOpenGLBuffer têm seus próprios destrutores
 * que cuidam da liberação, mas é uma boa prática chamar `destroy()` explicitamente
 * se houver chance de o contexto OpenGL não estar ativo durante a destruição automática.
 * No entanto, para objetos declarados como membros, seus destrutores serão chamados
 * automaticamente quando o contexto GL estiver ativo (se `makeCurrent()` for chamado
 * antes da destruição do widget pai).
 */
TerrainGrid::~TerrainGrid() {
    // Não é necessário chamar destroy() aqui se o contexto OpenGL for garantido antes do destrutor do widget.
    // Se o widget OpenGL estiver vivo, ele cuida do contexto.
    // m_vao.destroy(); // Exemplo se fosse preciso destruir manualmente.
    // m_vbo.destroy(); // Exemplo se fosse preciso destruir manualmente.
}

/**
 * @brief Inicializa os recursos OpenGL do grid.
 * @param config Ponteiro para a configuração do mundo.
 * @param glFuncs Ponteiro para as funções OpenGL.
 *
 * Cria e configura o VAO e VBO do grid.
 */
void TerrainGrid::init(const WorldConfig* config, QOpenGLFunctions* glFuncs) {
    m_config = config; // Armazena a configuração do mundo.
    m_glFuncsRef = glFuncs; // Armazena as funções OpenGL.

    if (!m_glFuncsRef || !m_config) {
        MY_LOG_ERROR("TerrainGrid", "Tentativa de inicializar TerrainGrid com ponteiros nulos para GL functions ou WorldConfig.");
        return;
    }

    // Cria e configura os objetos OpenGL para o grid.
    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();

    // Nenhuma alocação de dados inicial aqui, pois updateGridGeometry() fará isso.

    // Configura o ponteiro do atributo de vértice (layout location 0 no shader para posição).
    // Assumimos que o shader de linha (lineShaderProgram) usará a_position em location 0.
    m_glFuncsRef->glEnableVertexAttribArray(0);
    m_glFuncsRef->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    m_vao.release();
    m_vbo.release();

    MY_LOG_INFO("TerrainGrid", "TerrainGrid inicializado com sucesso.");
}

/**
 * @brief Recalcula e faz o upload dos vértices do grid para a GPU.
 * @param currentCameraWorldX A coordenada X da câmera no mundo.
 * @param currentCameraWorldZ A coordenada Z da câmera no mundo.
 * @param terrainRenderSizeChunks O número de chunks (largura/altura) da área de terreno visível.
 *
 * Esta função irá gerar os vértices para o grid de linhas, cobrindo a área visível do terreno.
 * A lógica de geração de vértices será implementada em uma etapa posterior.
 */
void TerrainGrid::updateGridGeometry(float currentCameraWorldX, float currentCameraWorldZ, int terrainRenderSizeChunks) {
 if (!m_glFuncsRef || !m_config) {
        MY_LOG_ERROR("TerrainGrid", "Tentativa de atualizar geometria do grid sem inicialização adequada.");
        return;
    }

    // A geometria da grade só precisa ser gerada uma vez, a menos que o tamanho de renderização mude.
    if (m_vertexCount == 0) {
        MY_LOG_INFO("TerrainGrid", QString("Gerando geometria do grid. Tamanho da grade: %1 chunks.").arg(terrainRenderSizeChunks));

        std::vector<float> gridVertices;
        float y_offset = 0.01f;
        float line_half_thickness = m_config->gridLineThickness / 2.0f;

        // A grade é gerada centrada em (0,0) no seu próprio espaço de modelo.
        float totalGridWorldSize = static_cast<float>(terrainRenderSizeChunks) * m_config->chunkSize;
        float halfGridSize = totalGridWorldSize / 2.0f;

        int numGridLines = static_cast<int>(totalGridWorldSize / m_config->gridSquareSize) + 1;

        // Gerar linhas paralelas ao eixo X (horizontais)
        for (int i = 0; i < numGridLines; ++i) {
            float z = (i * m_config->gridSquareSize) - halfGridSize;
            float x_start = -halfGridSize;
            float x_end = halfGridSize;

            gridVertices.push_back(x_start);          gridVertices.push_back(y_offset); gridVertices.push_back(z - line_half_thickness);
            gridVertices.push_back(x_end);            gridVertices.push_back(y_offset); gridVertices.push_back(z - line_half_thickness);
            gridVertices.push_back(x_end);            gridVertices.push_back(y_offset); gridVertices.push_back(z + line_half_thickness);

            gridVertices.push_back(x_start);          gridVertices.push_back(y_offset); gridVertices.push_back(z - line_half_thickness);
            gridVertices.push_back(x_end);            gridVertices.push_back(y_offset); gridVertices.push_back(z + line_half_thickness);
            gridVertices.push_back(x_start);          gridVertices.push_back(y_offset); gridVertices.push_back(z + line_half_thickness);
        }

        // Gerar linhas paralelas ao eixo Z (verticais)
        for (int i = 0; i < numGridLines; ++i) {
            float x = (i * m_config->gridSquareSize) - halfGridSize;
            float z_start = -halfGridSize;
            float z_end = halfGridSize;

            gridVertices.push_back(x - line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_start);
            gridVertices.push_back(x + line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_start);
            gridVertices.push_back(x + line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_end);

            gridVertices.push_back(x - line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_start);
            gridVertices.push_back(x + line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_end);
            gridVertices.push_back(x - line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_end);
        }

        m_vertexCount = static_cast<int>(gridVertices.size() / 3);

        if (m_vertexCount > 0) {
            m_vbo.bind();
            m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_vbo.allocate(gridVertices.data(), gridVertices.size() * sizeof(float));
            m_vbo.release();
            MY_LOG_INFO("TerrainGrid", QString("Grid gerado com %1 vértices.").arg(m_vertexCount));
        } else {
            MY_LOG_WARNING("TerrainGrid", "Nenhum vértice gerado para o grid.");
        }
    }
}







/**
 * @brief Desenha a grade na tela.
 * @param lineShaderProgram O programa de shader de linha a ser usado.
 * @param viewMatrix A matriz de visão atual da câmera.
 * @param projectionMatrix A matriz de projeção atual da câmera.
 */
void TerrainGrid::render(QOpenGLShaderProgram* lineShaderProgram, const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix) {
    if (m_vertexCount == 0 || !m_vao.isCreated()) {
        return;
    }

    lineShaderProgram->bind();
    lineShaderProgram->setUniformValue("projectionMatrix", projectionMatrix);
    lineShaderProgram->setUniformValue("viewMatrix", viewMatrix);

    // Lógica para posicionar a grade dinamicamente com a câmera.
    QVector3D cameraPos = viewMatrix.inverted() * QVector3D(0,0,0);
    float gridSquareSize = m_config->gridSquareSize;

    float gridOffsetX = qFloor(cameraPos.x() / gridSquareSize) * gridSquareSize;
    float gridOffsetZ = qFloor(cameraPos.z() / gridSquareSize) * gridSquareSize;

    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();
    modelMatrix.translate(gridOffsetX, 0.0f, gridOffsetZ);

    lineShaderProgram->setUniformValue("modelMatrix", modelMatrix);
    lineShaderProgram->setUniformValue("lineColor", QColor(255, 255, 0, 255));

    m_vao.bind();
    // A primitiva de desenho GL_TRIANGLES é a correta para a geometria gerada.
    m_glFuncsRef->glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    m_vao.release();
}
