#ifndef CHUNK_H
#define CHUNK_H

#include <QVector3D>          // Para representar vértices e normais 3D.
#include <QOpenGLBuffer>      // Para gerenciar buffers de vértices e índices na GPU.
#include <QOpenGLVertexArrayObject> // Para gerenciar Vertex Array Objects (VAOs).
#include <QOpenGLShaderProgram> // Para gerenciar programas de shader OpenGL.
#include <QOpenGLExtraFunctions> // Para acesso a funções OpenGL que podem não estar no perfil principal (Ex: glDrawElementsBaseVertex).
#include <QMatrix4x4>         // Para a matriz de modelo do chunk.
#include <vector>             // Para armazenar os dados de vértices e índices.
#include <utility>            // Para std::move (semântica de movimento).
#include <memory>             // Para std::unique_ptr (gerenciamento de memória de objetos OpenGL).
#include <QTimer>             // Incluído mas não utilizado diretamente no chunk.h. Pode ser resquício ou para futura expansão.
#include <QKeyEvent>          // Incluído mas não utilizado diretamente no chunk.h. Pode ser resquício ou para futura expansão.

// Estrutura: Vertex
// Descrição: Define a estrutura de um único vértice para o terreno.
//            Contém a posição 3D do vértice e seu vetor normal para iluminação.
struct Vertex {
    // Membro: position
    // Tipo: QVector3D
    // Descrição: As coordenadas X, Y e Z do vértice no espaço do modelo do chunk.
    QVector3D position;
    // Membro: normal
    // Tipo: QVector3D
    // Descrição: O vetor normal da superfície neste vértice. Usado para cálculos de iluminação.
    QVector3D normal;
};

// Classe: chunk
// Descrição: Representa um pedaço (chunk) do terreno 3D. Cada chunk gerencia sua própria
//            geometria (vértices e índices), Nível de Detalhe (LOD) e estado de renderização.
//            Ele é responsável por gerar seus próprios dados de malha na CPU e fazer o upload
//            para a GPU quando necessário.
class chunk {
public:
    // Estrutura: MeshData
    // Descrição: Uma estrutura de dados para empacotar e transportar os dados de malha
    //            (vértices e índices) entre threads, especificamente da thread de worker
    //            para a thread principal para upload na GPU.
    struct MeshData {
        // Membro: chunkGridX
        // Tipo: int
        // Descrição: Coordenada X do chunk na grade lógica do terreno.
        int chunkGridX;
        // Membro: chunkGridZ
        // Tipo: int
        // Descrição: Coordenada Z do chunk na grade lógica do terreno.
        int chunkGridZ;
        // Membro: vertices
        // Tipo: std::vector<Vertex>
        // Descrição: Um vetor contendo todos os vértices da malha para este chunk.
        std::vector<Vertex> vertices;
        // Membro: indices
        // Tipo: std::vector<GLuint>
        // Descrição: Um vetor contendo os índices que definem a ordem de desenho dos vértices
        //            para formar triângulos.
        std::vector<GLuint> indices;
        // Membro: resolution
        // Tipo: int
        // Descrição: A resolução atual com a qual a malha foi gerada (número de vértices por lado).
        int resolution;
    };

    // Construtor padrão: chunk
    // Descrição: Inicializa os membros do chunk com valores padrão.
    chunk();
    // Destrutor: ~chunk
    // Descrição: Libera os recursos alocados pelo chunk, como os objetos OpenGL.
    ~chunk();

    // ----- Semântica de Movimento e Cópia ------

    // Construtor de Movimento: chunk(chunk&& other)
    // Descrição: Permite que objetos `chunk` sejam movidos de forma eficiente, transferindo
    //            a propriedade dos recursos OpenGL (VAO, VBO, EBO) de um objeto temporário
    //            ou expirado para um novo, evitando cópias desnecessárias e caras.
    // Parâmetros:
    //   - other: O objeto `chunk` do qual os recursos serão movidos.
    chunk(chunk&& other) noexcept(true);

    // Operador de Atribuição por Movimento: operator=(chunk&& other)
    // Descrição: Permite a atribuição de um objeto `chunk` a outro via movimento,
    //            liberando os recursos do objeto de destino e assumindo a propriedade
    //            dos recursos do objeto de origem.
    // Parâmetros:
    //   - other: O objeto `chunk` do qual os recursos serão movidos.
    // Retorno: chunk& - Uma referência ao objeto `chunk` atual (this).
    chunk& operator=(chunk&& other) noexcept(true);

