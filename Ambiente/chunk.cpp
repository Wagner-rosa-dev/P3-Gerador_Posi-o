#include "chunk.h" // Inclui o cabeçalho da classe chunk.
#include "noiseutils.h" // Inclui NoiseUtils para obter altura e normal do terreno.
#include <QDebug> // Para mensagens de depuração (qInfo, qWarning).
#include "worldconfig.h" // Inclui WorldConfig para acessar parâmetros como chunkSize.

/**
 * @brief Construtor padrão da classe chunk.
 *
 * Inicializa os membros do chunk com valores padrão, garantindo um estado base válido.
 */
chunk::chunk() :
    m_chunkGridX(0), // Inicializa a coordenada X do chunk na grade.
    m_chunkGridZ(0), // Inicializa a coordenada Z do chunk na grade.
    // m_vao, m_vbo, m_ebo são inicializados por seus próprios construtores padrão (std::unique_ptr é nullptr por padrão).
    m_indexCount(0), // Inicializa a contagem de índices para 0.
    m_vertexCount(0), // Inicializa a contagem de vértices para 0.
    // m_modelMatrix é inicializada como identidade por padrão por QMatrix4x4.
    m_currentResolution(0), // Inicializa a resolução atual.
    m_currentLOD(-1), // Inicializa o LOD (Nível de Detalhe) para um valor inválido.
    m_hasPendingMesh(false) // Inicializa a flag de malha pendente como falsa.
{}

/**
 * @brief Destrutor da classe chunk.
 *
 * Os membros `std::unique_ptr` (m_vao, m_vbo, m_ebo) gerenciam a vida útil
 * dos objetos OpenGL. Seus destrutores serão chamados automaticamente,
 * liberando os recursos da GPU quando o chunk for destruído.
 */
chunk::~chunk() {}

/**
 * @brief Construtor de movimento da classe chunk.
 * @param other O objeto chunk do qual os recursos serão movidos.
 *
 * Permite que objetos `chunk` sejam movidos eficientemente, transferindo
 * a propriedade dos recursos OpenGL (VAO, VBO, EBO) de um objeto temporário
 * ou expirado para um novo, evitando cópias desnecessárias e caras.
 */
chunk::chunk(chunk&& other) noexcept(true)
    :   m_chunkGridX(other.m_chunkGridX), // Move o valor de m_chunkGridX.
    m_chunkGridZ(other.m_chunkGridZ), // Move o valor de m_chunkGridZ.
    m_indexCount(other.m_indexCount), // Move o valor de m_indexCount.
    m_vertexCount(other.m_vertexCount), // Move o valor de m_vertexCount.
    m_currentResolution(other.m_currentResolution), // Move o valor de m_currentResolution.
    m_currentLOD(other.m_currentLOD), // Move o valor de m_currentLOD.
    m_vao(std::move(other.m_vao)),         // Move a propriedade do unique_ptr m_vao.
    m_vbo(std::move(other.m_vbo)),         // Move a propriedade do unique_ptr m_vbo.
    m_ebo(std::move(other.m_ebo)),         // Move a propriedade do unique_ptr m_ebo.
    m_modelMatrix(std::move(other.m_modelMatrix)) // QMatrix4x4 também suporta movimento.
{
    // Deixa o objeto 'other' em um estado válido, mas "vazio" ou resetado,
    // para que seu destrutor não tente liberar recursos que foram movidos.
    other.m_chunkGridX = 0; // Zera a coordenada X do objeto 'other'.
    other.m_chunkGridZ = 0; // Zera a coordenada Z do objeto 'other'.
    other.m_indexCount = 0; // Zera a contagem de índices do objeto 'other'.
    other.m_vertexCount = 0; // Zera a contagem de vértices do objeto 'other'.
    other.m_currentResolution = 0; // Zera a resolução do objeto 'other'.
    other.m_currentLOD = -1; // Define o LOD do objeto 'other' como inválido.
    // Os objetos m_vao, m_vbo, m_ebo em 'other' agora estão em um estado "movido de"
    // (geralmente inválido para uso, mas seguro para destruição).
    // qInfo() << "Chunk Move Constructed";
}

/**
 * @brief Operador de atribuição por movimento da classe chunk.
 * @param other O objeto chunk do qual os recursos serão movidos.
 * @return Uma referência ao objeto chunk atual (this).
 *
 * Permite a atribuição de um objeto `chunk` a outro via movimento,
 * liberando os recursos do objeto de destino e assumindo a propriedade
 * dos recursos do objeto de origem.
 */
