#include "myglwidget.h" // Inclui o cabeçalho da classe MyGLWidget.
#include <QOpenGLContext> // Inclui QOpenGLContext para obter funções OpenGL adicionais.
#include <QDebug> // Inclui QDebug para mensagens de depuração.
#include <QOpenGLShaderProgram> // Incluído novamente, embora já esteja no .h.
#include "noiseutils.h" // Inclui o cabeçalho NoiseUtils para funções de altura e normal do terreno.
#include <QKeyEvent> // Inclui QKeyEvent para lidar com eventos de teclado.
#include <QFile> // Inclui QFile para ler arquivos (ex: temperatura da CPU).
#include <QTextStream> // Inclui QTextStream para ler texto de arquivos.
#include <QTime> // Inclui QTime, embora QElapsedTimer seja preferido para medição de tempo.
#include <QElapsedTimer> // Inclui QElapsedTimer para medições de tempo precisas (FPS, temp).
#include <cmath> // Inclui cmath para funções matemáticas como sin, cos, etc.

// Constantes com o código GLSL dos shaders
// Estes blocos de string R"(...)" contêm o código-fonte GLSL para os shaders.
// Eles são compilados e linkados em tempo de execução.

// Shader de Vértices para o Terreno
const char* terrainVertexShaderSource = R"(#version 300 es
// Terrain Vertex Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

layout (location = 0) in vec3 a_position; // Atributo de entrada: posição do vértice (location 0).
layout (location = 1) in vec3 a_normal;   // Atributo de entrada: normal do vértice (location 1).

uniform mat4 projectionMatrix; // Matriz de projeção da câmera.
uniform mat4 viewMatrix;       // Matriz de visão da câmera.
uniform mat4 modelMatrix;      // Matriz de modelo do objeto (chunk).

out vec3 v_worldPos; // Saída para o fragment shader: posição do vértice no espaço do mundo.
out vec3 v_normal;   // Saída para o fragment shader: normal do vértice no espaço do mundo.

void main() {
    // Calcula a posição do vértice no espaço do mundo.
    // Multiplica a posição do vértice pela matriz de modelo para transformar do espaço do modelo para o espaço do mundo.
    vec4 worldPos4 = modelMatrix * vec4(a_position, 1.0);
    // Calcula a posição final do vértice no espaço de corte (clip space).
    // Multiplica pela matriz de projeção e visão para transformar do espaço do mundo para o espaço da tela.
    gl_Position = projectionMatrix * viewMatrix * worldPos4;
    v_worldPos = worldPos4.xyz; // Passa a posição do mundo para o fragment shader.
    // Calcula a normal no espaço do mundo, aplicando a parte de rotação da matriz de modelo
    // e normalizando o resultado para garantir que o vetor mantenha seu comprimento unitário.
    v_normal = normalize(mat3(modelMatrix) * a_normal);
}
)";

// Shader de Fragmentos para o Terreno
const char* terrainFragmentShaderSource = R"(#version 300 es
// Terrain Fragment Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

precision mediump float; // Define a precisão padrão para floats.

in vec3 v_worldPos; // Entrada do vertex shader: posição do fragmento no espaço do mundo.
in vec3 v_normal;   // Entrada do vertex shader: normal do fragmento no espaço do mundo.

out vec4 FragColor; // Saída: cor final do fragmento.

uniform vec3 lightDirection; // Direção da luz (geralmente do sol).
uniform vec3 lightColor;     // Cor da luz.
uniform vec3 objectBaseColor; // Cor base do objeto (terreno).

void main() {
    vec3 norm = normalize(v_normal); // Garante que a normal esteja normalizada.
    vec3 lightDir = normalize(-lightDirection); // A direção da luz é invertida para apontar *para* a luz.
    // Calcula a componente difusa da iluminação (Lambertian reflection).
    // `max` garante que a luz não seja subtraída quando a normal aponta para longe da luz.
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor; // Cor difusa resultante.

    float ambientStrength = 0.2; // Intensidade da luz ambiente.
    vec3 ambient = ambientStrength * lightColor; // Cor ambiente resultante.

    // Combina a luz ambiente e difusa e multiplica pela cor base do objeto.
    vec3 resultColor = (ambient + diffuse) * objectBaseColor;

    // Calcula um fator de altura para misturar cores baseadas na altura do terreno.
    // `clamp` limita o valor entre 0.0 e 1.0.
    float heightFactor = clamp(v_worldPos.y / 20.0, 0.0, 1.0);
    // Mistura a cor resultante com uma cor marrom/areia, tornando o terreno mais marrom em altitudes mais altas.
    resultColor = mix(resultColor, vec3(0.6, 0.5, 0.3), heightFactor * 0.5);

    FragColor = vec4(resultColor, 1.0); // Define a cor final do fragmento.
}
)";

// Shader de Vértices para Linhas (Bordas de Chunks)
const char* lineVertexShaderSource = R"(#version 300 es
// Line Vertex Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

layout (location = 0) in vec3 a_position; // Atributo de entrada: posição do vértice.

uniform mat4 projectionMatrix; // Matriz de projeção da câmera.
uniform mat4 viewMatrix;       // Matriz de visão da câmera.
uniform mat4 modelMatrix;      // Matriz de modelo do objeto (chunk).

