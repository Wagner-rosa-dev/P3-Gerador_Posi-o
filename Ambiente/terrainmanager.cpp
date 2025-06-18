#include "terrainmanager.h" // Inclui o cabeçalho da classe terrainmanager.
#include "chunkworker.h"    // Inclui o cabeçalho da classe ChunkWorker.
#include <QDebug>           // Para mensagens de depuração.
#include <cmath>            // Para funções matemáticas como std::floor.
#include <QThreadPool>      // Para gerenciar um pool de threads para tarefas em segundo plano.
#include "worldconfig.h"    // Inclui a estrutura WorldConfig para parâmetros do mundo.

/**
 * @brief Construtor da classe terrainmanager.
 *
 * Inicializa os membros da classe e configura o número máximo de threads
 * que o QThreadPool global pode usar para gerar chunks, deixando um núcleo
 * livre para a thread principal e o sistema operacional.
 */
terrainmanager::terrainmanager() :
    QObject(nullptr), // Chama o construtor da classe base QObject.
    m_centerChunkX(0), // Inicializa a coordenada X do chunk central da grade.
    m_centerChunkZ(0), // Inicializa a coordenada Z do chunk central da grade.
    m_lineQuadVaoRef(nullptr), // Inicializa a referência ao VAO das linhas.
    m_lineQuadVboRef(nullptr), // Inicializa a referência ao VBO das linhas.
    m_glFuncsRef(nullptr) // Inicializa a referência para as funções OpenGL.
{
    // Definimos o número máximo de threads que queremos usar para gerar chunks.
    // A recomendação é deixar 1 núcleo livre para a thread principal e o sistema operacional.
    QThreadPool::globalInstance()->setMaxThreadCount(3);
}

/**
 * @brief Destrutor da classe terrainmanager.
 *
 * Garante que todos os trabalhos de geração de chunk submetidos ao QThreadPool
 * sejam concluídos antes que o gerenciador de terreno seja destruído.
 */
terrainmanager::~terrainmanager()
{
    // Espera todos os trabalhos na piscina terminarem antes de fechar.
    QThreadPool::globalInstance()->waitForDone();
}

/**
 * @brief Inicializa o gerenciador de terreno.
 * @param config Ponteiro para a configuração do mundo.
 * @param terrainShaderProgram Ponteiro para o shader do terreno.
 * @param lineShaderProgram Ponteiro para o shader das linhas.
 * @param lineQuadVao Ponteiro para o VAO das linhas (compartilhado).
 * @param lineQuadVbo Ponteiro para o VBO das linhas (compartilhado).
 * @param glFuncs Ponteiro para as funções OpenGL.
 *
 * Armazena as referências necessárias, redimensiona a matriz de chunks
 * e inicia a grade de terreno centrada em (0,0).
 */
void terrainmanager::init(const WorldConfig* config, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo, QOpenGLFunctions *glFuncs) {
    m_config = config; // Armazena o ponteiro para a configuração do mundo.
    m_lineQuadVaoRef = lineQuadVao; // Armazena a referência para o VAO das linhas.
    m_lineQuadVboRef = lineQuadVbo; // Armazena a referência para o VBO das linhas.
    m_glFuncsRef = glFuncs; // Armazena a referência para as funções OpenGL.

    // Redimensiona a matriz de vetores para o tamanho da grade de renderização definida em m_config.
    m_chunks.resize(m_config->gridRenderSize);
    for (int i = 0; i < m_config->gridRenderSize; ++i) {
        m_chunks[i].resize(m_config->gridRenderSize);
    }
    // Inicia a grade de chunks centrada em (0, 0).
    recenterGrid(0, 0);
}

/**
 * @brief Atualiza o estado do terreno com base na posição da câmera.
 * @param cameraPos A posição atual da câmera no espaço do mundo.
 *
 * Esta função verifica se o centro da grade de chunks precisa ser atualizado (lógica de terreno infinito)
 * e também ajusta o Nível de Detalhe (LOD) dos chunks com base na distância da câmera,
 * disparando novos trabalhos de geração de malha se o LOD de um chunk precisar mudar.
 */