chunk& chunk::operator=(chunk&& other) noexcept(true) {
    if (this != &other) { // Proteção contra auto-atribuição (ex: c = std::move(c);).
        // Liberar recursos existentes deste objeto (this) antes de mover.
        // Mover os dados de 'other' para 'this'.
        m_chunkGridX = other.m_chunkGridX; // Move o valor de m_chunkGridX.
        m_chunkGridZ = other.m_chunkGridZ; // Move o valor de m_chunkGridZ.
        m_indexCount = other.m_indexCount; // Move o valor de m_indexCount.
        m_vertexCount = other.m_vertexCount; // Move o valor de m_vertexCount.
        m_currentResolution = other.m_currentResolution; // Move o valor de m_currentResolution.
        m_currentLOD = other.m_currentLOD; // Move o valor de m_currentLOD.
        m_vao = std::move(other.m_vao); // Move a propriedade do unique_ptr m_vao.
        m_vbo = std::move(other.m_vbo); // Move a propriedade do unique_ptr m_vbo.
        m_ebo = std::move(other.m_ebo); // Move a propriedade do unique_ptr m_ebo.
        m_modelMatrix = std::move(other.m_modelMatrix); // Move a QMatrix4x4.
        // Resetar o objeto 'other' para um estado válido, mas vazio.
        other.m_chunkGridX = 0;
        other.m_chunkGridZ = 0;
        other.m_indexCount = 0;
        other.m_vertexCount = 0;
        other.m_currentResolution = 0;
        other.m_currentLOD = -1;
    }
    // qInfo() << "Chunk Move Assigned";
    return *this; // Retorna uma referência ao objeto atual.
}

/**
 * @brief Gera os dados de vértices e índices para a malha de um chunk.
 * @param cX Coordenada X do chunk na grade.
 * @param cZ Coordenada Z do chunk na grade.
 * @param resolution A resolução da malha a ser gerada (número de vértices por lado).
 * @param chunkSize O tamanho do chunk, usado para calcular posições no mundo.
 * @return MeshData Uma estrutura contendo os dados de vértices e índices gerados.
 *
 * Esta função é intensiva em CPU e deve ser executada em uma thread separada.
 * Ela itera sobre uma grade para criar vértices, calcula suas posições (incluindo altura do ruído)
 * e normais, e então gera os índices para formar triângulos.
 */
chunk::MeshData chunk::generateMeshData(int cX, int cZ, int resolution, int chunkSize)
{
    MeshData data; // Cria uma estrutura MeshData para armazenar os resultados.
    data.chunkGridX = cX; // Armazena a coordenada X do chunk.
    data.chunkGridZ = cZ; // Armazena a coordenada Z do chunk.
    data.resolution = resolution; // Armazena a resolução utilizada.

    if (resolution <= 1) return data; // Retorna dados vazios se a resolução não for válida (mínimo 2x2 para triângulos).

    // Reserva espaço nos vetores para otimizar alocações de memória.
    data.vertices.reserve(static_cast<size_t>(resolution) * static_cast<size_t>(resolution));
    data.indices.reserve(static_cast<size_t>(resolution - 1) * static_cast<size_t>(resolution - 1) * 6);

    // Calcula o tamanho do passo entre os vértices com base no tamanho do chunk e na resolução.
    float step = static_cast<float>(chunkSize) / (resolution - 1);

    // Geração de Vértices:
    // Itera sobre cada ponto da grade para criar um vértice.
    for (int r = 0; r < resolution; ++r) { // Loop para as linhas (eixo Z local).
        for (int c = 0; c < resolution; ++c) { // Loop para as colunas (eixo X local).
            Vertex v; // Cria uma nova estrutura de vértice.
            float localX = c * step; // Calcula a coordenada X local do vértice dentro do chunk.
            float localZ = r * step; // Calcula a coordenada Z local do vértice dentro do chunk.

            // Calcula as coordenadas globais X e Z para a função de ruído.
            // Soma a posição base do chunk no mundo (cX * chunkSize, cZ * chunkSize) com as coordenadas locais.
            float noise_coord_x (static_cast<float>(cX * chunkSize) + localX);
            float noise_coord_z (static_cast<float>(cZ * chunkSize) + localZ);
            // Define a posição do vértice, obtendo a altura do terreno via NoiseUtils::getHeight.
            v.position = QVector3D(localX, NoiseUtils::getHeight(noise_coord_x, noise_coord_z), localZ);

            // Lógica da normal:
            // As normais são usadas para iluminação. São calculadas aproximando as mudanças de altura ao redor do vértice.
            float offset_norm = 0.1f; // Pequeno offset para amostragem da altura ao redor do ponto.
            // Amostra a altura em pontos ligeiramente deslocados para calcular as diferenças.
            float hL = NoiseUtils::getHeight(noise_coord_x - offset_norm, noise_coord_z); // Altura à esquerda.
            float hR = NoiseUtils::getHeight(noise_coord_x + offset_norm, noise_coord_z); // Altura à direita.
            float hD = NoiseUtils::getHeight(noise_coord_x, noise_coord_z - offset_norm); // Altura para baixo (Z-).
            float hU = NoiseUtils::getHeight(noise_coord_x, noise_coord_z + offset_norm); // Altura para cima (Z+).
            // Calcula o vetor normal usando as diferenças de altura e normaliza para ter comprimento 1.
            v.normal = QVector3D(hL - hR, 2.0f * offset_norm, hD - hU).normalized();

            data.vertices.push_back(v); // Adiciona o vértice ao vetor de vértices da malha.
        }
    }

    // Geração de Índices:
    // Itera sobre cada "quadrado" formado por 4 vértices adjacentes na grade e o divide em dois triângulos.
    // Cada quadrado [r][c] ... [r+1][c+1] forma dois triângulos.
    for (int r = 0; r < resolution - 1; ++r) {
        for (int c = 0; c < resolution - 1; ++c) {
            // Calcula os índices dos 4 vértices que formam o quadrado atual.
            GLuint topLeft = static_cast<GLuint>(r * resolution + c);
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = static_cast<GLuint>((r + 1) * resolution + c);
            GLuint bottomRight = bottomLeft + 1;
            // Adiciona os índices para o primeiro triângulo (superior esquerdo).
            data.indices.push_back(topLeft);
            data.indices.push_back(bottomLeft);
            data.indices.push_back(topRight);
            // Adiciona os índices para o segundo triângulo (inferior direito).
            data.indices.push_back(topRight);
            data.indices.push_back(bottomLeft);
            data.indices.push_back(bottomRight);
        }
    }
    return data; // Retorna a estrutura MeshData preenchida.
}