void main() {
    // Eleva um pouco a posição do vértice para evitar z-fighting com o terreno.
    vec3 elevated_position = a_position + vec3(0.0, 0.2, 0.0);
    // Calcula a posição final do vértice no espaço de corte.
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(elevated_position, 1.0);
}
)";

// Shader de Fragmentos para Linhas
const char* lineFragmentShaderSource = R"(#version 300 es
// Line Fragment Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

precision mediump float; // Define a precisão padrão para floats.

out vec4 FragColor; // Saída: cor final do fragmento.

uniform vec4 lineColor; // Cor da linha, passada como uniforme.

void main() {
    FragColor = lineColor; // Define a cor final do fragmento como a cor da linha.
}
)";

// Shader de Vértices para o Trator
const char* tractorVertexShaderSource = R"(#version 300 es
layout (location = 0) in vec3 a_position; // Atributo de entrada: posição do vértice do trator.

uniform mat4 projectionMatrix; // Matriz de projeção da câmera.
uniform mat4 viewMatrix;       // Matriz de visão da câmera.
uniform mat4 modelMatrix;      // Matriz de modelo do trator.

void main() {
    // Calcula a posição final do vértice do trator no espaço de corte.
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position, 1.0);
}
)";

// Shader de Fragmentos para o Trator
const char* tractorFragmentShaderSource = R"(#version 300 es
precision mediump float; // Define a precisão padrão para floats.

out vec4 FragColor; // Saída: cor final do fragmento.

void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Define a cor do trator como vermelho sólido.
}
)";

/**
 * @brief Construtor da classe MyGLWidget.
 * @param parent O QWidget pai deste widget.
 *
 * Inicializa os membros da classe, configura um QTimer para o loop do jogo,
 * define a política de foco para eventos de teclado e configura o SpeedController
 * para receber dados de velocidade e direção da porta serial.
 */
MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), // Chama o construtor da classe base QOpenGLWidget.
    m_tractorRotation(0), // Inicializa a rotação do trator.
    m_extraFunction(nullptr), // Inicializa o ponteiro para funções extras OpenGL.
    m_tractorSpeed(0.0f), // Inicializa a velocidade do trator.
    m_steeringValue(50), // Inicializa o valor de direção (centro).
    m_hasReferenceCoordinate(false), // inicializa como falso
    m_currentHeading(0.0f), // rumo inicial
    m_kalmanFilter(nullptr)

{
    // Conecta o sinal `timeout` do `m_timer` ao slot `gameTick` deste objeto.
    // Isso garante que `gameTick` seja chamado periodicamente para atualizar a lógica do jogo.
    connect(&m_timer, &QTimer::timeout, this, &MyGLWidget::gameTick);
    // Inicia o timer para disparar a cada 16 milissegundos, o que corresponde a aproximadamente 60 quadros por segundo (1000ms / 16ms = 62.5 FPS).
    m_timer.start(16);


    // Nova lógica do controlador:
    // Cria uma nova instância de SpeedController.
    m_speedController = new SpeedController(this);
    // Conecta o sinal `speedUpdate` do `m_speedController` ao slot `onSpeedUpdate` deste objeto.
    // Isso permite que MyGLWidget receba as atualizações de velocidade.
    connect(m_speedController, &SpeedController::speedUpdate, this, &MyGLWidget::onSpeedUpdate);
    // Conecta o sinal `steeringUpdate` do `m_speedController` ao slot `onSteeringUpdate` deste objeto.
    // Isso permite que MyGLWidget receba as atualizações de direção.
    connect(m_speedController, &SpeedController::steeringUpdate, this, &MyGLWidget::onSteeringUpdate);

    connect(m_speedController, &SpeedController::gpsDataUpdate, this, &MyGLWidget::onGpsDataUpdate);
    // Inicia a escuta na porta serial especificada.
    // É importante verificar qual porta USB está sendo usada no Linux.
    m_speedController->startListening("/dev/ttyACM0");
}

/**
 * @brief Destrutor da classe MyGLWidget.
 *
 * Garante que o contexto OpenGL seja ativo antes de quaisquer operações de limpeza
 * e libera os recursos da GPU (VAOs, VBOs, shaders) chamando `doneCurrent()`.
 */
MyGLWidget::~MyGLWidget() {
    makeCurrent(); // Garante que o contexto OpenGL está ativo para limpeza.
    // Objetos QOpenGL* (shaders, buffers, vao) são limpos por seus destrutores.
    delete m_kalmanFilter;
    doneCurrent(); // Libera o contexto OpenGL.
}

/**
 * @brief Inicializa o ambiente OpenGL.
 *
 * Chamado uma vez quando o widget OpenGL é criado.
 * Configura as funções OpenGL, compila os shaders, inicializa VAOs/VBOs e
 * configura o TerrainManager.
 */
