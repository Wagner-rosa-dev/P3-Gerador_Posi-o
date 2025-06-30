#ifndef TERRAINGRID_H
#define TERRAINGRID_H

#include <QOpenGLBuffer>      // Para gerenciar buffers de vértices/índices na GPU.
#include <QOpenGLVertexArrayObject> // Para gerenciar Vertex Array Objects (VAOs).
#include <QOpenGLShaderProgram> // Para gerenciar programas de shader OpenGL.
#include <QOpenGLFunctions>     // Para acesso a funções OpenGL.
#include <QMatrix4x4>         // Para a matriz de modelo do grid.
#include <QVector3D>          // Para representar a posição e outros vetores.
#include <vector>             // Para armazenar os dados de vértices.

// Declaração antecipada para evitar inclusão circular, se necessário
struct WorldConfig;

// Classe: TerrainGrid
// Descrição: Gerencia e renderiza uma grade contínua no chão do mundo 3D.
//            Esta grade se estenderá por toda a área de terreno visível,
//            e suas propriedades (tamanho do quadrado, espessura da linha)
//            são configuráveis via WorldConfig.
class TerrainGrid {
public:
    // Construtor: TerrainGrid
    // Descrição: Inicializa os membros da classe.
    TerrainGrid();

    // Destrutor: ~TerrainGrid
    // Descrição: Libera os recursos alocados pelo grid, como os objetos OpenGL.
    ~TerrainGrid();

    // Método: init
    // Descrição: Inicializa os recursos OpenGL do grid (VAO, VBO) e armazena
    //            referências para as funções GL e configurações do mundo.
    // Parâmetros:
    //   - config: Ponteiro constante para a configuração do mundo (WorldConfig).
    //   - glFuncs: Ponteiro para as funções OpenGL, necessário para operações de GPU.
    void init(const WorldConfig* config, QOpenGLFunctions* glFuncs);

    // Método: updateGridGeometry
    // Descrição: Recalcula e faz o upload dos vértices do grid para a GPU.
    //            Isso é chamado quando a área visível do terreno muda ou
    //            quando o grid precisa se recentrar na câmera.
    // Parâmetros:
    //   - currentCameraWorldX: A coordenada X da câmera no mundo.
    //   - currentCameraWorldZ: A coordenada Z da câmera no mundo.
    //   - terrainRenderSizeChunks: O número de chunks (largura/altura) da área de terreno visível.
    void updateGridGeometry(float currentCameraWorldX, float currentCameraWorldZ, int terrainRenderSizeChunks);

    // Método: render
    // Descrição: Desenha a grade na tela usando o shader de linha fornecido.
    // Parâmetros:
    //   - lineShaderProgram: O programa de shader OpenGL a ser usado para renderizar as linhas.
    //   - viewMatrix: A matriz de visão atual da câmera.
    //   - projectionMatrix: A matriz de projeção atual da câmera.
    void render(QOpenGLShaderProgram* lineShaderProgram, const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix);

private:
    // Membro: m_vao
    // Tipo: QOpenGLVertexArrayObject
    // Descrição: O Vertex Array Object (VAO) para os vértices do grid.
    QOpenGLVertexArrayObject m_vao;

    // Membro: m_vbo
    // Tipo: QOpenGLBuffer
    // Descrição: O Vertex Buffer Object (VBO) que armazena os dados de vértices do grid.
    QOpenGLBuffer m_vbo;

    // Membro: m_vertexCount
    // Tipo: int
    // Descrição: O número total de vértices no VBO do grid.
    int m_vertexCount;

    // Membro: m_glFuncsRef
    // Tipo: QOpenGLFunctions*
    // Descrição: Ponteiro para as funções OpenGL, obtidas do contexto OpenGL principal.
    QOpenGLFunctions* m_glFuncsRef;

    // Membro: m_config
    // Tipo: const WorldConfig*
    // Descrição: Ponteiro constante para a configuração global do mundo.
    const WorldConfig* m_config;

    // Membro: m_lastCenterGridX
    // Tipo: int
    // Descrição: Última coordenada X do chunk central em que o grid foi atualizado.
    int m_lastCenterGridX;

    // Membro: m_lastCenterGridZ
    // Tipo: int
    // Descrição: Última coordenada Z do chunk central em que o grid foi atualizado.
    int m_lastCenterGridZ;

    // Membro: m_modelMatrix
    // Tipo: QMatrix4x4
    // Descrição: Matriz de modelo para posicionar o grid no mundo.
    QMatrix4x4 m_modelMatrix;
};

#endif // TERRAINGRID_H
