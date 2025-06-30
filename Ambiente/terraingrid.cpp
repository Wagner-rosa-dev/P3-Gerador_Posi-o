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
    if (!m_glFuncsRef || !m_config) { // Verifica se a inicialização foi bem-sucedida
        MY_LOG_ERROR("TerrainGrid", "Tentativa de atualizar geometria do grid sem inicialização adequada.");
        return;
    }

    // Calcula o centro do grid com base na posição da câmera e no tamanho do chunk.
    // O grid deve se alinhar com a grade de chunks para cobrir a área correta.
    int currentCenterGridX = static_cast<int>(qFloor(currentCameraWorldX / m_config->chunkSize));
    int currentCenterGridZ = static_cast<int>(qFloor(currentCameraWorldZ / m_config->chunkSize));

    // Ajusta o centro para a grade visual do grid (não a grade de chunks, mas os quadrados individuais)
    // Isso garante que o grid se mova suavemente e se alinhe com a grade global de coordenadas,
    // em vez de pular apenas quando a câmera entra em um novo chunk.
    float gridWorldOffsetX = currentCenterGridX * m_config->chunkSize;
    float gridWorldOffsetZ = currentCenterGridZ * m_config->chunkSize;

    // A matriz de modelo do grid será usada para posicionar a grade no mundo, centralizando-a
    // em relação à posição do jogador, mas alinhada à grade de chunks.
    m_modelMatrix.setToIdentity();
    m_modelMatrix.translate(gridWorldOffsetX, 0.0f, gridWorldOffsetZ);


    // Verifica se a área visível do grid mudou significativamente para justificar uma regeneração.
    // Usamos um limiar que é a metade do tamanho de um chunk. Se o centro da câmera mudou
    // mais do que isso em relação ao último centro de regeneração do grid, regeneramos.
    // Isso ajuda a evitar regenerações excessivas para pequenos movimentos.
    // Para um grid que se estende por *todo* o terreno renderizado, basta verificar se
    // o centro do CHUNK da câmera mudou, ou se é a primeira vez.

    if (currentCenterGridX != m_lastCenterGridX || currentCenterGridZ != m_lastCenterGridZ || m_vertexCount == 0) {
        MY_LOG_INFO("TerrainGrid", QString("Regenerando geometria do grid. Centro de chunks: X:%1, Z:%2. Câmera mundo: X:%3, Z:%4")
                                       .arg(currentCenterGridX).arg(currentCenterGridZ).arg(currentCameraWorldX).arg(currentCameraWorldZ));

        m_lastCenterGridX = currentCenterGridX;
        m_lastCenterGridZ = currentCenterGridZ;

        std::vector<float> gridVertices;
        float y_offset = 0.01f; // Elevação para evitar z-fighting com o terreno.
        float line_half_thickness = m_config->gridLineThickness / 2.0f; // Metade da espessura da linha.

        // Calcula a extensão total do grid em unidades do mundo.
        // O grid deve cobrir a área de 'gridRenderSize' chunks.
        float totalGridWorldSize = static_cast<float>(terrainRenderSizeChunks) * m_config->chunkSize;

        // Calcula o número de linhas e colunas que precisamos no grid.
        int numGridLines = static_cast<int>(totalGridWorldSize / m_config->gridSquareSize) + 1; // +1 para incluir a última linha/coluna

        // Gerar linhas paralelas ao eixo X (linhas horizontais)
        for (int i = 0; i < numGridLines; ++i) {
            float z = i * m_config->gridSquareSize; // Posição Z da linha
            float x_start = 0.0f;
            float x_end = totalGridWorldSize;

            // Triângulo 1
            gridVertices.push_back(x_start);          gridVertices.push_back(y_offset); gridVertices.push_back(z - line_half_thickness);
            gridVertices.push_back(x_end);            gridVertices.push_back(y_offset); gridVertices.push_back(z - line_half_thickness);
            gridVertices.push_back(x_end);            gridVertices.push_back(y_offset); gridVertices.push_back(z + line_half_thickness);

            // Triângulo 2
            gridVertices.push_back(x_start);          gridVertices.push_back(y_offset); gridVertices.push_back(z - line_half_thickness);
            gridVertices.push_back(x_end);            gridVertices.push_back(y_offset); gridVertices.push_back(z + line_half_thickness);
            gridVertices.push_back(x_start);          gridVertices.push_back(y_offset); gridVertices.push_back(z + line_half_thickness);
        }

        // Gerar linhas paralelas ao eixo Z (linhas verticais)
        for (int i = 0; i < numGridLines; ++i) {
            float x = i * m_config->gridSquareSize; // Posição X da linha
            float z_start = 0.0f;
            float z_end = totalGridWorldSize;

            // Triângulo 1
            gridVertices.push_back(x - line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_start);
            gridVertices.push_back(x + line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_start);
            gridVertices.push_back(x + line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_end);

            // Triângulo 2
            gridVertices.push_back(x - line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_start);
            gridVertices.push_back(x + line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_end);
            gridVertices.push_back(x - line_half_thickness); gridVertices.push_back(y_offset); gridVertices.push_back(z_end);
        }

        m_vertexCount = static_cast<int>(gridVertices.size() / 3); // Cada vértice tem 3 componentes (X, Y, Z)

        if (m_vertexCount > 0) {
            m_vbo.bind();
            // Aloca e envia os dados. Usamos QOpenGLBuffer::StaticDraw para otimizar,
            // pois os dados não mudarão frequentemente uma vez que o grid é regenerado.
            m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
            m_vbo.allocate(gridVertices.data(), QOpenGLBuffer::StaticDraw);
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
        return; // Não renderiza se não houver vértices ou o VAO não foi criado.
    }

    lineShaderProgram->bind();
    lineShaderProgram->setUniformValue("projectionMatrix", projectionMatrix);
    lineShaderProgram->setUniformValue("viewMatrix", viewMatrix);
    lineShaderProgram->setUniformValue("modelMatrix", m_modelMatrix); // A matriz de modelo será a identidade se não houver translação.
    lineShaderProgram->setUniformValue("lineColor", QColor(255, 255, 0, 255)); // Cor do grid (amarelo)

    m_vao.bind();
    // Desenha as linhas. Como definiremos as linhas como GL_LINES, cada 2 vértices formam uma linha.
    // Ou, se usarmos quads para as linhas, será GL_TRIANGLES.
    // Por enquanto, usamos GL_LINES_STRIP ou GL_LINES para simplicidade, depois ajustamos se for quad.
    m_glFuncsRef->glDrawArrays(GL_LINES, 0, m_vertexCount); // Assumindo GL_LINES por enquanto
    m_vao.release();
}