/**
 * @brief Faz o upload dos dados de malha (vértices e índices) para a GPU.
 * @param data A estrutura MeshData contendo os vértices e índices a serem enviados para a GPU.
 * @param glFuncs Ponteiro para as funções OpenGL.
 *
 * Esta função DEVE ser chamada na thread que possui o contexto OpenGL ativo (geralmente a thread principal).
 * Ela cria e configura o VAO, VBO e EBO para o chunk, e copia os dados para a memória da GPU.
 */
void chunk::uploadMeshData(const chunk::MeshData& data, QOpenGLFunctions* glFuncs)
{
    // Retorna se o ponteiro para funções OpenGL é nulo ou se os dados de índice estão vazios.
    if (!glFuncs || data.indices.empty()) {
        return;
    }

    // Limpa os buffers antigos antes de criar os novos.
    // std::unique_ptr::reset() libera o recurso apontado e define o ponteiro para nullptr.
    m_vao.reset();
    m_vbo.reset();
    m_ebo.reset();

    m_currentResolution = data.resolution; // Atualiza a resolução atual do chunk.
    m_indexCount = static_cast<int>(data.indices.size()); // Atualiza a contagem de índices.
    m_vertexCount = static_cast<int>(data.vertices.size()); // Atualiza a contagem de vértices.

    // Cria e configura os objetos OpenGL:
    m_vao = std::make_unique<QOpenGLVertexArrayObject>(); // Cria um novo VAO.
    m_vao->create(); // Aloca o VAO na GPU.
    m_vao->bind();   // Ativa o VAO.

    m_vbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer); // Cria um novo VBO (tipo VertexBuffer).
    m_vbo->create(); // Aloca o VBO na GPU.
    m_vbo->bind();   // Ativa o VBO.
    // Aloca memória no VBO e copia os dados de vértice da CPU para a GPU.
    m_vbo->allocate(data.vertices.data(), m_vertexCount * sizeof(Vertex));

    // Configura os ponteiros de atributo de vértice dentro do VAO:
    // Ativa o atributo de posição (layout location 0 no shader).
    glFuncs->glEnableVertexAttribArray(0);
    // Define como os dados de posição são lidos do VBO: 3 floats, sem normalização, passo entre vértices, offset.
    glFuncs->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // Ativa o atributo de normal (layout location 1 no shader).
    glFuncs->glEnableVertexAttribArray(1);
    // Define como os dados de normal são lidos do VBO.
    glFuncs->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    m_ebo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer); // Cria um novo EBO (tipo IndexBuffer).
    m_ebo->create(); // Aloca o EBO na GPU.
    m_ebo->bind();   // Ativa o EBO.
    // Aloca memória no EBO e copia os dados de índice da CPU para a GPU.
    m_ebo->allocate(data.indices.data(), m_indexCount * sizeof(GLuint));

    m_vao->release(); // Libera o VAO (desvincula).
    m_vbo->release(); // Libera o VBO.
    m_ebo->release(); // Libera o EBO.
}