    // Impedir Cópias (Deletadas):
    // Descrição: Impede que objetos `chunk` sejam copiados. Isso é crucial para classes
    //            que gerenciam recursos OpenGL (como VAOs e VBOs), pois eles não podem
    //            ser copiados de forma trivial e devem ser movidos ou ter sua propriedade
    //            explicitamente transferida.
    chunk(const chunk& other) = delete;
    chunk& operator=(const chunk& other) = delete;

    // Método: init (DEPRECATED - Não mais usado para inicialização principal, substituído por recycle e workers)
    // Descrição: Anteriormente usado para inicializar o chunk com suas coordenadas de grade e contexto OpenGL.
    //            Agora, a inicialização e atualização da malha são tratadas por `recycle` e `generateMeshData`
    //            em conjunto com `uploadMeshData` através de threads.
    // Parâmetros:
    //   - cX: Coordenada X do chunk na grade.
    //   - cZ: Coordenada Z do chunk na grade.
    //   - glFuncs: Ponteiro para as funções OpenGL.
    void init(int cX, int cZ, QOpenGLFunctions *glFuncs);

    // Método: recycle
    // Descrição: Reutiliza um objeto chunk existente, redefinindo suas coordenadas de grade
    //            e matriz de modelo. Isso é usado para otimizar a criação de chunks,
    //            evitando alocações e liberações desnecessárias.
    // Parâmetros:
    //   - cX: Nova coordenada X do chunk na grade.
    //   - cZ: Nova coordenada Z do chunk na grade.
    //   - chunkSize: O tamanho do chunk, usado para calcular a posição no mundo.
    void recycle(int cX, int cZ, int chunkSize);

    // Método Estático: generateMeshData
    // Descrição: Uma função estática que gera os dados de vértices e índices para a malha de um chunk.
    //            Esta é uma operação que consome CPU e é projetada para ser executada em uma
    //            thread de worker para não bloquear a thread principal da UI/renderização.
    // Parâmetros:
    //   - cX: Coordenada X do chunk na grade.
    //   - cZ: Coordenada Z do chunk na grade.
    //   - resolution: A resolução da malha a ser gerada (número de vértices por lado).
    //   - chunkSize: O tamanho do chunk, usado para calcular posições no mundo.
    // Retorno: MeshData - Uma estrutura contendo os dados de vértices e índices gerados.
    static MeshData generateMeshData(int cX, int cZ, int resolution, int chunkSize);

    // Método: uploadMeshData
    // Descrição: Faz o upload dos dados de malha (vértices e índices) para a GPU,
    //            criando e configurando os VAOs, VBOs e EBOs. Esta função DEVE
    //            ser chamada na thread que possui o contexto OpenGL ativo (geralmente a thread principal).
    // Parâmetros:
    //   - data: A estrutura MeshData contendo os vértices e índices a serem enviados para a GPU.
    //   - glFuncs: Ponteiro para as funções OpenGL.
    void uploadMeshData(const MeshData& data, QOpenGLFunctions* glFuncs);

    // Método: render
    // Descrição: Desenha o chunk na tela usando o shader de terreno fornecido.
    //            Antes de desenhar, verifica se há dados de malha pendentes para upload.
    // Parâmetros:
    //   - terrainShaderProgram: O programa de shader OpenGL a ser usado para renderizar o terreno.
    //   - glFuncs: Ponteiro para as funções OpenGL.
    void render(QOpenGLShaderProgram* terrainShaderProgram, QOpenGLFunctions *glFuncs);

    // Método: renderBorders
    // Descrição: Desenha as bordas do chunk na tela usando o shader de linha fornecido.
    //            Útil para visualização da grade de chunks.
    // Parâmetros:
    //   - lineShaderProgram: O programa de shader OpenGL a ser usado para renderizar as linhas.
    //   - glFuncs: Ponteiro para as funções OpenGL.
    //   - lineQuadVao: O VAO para o quad da linha (normalmente compartilhado entre chunks).
    //   - lineQuadVbo: O VBO para o quad da linha (normalmente compartilhado entre chunks).
    void renderBorders(QOpenGLShaderProgram* lineShaderProgram, QOpenGLFunctions* glFuncs, QOpenGLVertexArrayObject* lineQuadVao, QOpenGLBuffer* lineQuadVbo);

    // Método: setLOD
    // Descrição: Define o Nível de Detalhe (LOD) atual para o chunk.
    //            Um LOD menor geralmente significa maior resolução.
    // Parâmetros:
    //   - lodLevel: O nível de LOD a ser definido (e.g., 0 para alta resolução, 1 para baixa).
    void setLOD(int lodLevel);