void MyGLWidget::initializeGL() {
    initializeOpenGLFunctions(); // Inicializa as funções OpenGL para o contexto atual.
    qInfo() << "MYGLWIDGET_CPP EXECUTANDO - VERSAO SUPER NOVA 04_06_2025_1530";
    // Obtém funções OpenGL extras (como UBOs) que podem não estar no perfil principal.
    m_extraFunction = QOpenGLContext::currentContext()->extraFunctions();
    if (!m_extraFunction) {
        qWarning("QOpenGLExtraFunctions not available. UBOs and other advanced features might not work.");
    }

    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade para que objetos mais próximos cubram os mais distantes.
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Define a cor de fundo (céu) como azul claro.

    qInfo() << "Compilando Terrain Shaders (Versão de Teste 04/06/2025)...";
    // Tenta adicionar e compilar o shader de vértice do terreno.
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, terrainVertexShaderSource)) {
        qWarning() << "Terrain Vertex Shader Compilation Error:" << m_terrainShaderProgram.log();
    }
    // Tenta adicionar e compilar o shader de fragmento do terreno.
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, terrainFragmentShaderSource)) {
        qWarning() << "Terrain Fragment Shader Compilation Error:" << m_terrainShaderProgram.log();
    }
    // Tenta linkar os shaders para criar o programa de shader.
    if (!m_terrainShaderProgram.link()) {
        qWarning() << "Terrain Shader Linker Error:" << m_terrainShaderProgram.log();
    } else {
        qInfo() << "Terrain Shaders linked successfully.";
    }

    qInfo() << "Compilando Line Shaders (Versão de Teste 04/06/2025)...";
    // Tenta adicionar e compilar o shader de vértice da linha.
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, lineVertexShaderSource)) {
        qWarning() << "Line Vertex Shader Compilation Error:" << m_lineShaderProgram.log();
    }
    // Tenta adicionar e compilar o shader de fragmento da linha.
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, lineFragmentShaderSource)) {
        qWarning() << "Line Fragment Shader Compilation Error:" << m_lineShaderProgram.log();
    }
    // Tenta linkar os shaders para criar o programa de shader da linha.
    if (!m_lineShaderProgram.link()) {
        qWarning() << "Line Shader Linker Error:" << m_lineShaderProgram.log();
    } else {
        qInfo() << "Line Shaders linked successfully.";
    }

    setupLineQuadVAO(); // Configura o VAO e VBO para o quad usado para desenhar as bordas dos chunks.
    setupTractorGL(); // Configura os shaders, VAO e VBO para o trator.
    // Inicializa o TerrainManager, passando a configuração do mundo, programas de shader e referências para objetos GL.
    m_terrainManager.init(&m_worldConfig, &m_terrainShaderProgram, &m_lineShaderProgram, &m_lineQuadVao, &m_lineQuadVbo, this);

    // Define a posição inicial do trator e ajusta sua altura ao terreno.
    m_tractorPosition = QVector3D(0.0f, 0.0f, 0.0f); //definindo uma posição inical
    m_tractorRotation = 0.0f; // sera atualizado pelo gps (rumo)

    m_frameCount = 0; // Zera o contador de quadros para cálculo de FPS.
    m_fpsTime.start(); // Inicia o timer para medição de FPS.
    m_tempReadTimer.start(); // Inicia o timer para leitura de temperatura.
}

/**
 * @brief Configura o Vertex Array Object (VAO) e Vertex Buffer Object (VBO) para desenhar um quad que representa as linhas das bordas dos chunks.
 *
 * Define as coordenadas dos vértices para um quad (na verdade, dois triângulos)
 * que formam a espessura da linha e o eleva ligeiramente para evitar z-fighting.
 */
void MyGLWidget::setupLineQuadVAO() {
    const float thickness = 0.10f; // Controla a espessura da linha.
    const float y_offset = 0.2f; // Pequena elevação em relação ao terreno para evitar z-fighting (artefato visual quando dois polígonos estão na mesma profundidade).

    const float s = m_worldConfig.chunkSize; // O tamanho do chunk, usado para dimensionar as linhas.
    const float t = thickness / 2.0f; // Metade da espessura para cálculos simétricos.

    // Define os vértices do quad da linha. São 12 floats (4 vértices * 3 componentes).
    // Originalmente 2 quads (4 triângulos) para as bordas de um chunk, mas o comentário indica 2 bordas, total de 12 vértices.
    // A lógica original dos vértices parece estar um pouco confusa ou incompleta para representar 4 bordas de um chunk.
    GLfloat lineQuadVertices[] = {
        // Borda Inferior (Z fixo) - representa a borda ao longo do eixo X
        0.0f, y_offset, -t,      s, y_offset, -t,       s, y_offset, t,    // Triângulo 1
        0.0f, y_offset, -t,      s, y_offset, t,        0.0f, y_offset, t,  // Triângulo 2
        // Lado 2 (X fixo) - representa a borda ao longo do eixo Z (Cuidado: valores atuais parecem incorretos para uma borda vertical)
        s-t, y_offset, 0.0f,     s+t, y_offset, 0.0f,   s+t, y_offset, s,
        s-t, y_offset, 0.0f,     s+t, y_offset, s,      s-t, y_offset, s,
    };

    m_lineQuadVao.create(); // Cria o VAO para as linhas.
    m_lineQuadVao.bind();   // Ativa o VAO.

    m_lineQuadVbo.create(); // Cria o VBO para os dados dos vértices das linhas.
    m_lineQuadVbo.bind();   // Ativa o VBO.
    // Aloca e copia os dados dos vértices para o VBO na GPU.
    m_lineQuadVbo.allocate(lineQuadVertices, sizeof(lineQuadVertices));

    // Configura o ponteiro do atributo de vértice (layout location 0) para as posições.
    if (m_lineShaderProgram.isLinked()) {
        m_lineShaderProgram.enableAttributeArray(0);
        m_lineShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    } else {
        qWarning("Line shader program not linked, cannot set attribute buffers for line quad VAO.");
    }
    m_lineQuadVao.release(); // Libera o VAO.
    m_lineQuadVbo.release(); // Libera o VBO.
}