void terrainmanager::update(const QVector3D& cameraPos) {
    // Verifica se o centro da grade precisa mudar (lógica de terreno infinito):
    // Calcula em qual chunk a câmera está localizada no momento.
    int cameraChunkX = static_cast<int>(std::floor(cameraPos.x() / m_config->chunkSize));
    int cameraChunkZ = static_cast<int>(std::floor(cameraPos.z() / m_config->chunkSize));

    // Se a câmera se moveu para um novo chunk central, recentra a grade.
    if (cameraChunkX != m_centerChunkX || cameraChunkZ != m_centerChunkZ) {
        recenterGrid(cameraChunkX, cameraChunkZ);
    }

    // Atualiza o nível de detalhe (LOD) dos chunks existentes:
    for (int i = 0; i < m_config->gridRenderSize; ++i) {
        for (int j = 0; j < m_config->gridRenderSize; ++j){
            chunk& currentChunk = m_chunks[i][j]; // Obtém uma referência para o chunk atual.
            int currentLOD = currentChunk.getLOD(); // Obtém o LOD atual do chunk.
            int desiredLOD = currentLOD; // Inicializa o LOD desejado com o LOD atual.

            // Calcula a distância da câmera até o centro do chunk.
            float distanceToChunk = cameraPos.distanceToPoint(currentChunk.getCenterPosition(m_config->chunkSize));

            // Lógica de histerese para transição de LOD:
            // Isso evita que os chunks fiquem "pipocando" entre LODs quando a câmera está exatamente no limiar.
            if (currentLOD == 1 && distanceToChunk < m_config->lodDistanceThreshold - LOD_HYSTERESIS_BUFFER) {
                // Se o chunk está em baixa resolução (LOD 1), ele só muda para alta resolução (LOD 0)
                // se entrar bem na zona de alta resolução (distância menor que o limiar menos o buffer).
                desiredLOD = 0;
            } else if (currentLOD == 0 && distanceToChunk > m_config->lodDistanceThreshold + LOD_HYSTERESIS_BUFFER) {
                // Se o chunk está em alta resolução (LOD 0), ele só muda para baixa resolução (LOD 1)
                // se sair bem da zona de alta resolução (distância maior que o limiar mais o buffer).
                desiredLOD = 1;
            }

            // Se o LOD desejado for diferente do LOD atual, atualiza o chunk e dispara um novo trabalho de geração de malha.
            if (currentLOD != desiredLOD) {
                currentChunk.setLOD(desiredLOD); // Define o novo LOD para o chunk.
                // Define a nova resolução com base no LOD desejado.
                int newRes = (desiredLOD == 0) ? m_config->highRes : m_config->lowRes;
                // Cria um novo trabalho (QRunnable) para gerar a malha em uma thread separada.
                ChunkWorker* worker = new ChunkWorker(currentChunk.chunkGridX(), currentChunk.chunkGridZ(), newRes, m_config, this);
                // Submete o trabalho à piscina de threads. O Qt cuida do resto (execução em background).
                QThreadPool::globalInstance()->start(worker);
            }
        }
    }
}

/**
 * @brief Recentra a grade de chunks ao redor de uma nova posição central.
 * @param newCenterX A nova coordenada X do chunk central da grade.
 * @param newCenterZ A nova coordenada Z do chunk central da grade.
 *
 * Esta função é chamada quando a câmera se move para um novo chunk "central".
 * Ela reutiliza os objetos `chunk` existentes, atribuindo-lhes novas coordenadas
 * de grade e disparando a geração de novas malhas para essas posições.
 */