/**
 * @brief Define o Nível de Detalhe (LOD) atual para o chunk.
 * @param lodLevel O nível de LOD a ser definido (e.g., 0 para alta resolução, 1 para baixa).
 */
void chunk::setLOD(int lodLevel) {
    m_currentLOD = lodLevel;
}

/**
 * @brief Calcula e retorna a posição central do chunk no espaço do mundo.
 * @param chunkSize O tamanho do chunk.
 * @return QVector3D representando a posição central do chunk.
 *
 * A posição Y é obtida usando a função de altura do terreno no centro do chunk.
 */
QVector3D chunk::getCenterPosition(int chunkSize) const {
    // Calcula a coordenada X central do chunk no mundo.
    float worldX = (static_cast<float>(m_chunkGridX) + 0.5f) * chunkSize;
    // Calcula a coordenada Z central do chunk no mundo.
    float worldZ = (static_cast<float>(m_chunkGridZ) + 0.5f) * chunkSize;
    // Retorna um QVector3D com a posição X, a altura do terreno em (worldX, worldZ) e a posição Z.
    return QVector3D(worldX, NoiseUtils::getHeight(worldX, worldZ), worldZ);
}

/**
 * @brief Armazena os dados de malha recebidos de uma thread de worker.
 * @param data A estrutura MeshData contendo os dados gerados pela thread de worker.
 *
 * Marca o chunk como tendo uma malha pendente, que será uploaded na GPU
 * na próxima chamada a `render` (quando o contexto OpenGL estiver ativo).
 */
void chunk::setPendingMeshData(const MeshData& data) {
    m_pendingMeshData = data; // Copia os dados da malha gerada para o membro m_pendingMeshData.
    m_hasPendingMesh = true; // Define a flag para indicar que há uma malha pendente para upload.
}

/**
 * @brief Renderiza o chunk na tela.
 * @param terrainShaderProgram O programa de shader de terreno a ser usado.
 * @param glFuncs Ponteiro para as funções OpenGL.
 *
 * Se houver uma malha pendente (gerada em outra thread), esta função faz o upload
 * dos dados para a GPU. Em seguida, ativa o shader de terreno, define a matriz de modelo
 * do chunk e desenha a malha usando os índices.
 */
void chunk::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs) {
    // Se há uma malha pendente, faça o upload agora, com o contexto ativo.
    if (m_hasPendingMesh) {
        uploadMeshData(m_pendingMeshData, glFuncs); // Chama uploadMeshData para enviar os dados para a GPU.
        m_pendingMeshData = {}; // Limpa os dados da CPU após o upload para economizar memória.
        m_hasPendingMesh = false; // Reseta a flag de malha pendente.
    }

    // Retorna se não há índices para desenhar ou se o VAO não foi criado corretamente.
    if (m_indexCount == 0 || !m_vao || !m_vao->isCreated()) { return; }

    // Define a matriz de modelo do chunk no shader de terreno.
    terrainShaderProgram->setUniformValue("modelMatrix", m_modelMatrix);
    m_vao->bind(); // Ativa o VAO do chunk.
    // Desenha os triângulos usando os índices do EBO.
    glFuncs->glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    m_vao->release(); // Libera o VAO.
}


/**
 * @brief Reutiliza um objeto chunk existente em uma nova posição na grade.
 * @param cX Nova coordenada X do chunk na grade.
 * @param cZ Nova coordenada Z do chunk na grade.
 * @param chunkSize O tamanho do chunk.
 *
 * Atualiza as coordenadas de grade do chunk e recalcula sua matriz de modelo
 * para refletir a nova posição no mundo.
 */
void chunk::recycle(int cX,int cZ, int chunkSize) {
    // Esta função reutiliza o chunk em uma nova posição.
    m_chunkGridX = cX; // Atualiza a coordenada X da grade.
    m_chunkGridZ = cZ; // Atualiza a coordenada Z da grade.
    // Calcula a posição do chunk no espaço do mundo.
    float worldX = static_cast<float>(m_chunkGridX * chunkSize);
    float worldZ = static_cast<float>(m_chunkGridZ * chunkSize);
    m_modelMatrix.setToIdentity(); // Reseta a matriz de modelo para identidade.
    m_modelMatrix.translate(worldX, 0.0f, worldZ); // Traduz a matriz de modelo para a nova posição no mundo.
}