/**
 * @brief Função de desenho principal do OpenGL.
 *
 * Chamado a cada quadro para renderizar a cena.
 * Atualiza a posição da câmera para seguir o trator, limpa o buffer de tela,
 * atualiza o terreno, renderiza o terreno e suas bordas, e renderiza o trator.
 * Também calcula e emite o FPS.
 */
void MyGLWidget::paintGL() {
    // Lógica da câmera inteligente (segue o trator):
    float distancia = 12.0f; // Distância da câmera em relação ao trator.
    float altura = 3.0f; // Altura da câmera em relação ao trator.

    // Calcula o ângulo do trator em radianos para determinar seu vetor 'para frente'.
    float angleRad = qDegreesToRadians(m_tractorRotation);
    // Cria um vetor que aponta para frente em relação ao trator (assumindo rotação em Y).
    QVector3D tractorForward(sin(angleRad), 0.0f, -cos(angleRad));

    // Calcula a posição da câmera: recua em relação ao trator e eleva.
    QVector3D cameraPos = m_tractorPosition - (tractorForward * distancia) + QVector3D(0.0f, altura, 0.0f);

    // Define o ponto para onde a câmera está olhando (ligeiramente acima do trator).
    QVector3D cameraTarget = m_tractorPosition + QVector3D(0.0f, 1.0f, 0.0f);

    // Atualiza a câmera para "olhar" do `cameraPos` para o `cameraTarget`.
    m_camera.lookAt(cameraPos, cameraTarget, QVector3D(0.0f, 1.0f, 0.0f));

    // Limpa os buffers de cor e profundidade antes de desenhar o novo quadro.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Verifica se os programas de shader estão linkados corretamente.
    bool terrainShaderOk = m_terrainShaderProgram.isLinked();
    bool lineShaderOk = m_lineShaderProgram.isLinked();

    // Atualiza o TerrainManager com a posição atual da câmera para gerenciar LOD e recentragem de chunks.
    m_terrainManager.update(m_camera.position());

    // Renderiza o terreno
    if (terrainShaderOk) {
        m_terrainShaderProgram.bind(); // Ativa o programa de shader do terreno.
        // Define os uniformes da matriz de projeção e visão para o shader do terreno.
        m_terrainShaderProgram.setUniformValue("projectionMatrix", m_camera.projectionMatrix());
        m_terrainShaderProgram.setUniformValue("viewMatrix", m_camera.viewMatrix());
        // Define a direção da luz (simulando o sol) e normaliza.
        QVector3D sunDirection = QVector3D(-0.5f, -1.0f, -0.5f).normalized();
        // Define a cor da luz (branco).
        m_terrainShaderProgram.setUniformValue("lightDirection", sunDirection);
        m_terrainShaderProgram.setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
        // Define a cor base do terreno (verde).
        m_terrainShaderProgram.setUniformValue("objectBaseColor", QVector3D(0.4f, 0.6f, 0.2f));

        // Pede para o TerrainManager renderizar apenas o terreno.
        m_terrainManager.render(&m_terrainShaderProgram, nullptr, this);

        m_terrainShaderProgram.release(); // Desativa o programa de shader do terreno.
    }

    // Renderiza as bordas dos chunks
    if (lineShaderOk) {
        m_lineShaderProgram.bind(); // Ativa o programa de shader da linha.
        // Define os uniformes da matriz de projeção e visão para o shader da linha.
        m_lineShaderProgram.setUniformValue("projectionMatrix", m_camera.projectionMatrix());
        m_lineShaderProgram.setUniformValue("viewMatrix", m_camera.viewMatrix());

        // Define a cor da linha (amarelo).
        m_lineShaderProgram.setUniformValue("lineColor", QColor(255, 255, 0, 255));

        // Pede para o TerrainManager renderizar apenas as bordas.
        m_terrainManager.render(nullptr, &m_lineShaderProgram, this);

        m_lineShaderProgram.release(); // Desativa o programa de shader da linha.
    }

    // Renderiza o trator
    if (m_tractorShaderProgram.isLinked()) {
        m_tractorShaderProgram.bind(); // Ativa o programa de shader do trator.
        QMatrix4x4 tractorModelMatrix; // Matriz de modelo para o trator.

        // Lógica de orientação avançada para alinhar o trator ao terreno:
        // Pega a normal do terreno na posição exata do trator.
        QVector3D terrainNormal = NoiseUtils::getNormal(m_tractorPosition.x(), m_tractorPosition.z());

        // Calcula o vetor 'para frente' do trator baseado na rotação atual.
        float angleRad = qDegreesToRadians(m_tractorRotation);
        QVector3D baseForward(sin(angleRad), 0.0f, -cos(angleRad));

        // Calcula os novos eixos de direção do trator, alinhados ao terreno.
        // `tractorUp` é a normal do terreno.
        QVector3D tractorUp = terrainNormal;
        // `tractorRight` é o produto vetorial de `baseForward` e `tractorUp`, normalizado.
        QVector3D tractorRight = QVector3D::crossProduct(baseForward, tractorUp).normalized();
        // `tractorForward` é o produto vetorial de `tractorUp` e `tractorRight`, normalizado.
        QVector3D tractorForward = QVector3D::crossProduct(tractorUp, tractorRight).normalized();

        // Cria a matriz de rotação a partir dos novos eixos.
        QMatrix4x4 rotationMatrix;
        rotationMatrix.setColumn(0, tractorRight.toVector4D()); // Eixo X do trator.
        rotationMatrix.setColumn(1, tractorUp.toVector4D());    // Eixo Y do trator.
        rotationMatrix.setColumn(2, tractorForward.toVector4D());// Eixo Z do trator.

        // Cria a matriz de translação para a posição do trator.
        QMatrix4x4 translationMatrix;
        translationMatrix.translate(m_tractorPosition);

        // Combina as matrizes de translação e rotação para formar a matriz de modelo final do trator.
        tractorModelMatrix = translationMatrix * rotationMatrix;
        // Fim da lógica de orientação.

        // Define os uniformes da matriz de projeção, visão e modelo para o shader do trator.
        m_tractorShaderProgram.setUniformValue("projectionMatrix", m_camera.projectionMatrix());
        m_tractorShaderProgram.setUniformValue("viewMatrix", m_camera.viewMatrix());
        m_tractorShaderProgram.setUniformValue("modelMatrix", tractorModelMatrix);

        m_tractorVao.bind(); // Ativa o VAO do trator.
        glDrawArrays(GL_TRIANGLES, 0, 3); // Desenha o trator (assumindo que é um triângulo simples com 3 vértices).
        m_tractorVao.release(); // Libera o VAO do trator.
    }

    // Lógica de cálculo de FPS:
    m_frameCount++; // Incrementa o contador de quadros.
    if (m_fpsTime.elapsed() >= 1000) { // Verifica se um segundo se passou.
        // Calcula o FPS: número de quadros dividido pelo tempo decorrido em segundos.
        float fps = m_frameCount / (m_fpsTime.elapsed() / 1000.0f);

        emit fpsUpdated(qRound(fps)); // Emite o sinal `fpsUpdated` com o FPS arredondado.

        m_frameCount = 0; // Reseta o contador de quadros.
        m_fpsTime.restart(); // Reinicia o timer de FPS.
    }

    // Verificação de erros OpenGL:
    // Loop para verificar e registrar quaisquer erros OpenGL que ocorreram durante a renderização.
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        qWarning() << "Erro no OpenGl em tempo de execução" << err;
    }
}