void terrainmanager::recenterGrid(int newCenterX, int newCenterZ){
    qInfo() << "Recentering grid to:" << newCenterX << "," << newCenterZ;
    m_centerChunkX = newCenterX; // Atualiza a coordenada X do centro da grade.
    m_centerChunkZ = newCenterZ; // Atualiza a coordenada Z do centro da grade.

    int halfGrid = m_config->gridRenderSize / 2; // Calcula a metade do tamanho da grade.

    // Itera sobre todas as posições na grade de chunks.
    for (int i = 0; i < m_config->gridRenderSize; ++i) {
        for (int j = 0; j < m_config->gridRenderSize; ++j) {
            // Calcula as coordenadas X e Z do chunk no mundo com base no novo centro.
            int chunkX = m_centerChunkX - halfGrid + i;
            int chunkZ = m_centerChunkZ - halfGrid + j;
            // Recicla o chunk na posição [i][j] da nossa matriz para a nova coordenada.
            m_chunks[i][j].recycle(chunkX, chunkZ, m_config->chunkSize);

            // Dispara um trabalho de geração de malha em segundo plano para este chunk.
            // A lógica de LOD inicializa todos os chunks com baixa resolução ao recentrar.
            m_chunks[i][j].setLOD(1); // Define o LOD inicial como baixa resolução.
            ChunkWorker* worker = new ChunkWorker(chunkX, chunkZ, m_config->lowRes, m_config, this);
            QThreadPool::globalInstance()->start(worker); // Submete o trabalho ao pool de threads.
        }
    }
}

/**
 * @brief Renderiza todos os chunks gerenciados.
 * @param terrainShaderProgram Ponteiro para o shader do terreno (pode ser nullptr).
 * @param lineShaderProgram Ponteiro para o shader das linhas (pode ser nullptr).
 * @param glFuncs Ponteiro para as funções OpenGL.
 *
 * Itera sobre todos os chunks na grade e chama seus métodos de renderização,
 * dependendo de qual shader foi fornecido (terreno ou linhas).
 */
void terrainmanager::render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions *glFuncs) {
    // Se o shader de terreno foi passado, desenhamos o terreno.
    if (terrainShaderProgram) {
        for (int i = 0; i < m_config->gridRenderSize; ++i) {
            for (int j = 0; j < m_config->gridRenderSize; ++j) {
                m_chunks[i][j].render(terrainShaderProgram, glFuncs);
            }
        }
    }
    // Se o shader de linha foi passado, desenhamos as bordas.
    if (lineShaderProgram) {
        for (int i = 0; i < m_config->gridRenderSize; ++i) {
            for (int j = 0; j < m_config->gridRenderSize; ++j) {
                // Passa as referências compartilhadas do VAO e VBO das linhas para o chunk renderizar suas bordas.
                m_chunks[i][j].renderBorders(lineShaderProgram, glFuncs, m_lineQuadVaoRef, m_lineQuadVboRef);
            }
        }
    }
}

/**
 * @brief Slot para receber a malha pronta de um ChunkWorker.
 * @param chunkX Coordenada X do chunk.
 * @param chunkZ Coordenada Z do chunk.
 * @param meshData A estrutura MeshData contendo a malha gerada.
 *
 * Este slot é chamado na thread principal (graças ao Qt::QueuedConnection)
 * e é responsável por armazenar a malha gerada no `chunk` correspondente.
 * O upload real para a GPU ocorrerá na próxima chamada a `render`.
 */
void terrainmanager::onMeshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData)
{
    //qInfo() << "main thread: recebido malha pronta para chunk" << chunkX << "," << chunkZ;

    // Calcula a posição do chunk na nossa grade interna (matriz m_chunks).
    int grid_i = (chunkX - m_centerChunkX) + m_config->gridRenderSize / 2;
    int grid_j = (chunkZ - m_centerChunkZ) + m_config->gridRenderSize / 2;

    // Verifica se o chunk ainda pertence à grade atual.
    // É possível que o chunk já tenha saído da área de renderização antes que sua malha fosse gerada.
    if(grid_i >= 0 && grid_i < m_config->gridRenderSize && grid_j >= 0 && grid_j < m_config->gridRenderSize) {
        chunk& targetChunk = m_chunks[grid_i][grid_j]; // Obtém uma referência para o chunk alvo.
        // Apenas armazena os dados da malha; o upload para a GPU (uploadMeshData) será feito em render().
        targetChunk.setPendingMeshData(meshData);
    }
}
