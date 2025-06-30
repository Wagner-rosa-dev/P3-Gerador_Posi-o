#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include "chunk.h"              // Inclui a definição da classe Chunk.
#include <vector>               // Para std::vector, usado para armazenar a grade de chunks.
#include <QVector3D>            // Para QVector3D, usado para a posição da câmera e cálculo de distância.
#include <QOpenGLShaderProgram> // Para QOpenGLShaderProgram, usado para os shaders de terreno e linha.
#include <QOpenGLFunctions>     // Para QOpenGLFunctions, para acesso às funções OpenGL.
#include <QThread>              // Incluído, mas QThreadPool é usado para gerenciamento de threads de worker.

// Declaração antecipada de ChunkWorker e WorldConfig
// Descrição: Usadas para evitar inclusões circulares e para declarar que terrainmanager
//            terá membros/usará ponteiros para essas classes/estruturas.
class ChunkWorker;
struct WorldConfig;

// Classe: terrainmanager
// Descrição: Gerencia a geração, atualização e renderização dos chunks de terreno.
//            Esta classe implementa a lógica de terreno infinito, LOD (Nível de Detalhe)
//            e delega a geração de malha para threads de worker para evitar bloqueios na UI.
//            É responsável por manter a grade de chunks centrada na câmera e por disparar
//            a recriação de malhas quando o LOD ou a posição do centro da grade muda.
class terrainmanager : public QObject {
    Q_OBJECT // Macro necessária para classes que usam sinais e slots do Qt.

public:
    // Construtor: terrainmanager
    // Descrição: Inicializa o gerenciador de terreno e configura o QThreadPool global
    //            para limitar o número de threads usadas para geração de chunks.
    terrainmanager();

    // Destrutor: ~terrainmanager
    // Descrição: Garante que todos os trabalhos pendentes no QThreadPool sejam concluídos
    //            antes que o gerenciador seja destruído.
    ~terrainmanager();

    // Método: init
    // Descrição: Inicializa o gerenciador de terreno com a configuração do mundo,
    //            shaders, e referências para VAO/VBO/funções OpenGL que podem ser
    //            compartilhadas para desenhar bordas dos chunks.
    // Parâmetros:
    //   - config: Ponteiro constante para a configuração do mundo (WorldConfig).
    //   - terrainShaderProgram: Ponteiro para o programa de shader de terreno.
    //   - lineShaderProgram: Ponteiro para o programa de shader de linha (para bordas).
    //   - lineQuadVao: Ponteiro para o VAO do quad de linha (compartilhado).
    //   - lineQuadVbo: Ponteiro para o VBO do quad de linha (compartilhado).
    //   - glFuncs: Ponteiro para as funções OpenGL, necessário para operações de GPU.
    void init(const WorldConfig* config, QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs);

    // Método: update
    // Descrição: Atualiza o estado do terreno com base na posição atual da câmera.
    //            Verifica se a grade de chunks precisa ser recentrada e se o LOD
    //            dos chunks precisa ser ajustado, disparando novos trabalhos de geração de malha.
    // Parâmetros:
    //   - cameraPos: A posição atual da câmera no espaço do mundo.
    void update(const QVector3D& cameraPos);

    // Método: render
    // Descrição: Renderiza todos os chunks gerenciados, usando os shaders fornecidos.
    //            Esta função também lida com o upload de dados de malha pendentes para a GPU.
    // Parâmetros:
    //   - terrainShaderProgram: Ponteiro para o programa de shader de terreno (pode ser nullptr para renderizar apenas bordas).
    //   - lineShaderProgram: Ponteiro para o programa de shader de linha (pode ser nullptr para renderizar apenas terreno).
    //   - glFuncs: Ponteiro para as funções OpenGL.
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs);

private slots:
    // Slot Privado: onMeshReady
    // Descrição: Slot que recebe os dados de malha gerados por um `ChunkWorker` em uma thread separada.
    //            É chamado via `QMetaObject::invokeMethod` com `Qt::QueuedConnection`,
    //            garantindo que seja executado na thread do `terrainmanager` (a thread principal),
    //            onde as operações OpenGL podem ser realizadas com segurança.
    // Parâmetros:
    //   - chunkX: Coordenada X do chunk para o qual a malha foi gerada.
    //   - chunkZ: Coordenada Z do chunk para o qual a malha foi gerada.
    //   - meshData: A estrutura `chunk::MeshData` contendo os vértices e índices prontos.
    void onMeshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData);

private:
    // Método Privado: recenterGrid
    // Descrição: Recentra a grade de chunks ao redor de uma nova posição central.
    //            Isso envolve reciclar chunks existentes e disparar a geração de novas malhas
    //            para os chunks recém-visíveis.
    // Parâmetros:
    //   - newCenterX: A nova coordenada X do chunk central da grade.
    //   - newCenterZ: A nova coordenada Z do chunk central da grade.
    void recenterGrid(int newCenterX, int newCenterZ);

    // Membro: m_config
    // Tipo: const WorldConfig*
    // Descrição: Ponteiro constante para a configuração global do mundo.
    const WorldConfig* m_config;

    // Membro: m_chunks
    // Tipo: std::vector<std::vector<chunk>>
    // Descrição: Uma matriz 2D (vetor de vetores) que armazena os objetos `chunk`
    //            atualmente visíveis e sendo gerenciados. Representa a grade de chunks.
    std::vector<std::vector<chunk>> m_chunks;

    // Membro: m_centerChunkX
    // Tipo: int
    // Descrição: A coordenada X da grade do chunk que está atualmente no centro da grade de renderização.
    int m_centerChunkX;

    // Membro: m_centerChunkZ
    // Tipo: int
    // Descrição: A coordenada Z da grade do chunk que está atualmente no centro da grade de renderização.
    int m_centerChunkZ;


    // Membro: m_glFuncsRef
    // Tipo: QOpenGLFunctions*
    // Descrição: Referência (ponteiro) para as funções OpenGL, obtidas do contexto OpenGL principal.
    //            Usado para realizar operações OpenGL dentro da classe.
    QOpenGLFunctions* m_glFuncsRef;

    // Membro: LOD_HYSTERESIS_BUFFER
    // Tipo: const float
    // Descrição: Um valor de buffer usado na lógica de transição de LOD para evitar
    //            "trepidação" visual quando a câmera está exatamente na distância de transição.
    //            Ele cria uma pequena zona de histerese, onde o LOD só muda se a distância
    //            cruzar o limite mais o buffer.
    const float LOD_HYSTERESIS_BUFFER = 5.0f;
};

#endif // TERRAINMANAGER_H