/**
 * @brief Chamado quando o widget é redimensionado.
 * @param w Nova largura do widget.
 * @param h Nova altura do widget.
 *
 * Ajusta a viewport OpenGL para corresponder ao novo tamanho do widget
 * e atualiza a matriz de projeção da câmera para refletir a nova razão de aspecto.
 */
void MyGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h); // Define a área de renderização na janela.
    // Atualiza a matriz de projeção da câmera com a nova razão de aspecto.
    m_camera.setPerspective(35.0f, static_cast<float>(w) / static_cast<float>(h > 0 ? h : 1), 0.1f, 1000.0f);
}

/**
 * @brief O slot gameTick é acionado periodicamente pelo QTimer.
 *
 * Contém a lógica de atualização da simulação, incluindo:
 * - Leitura da temperatura da CPU (em Linux).
 * - Lógica de direção do trator baseada no `m_steeringValue`.
 * - Lógica de movimento automático do trator baseada no `m_tractorSpeed`.
 * - Emissão de sinais de atualização da UI (velocidade em Km/h, coordenadas).
 * - Reagendamento da função paintGL() para redesenhar a cena.
 */
void MyGLWidget::gameTick() {
    // Lógica de leitura de temperatura da CPU (a cada 2 segundos):
    if (m_tempReadTimer.elapsed() >= 2000) { // Verifica se 2 segundos se passaram desde a última leitura.
        //qInfo() << "Timer de 2s atingido. tentando ler a temperatura..";
#ifdef Q_OS_LINUX // Esta seção de código só é compilada se o sistema operacional for Linux.
        QString tempFilePath = "/sys/class/thermal/thermal_zone0/temp"; // Caminho do arquivo para ler a temperatura da CPU no Linux.
        QFile tempFile(tempFilePath); // Cria um objeto QFile para interagir com o arquivo.

        if (tempFile.open(QIODevice::ReadOnly)) { // Tenta abrir o arquivo em modo somente leitura.
            QTextStream in(&tempFile); // Cria um QTextStream para ler texto do arquivo.
            QString line = in.readLine(); // Lê a primeira linha do arquivo.

            // Verifica se a linha lida não está vazia.
            if (line.trimmed().isEmpty()) {
                qWarning() << "Arquivo de temperatura esta vazio";
            } else {
                bool ok; // Variável para verificar se a conversão foi bem-sucedida.
                // Converte a string lida para float e divide por 1000 (o valor do arquivo é em miliCelsius).
                float temperature = line.toFloat(&ok) / 1000.0;
                if (ok) { // Se a conversão foi bem-sucedida.
                    //qInfo() << "Sucesso: leitura da temperatura concluida";
                    emit tempUpdated(temperature); // Emite o sinal `tempUpdated` com a temperatura.
                } else {
                    qWarning() << "Erro: nao foi possivel converter o conteudo'" << line << "'para numero";
                }
            }
            tempFile.close(); // Fecha o arquivo.
        } else {
            // Se não foi possível abrir o arquivo, registra um aviso.
            qWarning() << "erro: nao foi possivel abrir o arquivo de temperatura em:" << tempFilePath;
            qWarning() << "Verifique se o caminho esta correto para a sua placa";
        }
#endif
        m_tempReadTimer.restart(); // Reinicia o timer de leitura de temperatura.
    }

    // Calcula a velocidade em Km/h (velocidade em unidades/segundo * 3.6 para converter para Km/h).
    float speedkm = m_tractorSpeed * 3.6f;
    emit kmUpdated(speedkm); // Emite o sinal `kmUpdated` com a velocidade em Km/h.

    update();
}