    // Método: getLOD
    // Descrição: Retorna o Nível de Detalhe (LOD) atual do chunk.
    // Retorno: int - O nível de LOD.
    int getLOD() const {return m_currentLOD; }

    // Método: getCenterPosition
    // Descrição: Calcula e retorna a posição central do chunk no espaço do mundo.
    // Parâmetros:
    //   - chunkSize: O tamanho do chunk.
    // Retorno: QVector3D - A posição central do chunk.
    QVector3D getCenterPosition(int chunkSize) const;

    // Método: modelMatrix
    // Descrição: Retorna a matriz de modelo do chunk. Esta matriz posiciona e orienta
    //            o chunk no espaço do mundo.
    // Retorno: QMatrix4x4 - A matriz de modelo.
    QMatrix4x4 modelMatrix() const { return m_modelMatrix; }

    // Método: chunkGridX
    // Descrição: Retorna a coordenada X do chunk na grade lógica.
    // Retorno: int - Coordenada X.
    int chunkGridX() const { return m_chunkGridX; }

    // Método: chunkGridZ
    // Descrição: Retorna a coordenada Z do chunk na grade lógica.
    // Retorno: int - Coordenada Z.
    int chunkGridZ() const { return m_chunkGridZ; }

    // Método: setPendingMeshData
    // Descrição: Armazena os dados de malha recebidos de uma thread de worker.
    //            Esses dados serão enviados para a GPU na próxima chamada a `render`.
    // Parâmetros:
    //   - data: A estrutura MeshData contendo os dados gerados pela thread de worker.
    void setPendingMeshData(const MeshData& data);

private:
    // Membro: m_chunkGridX
    // Tipo: int
    // Descrição: Coordenada X do chunk na grade lógica do terreno.
    int m_chunkGridX;
    // Membro: m_chunkGridZ
    // Tipo: int
    // Descrição: Coordenada Z do chunk na grade lógica do terreno.
    int m_chunkGridZ;

    // Membro: m_indexCount
    // Tipo: int
    // Descrição: O número total de índices usados para desenhar a malha do chunk.
    int m_indexCount;
    // Membro: m_vertexCount
    // Tipo: int
    // Descrição: O número total de vértices na malha do chunk.
    int m_vertexCount;

    // Membro: m_currentResolution
    // Tipo: int
    // Descrição: A resolução (número de vértices por lado) com a qual a malha atual foi gerada.
    int m_currentResolution;
    // Membro: m_currentLOD
    // Tipo: int
    // Descrição: O Nível de Detalhe (LOD) atual do chunk (e.g., 0 para alta, 1 para baixa).
    int m_currentLOD;

    // Membro: m_vao
    // Tipo: std::unique_ptr<QOpenGLVertexArrayObject>
    // Descrição: Um ponteiro único para o Vertex Array Object (VAO) deste chunk.
    //            VAOs encapsulam a configuração de atributos de vértice para simplificar o desenho.
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    // Membro: m_vbo
    // Tipo: std::unique_ptr<QOpenGLBuffer>
    // Descrição: Um ponteiro único para o Vertex Buffer Object (VBO) deste chunk.
    //            VBOs armazenam os dados de vértice (posições, normais, etc.) na GPU.
    std::unique_ptr<QOpenGLBuffer> m_vbo;
    // Membro: m_ebo
    // Tipo: std::unique_ptr<QOpenGLBuffer>
    // Descrição: Um ponteiro único para o Element Buffer Object (EBO) ou Index Buffer Object (IBO) deste chunk.
    //            EBOs armazenam os índices de vértice na GPU, permitindo reutilizar vértices.
    std::unique_ptr<QOpenGLBuffer> m_ebo;

    // Membro: m_modelMatrix
    // Tipo: QMatrix4x4
    // Descrição: A matriz de modelo que posiciona, escala e orienta o chunk no espaço do mundo.
    QMatrix4x4 m_modelMatrix;

    // Membro: m_hasPendingMesh
    // Tipo: bool
    // Descrição: Sinaliza se há dados de malha novos (gerados por uma thread de worker)
    //            que precisam ser enviados para a GPU.
    bool m_hasPendingMesh;
    // Membro: m_pendingMeshData
    // Tipo: MeshData
    // Descrição: Armazena temporariamente os dados de malha gerados por uma thread de worker
    //            antes que possam ser enviados para a GPU na thread principal.
    MeshData m_pendingMeshData;
};

#endif // CHUNK_H