/**
 * @brief Configura os shaders e os buffers (VAO, VBO) para renderizar o modelo do trator.
 *
 * Carrega e linka os shaders do trator e define os dados de vértice para um triângulo simples
 * que representa o trator.
 */
void MyGLWidget::setupTractorGL() {
    // Tenta adicionar e compilar os shaders de vértice e fragmento do trator.
    if (!m_tractorShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, tractorVertexShaderSource) ||
        !m_tractorShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, tractorFragmentShaderSource) ||
        !m_tractorShaderProgram.link()) {
        qWarning() << "Erro no shader do trator:" << m_tractorShaderProgram.log();
        return; // Retorna se houver erro na compilação ou linkagem.
    }

    // Define os vértices do trator como um triângulo simples apontando para o Z negativo.
    GLfloat tractorVertices[] = {0.0f, 0.5f, -0.75, // Vértice superior
                                 -0.5, 0.0f, 0.25f, // Vértice inferior esquerdo
                                 0.5f, 0.0f, 0.25f}; // Vértice inferior direito

    m_tractorVao.create(); // Cria o VAO para o trator.
    m_tractorVao.bind();   // Ativa o VAO.
    m_tractorVbo.create(); // Cria o VBO para os dados dos vértices do trator.
    m_tractorVbo.bind();   // Ativa o VBO.
    // Aloca e copia os dados dos vértices para o VBO na GPU.
    m_tractorVbo.allocate(tractorVertices, sizeof(tractorVertices));
    m_tractorShaderProgram.enableAttributeArray(0); // Habilita o atributo de posição (location 0) no shader.
    // Configura o ponteiro do atributo de vértice para as posições.
    m_tractorShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    m_tractorVao.release(); // Libera o VAO.
    m_tractorVbo.release(); // Libera o VBO.
}

/**
 * @brief Slot para receber atualizações de velocidade.
 * @param newSpeed A nova velocidade recebida do SpeedController.
 *
 * Atualiza a variável interna `m_tractorSpeed` com o valor recebido,
 * garantindo que o valor seja finito para evitar problemas.
 */
void MyGLWidget::onSpeedUpdate(float newSpeed)
{
    // Simplesmente atualiza a variável de velocidade da classe.
    // Verifica se o valor recebido é um número finito (não infinito ou NaN) para segurança.
    if (std::isfinite(newSpeed)) {
        m_tractorSpeed = newSpeed;
    } else {
        // Se receber um valor inválido, zera a velocidade para segurança e registra um aviso.
        m_tractorSpeed = 0.0f;
        qWarning() << "Recebido valor de velocidade invalido (inf ou nan)";
    }
}

/**
 * @brief Slot para receber atualizações de valor de esterçamento.
 * @param steeringValue O novo valor de esterçamento recebido do SpeedController.
 *
 * Atualiza a variável interna `m_steeringValue`.
 */
void MyGLWidget::onSteeringUpdate(int steeringValue) {
    m_steeringValue = steeringValue; // Atualiza o valor de esterçamento do trator.
}

void MyGLWidget::onGpsDataUpdate(const GpsData& data) {
    m_currentGpsData = data;

    if (!m_currentGpsData.isValid) {
        // se os dados nao sao validos, nao atualizamos a posição nem o status de moviemnto;
        // a logica de contenção de spam ja esta no speedcontroller
        qWarning() << "Dado GPS recebidos invalidos. Posição do trator nao atualizada.";
        return;
    }

    // a velocidade do trator agora bem do gps
    m_tractorSpeed = m_currentGpsData.speedKnots * 0.514444f; //convertendo nós para metros/segundo
    m_currentHeading = m_currentGpsData.courseOverGround; // rumo em graus

     //Converter coordenadas GPS para X/Z do mundo OpenGl
    //Usaremos a logica de referencia existente para calcular deltaX e deltaZ
    double deltaX_world = 0.0;
    double deltaZ_world = 0.0;

    if (!m_hasReferenceCoordinate) {
        m_referenceCoordinate = QGeoCoordinate(data.latitude, data.longitude);
        m_hasReferenceCoordinate = true;
        //Se é a primeira medição, inicializa o filtro de kalman na origem (0,0) do mundo 3D
        //com a primeira leitura do GPS
        m_kalmanFilter = new KalmanFilter(0.0, 0.0);
        qInfo() << "Coordenada de referencia GPS definida e Kalman filter inicializado";
    } else {
        QGeoCoordinate currentCoord(data.latitude, data.longitude);
        double distance = m_referenceCoordinate.distanceTo(currentCoord);
        double azimuth = m_referenceCoordinate.azimuthTo(currentCoord);
        double radAzimuth = qDegreesToRadians(azimuth);

        deltaX_world = distance * qSin(radAzimuth);
        deltaZ_world = -distance * qCos(radAzimuth);
    }

    //Aplicar filtro de Kalman
    if (m_kalmanFilter) { //Garante que o filtro foi inicializado
        //A fase de predição é chamada periodicamente pelo gameTick ou na chegaada de dados
        //mas é crucial que o dt seja o tempo real entre as atualizações do filtro
        //vamos usar o QDateTime para calcular o dt aqui
        double dt = m_lastGpsData.timestamp.msecsTo(m_currentGpsData.timestamp) / 1000.0;
        if (dt <= 0) dt = 0.016; // Garante um dt minimo se as timestamps forem iguais ou inferiores

        //Predição baseado na ultima estimativa, prevea o estado atual
        //isso é feito antes da atualização com a nova medição
        //No entanto, se voce esta chamando update em casa nova medição
        // e o predict é feito no loop principal (gametick), o dt precisa ser
        //o dt do gametick. Para um filtro de kalman baseado em evento (nova medição),
        //o predict deve usar o dt desde a ultima medição valida
        //o dt usado aqui deve ser o tempo real decorrido
        if (m_lastGpsData.isValid) { // Só prediz se houver uma medição anterior valida
            m_kalmanFilter->predict(dt); // predição do filtro de kalman
        } else {
            // se for a primeira medição valida, o filtro ja foi resetado com ela
            //nao fazemos predict ainda, pois nao ha estado anterior para prever
        }

        //Atualização: refine a previsão com a nova medição
        m_kalmanFilter->update(deltaX_world, deltaZ_world);

        //Obtenha a posição e velocidade suavizadas do filtro kalman
        QVector2D estimatedPosition = m_kalmanFilter->getStatePosition();
        QVector2D estimatedVelocity = m_kalmanFilter->getStateVelocity();

        m_tractorPosition.setX(estimatedPosition.x());
        m_tractorPosition.setZ(estimatedPosition.y()); // y do QVector2D para o Z do mundo
        //Se desejar usar a velocidade suavizada:
        m_tractorSpeed = estimatedVelocity.length();
        m_currentHeading = qRadiansToDegrees(qAtan2(estimatedVelocity.x(), -estimatedVelocity.y())); //Convertendo Vx, Vz para rumo
    } else {
        //Fallback: se o filtro nao estiver inicializado (primeira leitura), use a posição bruta
        m_tractorPosition.setX(static_cast<float>(deltaX_world));
        m_tractorPosition.setZ(static_cast<float>(deltaZ_world));
    }

    // Atualizar a altura Y do trator com base na posição X, Z do terreno
    // (Permanecerá 0.0f devido ao NoiseUtils::getHeight atual)
    m_tractorPosition.setY(NoiseUtils::getHeight(m_tractorPosition.x(), m_tractorPosition.z()));

    // A rotação do trator pode vir do rumo do GPS ou da velocidade estimada pelo Kalman.
    // Usaremos o rumo do GPS por simplicidade, ou o estimado pelo Kalman se for mais estável.
    // m_tractorRotation = -m_currentHeading; (se usar rumo do GPS)
    // Para usar o rumo estimado do Kalman, seria:
    m_tractorRotation = -qRadiansToDegrees(qAtan2(m_kalmanFilter->getStateVelocity().x(), -m_kalmanFilter->getStateVelocity().y()));


    // Verificar o status de movimento (linha reta/curva)
    // A lógica de checkMovementStatus pode se beneficiar dos dados suavizados do Kalman
    checkMovementStatus();

    // Armazena os dados atuais como ultimos dados para a proxima iteração
    m_lastGpsData = m_currentGpsData; // A timestamp da última medição é importante para o dt

    // Atualiza a tela para refletir as novas coordenadas
    // Emita as coordenadas *suavizadas* do Kalman para a UI
    emit coordinatesUpdate(m_kalmanFilter->getStatePosition().x(), m_kalmanFilter->getStatePosition().y()); // <--- CORREÇÃO: Ordem Lat/Lon e uso do Kalman
}

//coverte coordenadas GPS para X/Z do mundo
void MyGLWidget::updateTractorPositionFromGps(const GpsData& data) {
    if (!m_hasReferenceCoordinate) {
        m_referenceCoordinate = QGeoCoordinate(data.latitude, data.longitude);
        m_hasReferenceCoordinate = true;
        //centraliza o trator no ponto inicial do mundo para evitar offsets muito grandes
        m_tractorPosition.setX(0.0f);
        m_tractorPosition.setZ(0.0f);
        //qInfo() << "Coordenada de referencia GPS definida" << m_referenceCoordiante.latitude() << "," << m_referenceCoordinate.longitude();
    } else {
        QGeoCoordinate currentCoord(data.latitude, data.longitude);

        // Calcula a distância Leste-Oeste (X) e Norte-Sul (Z) do ponto de referência.
        // O QGeoCoordinate::distanceTo() retorna a distância em metros.
        // AzimuthTo() retorna o rumo (bearing) da coordenada atual para a referência (0=N, 90=E, 180=S, 270=W)
        // AzimuthFrom() retorna o rumo da referência para a coordenada atual.
        // Precisamos da distância em X e Z.

        //distancia total em metro
        double distance = m_referenceCoordinate.distanceTo(currentCoord);
        //Azimute de coordenada de referencia para coordenada atual
        double azimuth = m_referenceCoordinate.azimuthTo(currentCoord); // em graus
        // Convertendo distância e azimute para componentes X e Z
        // Assumindo que X aumenta para Leste e Z aumenta para Norte (ou diminui, dependendo do seu sistema)
        // No seu sistema OpenGL, -Z é "para frente" (Norte), +X é "direita" (Leste).
        // Um azimute de 0 (Norte) deveria levar a -Z.
        // Um azimute de 90 (Leste) deveria levar a +X.
        double radAzimuth = qDegreesToRadians(azimuth);

        // Componente X (Leste/Oeste)
        // Para Leste (90 deg), sin é 1. Para Oeste (270 deg), sin é -1.
        double deltaX = distance * qSin(radAzimuth);
        // Componente Z (Norte/Sul)
        // Para Norte (0 deg), cos é 1. Para Sul (180 deg), cos é -1.
        // Como o seu Z aponta para "trás" (ou -Z é "frente"), precisamos inverter o sinal
        double deltaZ = distance * qCos(radAzimuth);

        m_tractorPosition.setX(static_cast<float>(deltaX));
        m_tractorPosition.setZ(static_cast<float>(-deltaZ));
    }
}

//Verifica se o trator esta em linha reta ou fazendo curva
void MyGLWidget::checkMovementStatus() {
    //Requer pelo menos dois pontos de dados GPS para comaparar
    if (!m_lastGpsData.isValid || !m_currentGpsData.isValid) {
        emit movementStatusUpdated("Aguardando dados GPS...");
        return;
    }

    //limites de tolerancia
    const float SPEED_THRESHOLD = 0.5f; //Metros/segundos, abaixo disso é considerado parado
    const float HEADIND_CHANGE_THRESHOLD = 2.0f; // Graus, mudança maxima para ser considerado linha reta
    const float COORD_CHANGE_PROPORTIONALITY_THRESHOLD = 0.1f; // limite para proporicionalidade (idealmente baixo)

    //considerar parado se a velocidade for muito baixa
    if (m_tractorSpeed < SPEED_THRESHOLD) {
        emit movementStatusUpdated("Parado");
        return;
    }

    //Variação do rumo (heading)
    float headingDelta = m_currentGpsData.courseOverGround - m_lastGpsData.courseOverGround;
    //normalizar a variação para estar entre -180 e 180 graus
    if (headingDelta > 180.0f) headingDelta -= 360.0f;
    if (headingDelta < -180.0f) headingDelta += 360.0f;

    //Verificar se esta fazendo curva
    if (qAbs(headingDelta) > HEADIND_CHANGE_THRESHOLD) {
        emit movementStatusUpdated("Fazendo Curva!");
        return;
    }

    // Verificar proporcionalidade da mudança de latitude/longitude para linha reta
    // Isso é um pouco mais complexo e pode ser redundante se o rumo já for suficiente.
    // Para uma linha reta, a proporção entre a mudança de Latitude e a mudança de Longitude
    // deve ser aproximadamente constante (ou a variação de uma delas muito pequena se o movimento for alinhado a um eixo).
    // Ou, mais robusto: a direção do movimento (azimute entre pontos) deve ser muito próxima ao rumo.

    QGeoCoordinate lastCoord(m_lastGpsData.latitude, m_currentGpsData.longitude);
    QGeoCoordinate currentCoord(m_currentGpsData.latitude, m_currentGpsData.longitude);

    //calcula o azimute entre o ponto anterior e o ponto atual
    double pathAzimuth = lastCoord.azimuthTo(currentCoord);

    //diferaça entre o rumo do gps e o azimute do caminho real
    float azimuthDifference = qAbs(pathAzimuth - m_currentGpsData.courseOverGround);
    //normalizar a diferença
    if (azimuthDifference > 180.0f) azimuthDifference = 360.0f - azimuthDifference;

    //se a diferença entre o azimute do caminho e or umo do gps for pequena, e o rumo nao mudou muito
    if (azimuthDifference < COORD_CHANGE_PROPORTIONALITY_THRESHOLD) {
        //assume linha reta se o rumo nao mudou siginificamente e
        // a direção inserida pelos pontos esta alinhada com o rumo do GPS
        emit movementStatusUpdated("em linha reta");
    } else {
        // Se o rumo mudou pouco mas a proporcionalidade não se manteve,
        // pode ser ruído ou movimento muito sutil.
        // Para este caso, vamos considerar "Movimento Impreciso" ou continuar com a última determinação.
        // Por simplicidade, se não foi curva e não está perfeitamente alinhado,
        // podemos deixar a lógica do heading Delta decidir ou um estado de transição.
        // Ou simplesmente: se o heading Delta é pequeno, é linha reta. O azimute de pontos valida.
        emit movementStatusUpdated("Movimento!"); // Ou um status mais genérico se não for claramente curva ou reta
    }
}

