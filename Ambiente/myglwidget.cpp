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
#include "logger.h"
#include "terraingrid.h"
#include <QPainter>


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
MyGLWidget::MyGLWidget(const WorldConfig& config, QWidget *parent)
    : QOpenGLWidget(parent), // Chama o construtor da classe base QOpenGLWidget.
    m_worldConfig(config),
    m_tractorRotation(0), // Inicializa a rotação do trator.
    m_extraFunction(nullptr), // Inicializa o ponteiro para funções extras OpenGL.
    m_tractorCurrentSpeed(0.0f),
    m_tractorTargetSpeed(0.0f),
    m_steeringAngle(0.0f),
    m_tractorSpeed(0.0f), // Inicializa a velocidade do trator.
    m_steeringValue(50), // Inicializa o valor de direção (centro).
    m_hasReferenceCoordinate(false), // inicializa como falso
    m_currentHeading(0.0f), // rumo inicial
    m_immFilter(nullptr)

{
    m_immFilter = new immfilter();
    // Conecta o sinal `timeout` do `m_timer` ao slot `gameTick` deste objeto.
    // Isso garante que `gameTick` seja chamado periodicamente para atualizar a lógica do jogo.
    connect(&m_timer, &QTimer::timeout, this, &MyGLWidget::gameTick);
    // Inicia o timer para disparar a cada 16 milissegundos, o que corresponde a aproximadamente 60 quadros por segundo (1000ms / 16ms = 62.5 FPS).
    m_timer.start(16);


#ifdef USE_LIVE_GPS
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

#else
    // Lógica para reprodução de arquivo GPS (GpsFilePlayer)
    m_speedController = nullptr; // Garante que o ponteiro não aponte para lixo se não for usado
    m_gpsFilePlayer = new GpsFilePlayer(this);
    connect(m_gpsFilePlayer, &GpsFilePlayer::gpsDataUpdate, this, &MyGLWidget::onGpsDataUpdate);
    connect(m_gpsFilePlayer, &GpsFilePlayer::playbackFinished, this, [](){
        MY_LOG_INFO("GPS_Input", "Reprodução do arquivo GPS concluída.");
        // Opcional: Adicione lógica aqui para lidar com o fim da reprodução (e.g., reiniciar, parar o app)
    });

    m_gpsFilePlayer->startPlayback("/home/root/GPSTEXT.txt", 500); // 100ms para simular um GPS de 10Hz
    MY_LOG_INFO("GPS_Input", "Usando reprodução de arquivo GPS (GpsFilePlayer).");
#endif

    m_gameTickTimer.start();
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
    delete m_immFilter;
    m_immFilter = nullptr;
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
    MY_LOG_INFO("Render", "MYGLWIDGET_CPP EXECUTANDO - VERSAO SUPER NOVA 04_06_2025_1530");
    // Obtém funções OpenGL extras (como UBOs) que podem não estar no perfil principal.
    m_extraFunction = QOpenGLContext::currentContext()->extraFunctions();
    if (!m_extraFunction) {
        MY_LOG_WARNING("Render", "QOpenGLExtraFunctions not available. UBOs and other advanced features might not work.");
    }

    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade para que objetos mais próximos cubram os mais distantes.
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Define a cor de fundo (céu) como azul claro.

    MY_LOG_INFO("Render", "Compilando Terrain Shaders (Versão de Teste 30/06/2025)...");
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, terrainVertexShaderSource)) {
        MY_LOG_ERROR("Render", QString("Terrain Vertex Shader Compilation Error: %1").arg(m_terrainShaderProgram.log()));
    }
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, terrainFragmentShaderSource)) {
        MY_LOG_ERROR("Render", QString("Terrain Fragment Shader Compilation Error: %1").arg(m_terrainShaderProgram.log()));
    }
    if (!m_terrainShaderProgram.link()) {
        MY_LOG_ERROR("Render", QString("Terrain Shader Linker Error: %1").arg(m_terrainShaderProgram.log()));
    } else {
        MY_LOG_INFO("Render", "Terrain Shaders linked successfully.");
    }

    MY_LOG_INFO("Render", "Compilando Line Shaders (Versão de Teste 04/06/2025)...");
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, lineVertexShaderSource)) {
        MY_LOG_ERROR("Render", QString("Line Vertex Shader Compilation Error: %1").arg(m_lineShaderProgram.log()));
    }
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, lineFragmentShaderSource)) {
        MY_LOG_ERROR("Render", QString("Line Fragment Shader Compilation Error: %1").arg(m_lineShaderProgram.log()));
    }
    if (!m_lineShaderProgram.link()) {
        MY_LOG_ERROR("Render", QString("Line Shader Linker Error: %1").arg(m_lineShaderProgram.log()));
    } else {
        MY_LOG_INFO("Render", "Line Shaders linked successfully.");
    }

    m_terrainGrid.init(&m_worldConfig, this);
    setupTractorGL(); // Configura os shaders, VAO e VBO para o trator.
    // Inicializa o TerrainManager, passando a configuração do mundo, programas de shader e referências para objetos GL.
    m_terrainManager.init(&m_worldConfig, &m_terrainShaderProgram, this);

    // Define a posição inicial do trator e ajusta sua altura ao terreno.
    m_tractorPosition = QVector3D(0.0f, 0.0f, 0.0f); //definindo uma posição inical
    m_tractorRotation = 0.0f; // sera atualizado pelo gps (rumo)

    m_frameCount = 0; // Zera o contador de quadros para cálculo de FPS.
    m_fpsTime.start(); // Inicia o timer para medição de FPS.
    m_tempReadTimer.start(); // Inicia o timer para leitura de temperatura.
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
    float distancia = m_worldConfig.cameraFollowDistance; // Distância da câmera em relação ao trator.
    float altura = m_worldConfig.cameraFollowHeight; // Altura da câmera em relação ao trator.

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

    //logica para exibir mensagem de perda de sinal RTK
    if (m_isRtkSignalLost) {
        QPainter painter(this);
        painter.setPen(Qt::red);
        painter.setFont(QFont("Arial", 24, QFont::Bold));
        painter.drawText(rect(), Qt::AlignHCenter, "Sinal RTK perdido ou baixa qualidade!");
        painter.end();
        return;
    }

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
        m_terrainShaderProgram.setUniformValue("objectBaseColor", QVector3D(m_worldConfig.terrainColorR, m_worldConfig.terrainColorG, m_worldConfig.terrainColorB));

        // Pede para o TerrainManager renderizar apenas o terreno.
        m_terrainManager.render(&m_terrainShaderProgram, this);

        m_terrainShaderProgram.release(); // Desativa o programa de shader do terreno.
    }

    // Renderiza o Grid (Nova Lógica)
    if (lineShaderOk) {
        // Atualiza a geometria do grid com a posição atual da câmera.
        // O grid se estende pela área de renderização do TerrainManager (gridRenderSize chunks).
        m_terrainGrid.updateGridGeometry(m_camera.position().x(), m_camera.position().z(), m_worldConfig.gridRenderSize);
        // Renderiza o grid usando o shader de linha.
        m_terrainGrid.render(&m_lineShaderProgram, m_camera.viewMatrix(), m_camera.projectionMatrix());
    }

    // Renderiza o trator
    if (m_tractorShaderProgram.isLinked()) {
        m_tractorShaderProgram.bind(); // Ativa o programa de shader do trator.
        QMatrix4x4 tractorModelMatrix; // Matriz de modelo para o trator.

        tractorModelMatrix.translate(m_tractorPosition);
        tractorModelMatrix.rotate(m_tractorRotation, 0.0f, 1.0f, 0.0f);

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
    m_camera.setPerspective(m_worldConfig.cameraFov, static_cast<float>(w) / static_cast<float>(h > 0 ? h : 1), 0.1f, 1000.0f);
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
    double dt = m_gameTickTimer.restart() / 1000.0;

    if (m_immFilter && m_immFilter->isInitialized()) {
        QVector2D predicted_pos = m_immFilter->predictSmoothPosition(dt);

        // Usa o estado combinado mais recente para velocidade e rotação
        QVector2D filtered_vel = m_immFilter->getStateVelocity();

        const float TRACTOR_Y_OFFSET = 0.02f;
        m_tractorPosition.setX(predicted_pos.x());
        m_tractorPosition.setZ(predicted_pos.y());
        m_tractorPosition.setY(NoiseUtils::getHeight(m_tractorPosition.x(), m_tractorPosition.z()) + TRACTOR_Y_OFFSET);

        m_tractorCurrentSpeed = filtered_vel.length();

        if (m_tractorCurrentSpeed > 0.1) {
            m_tractorRotation = -qRadiansToDegrees(qAtan2(filtered_vel.x(), -filtered_vel.y()));
        }

        // A cada quadro, buscamos as probabilidades atuais do MMI e emitimos o sinal.
        const Eigen::VectorXd& probs = m_immFilter->getModeProbabilities();
        QString status = (probs(0) > probs(1)) ? "Reta (FKL)" : "Curva (UKF)";
        emit immStatusUpdated(status, probs(0) * 100.0, probs(1) * 100.0);
    }

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
                MY_LOG_WARNING("CPU_Temp", "Arquivo de temperatura esta vazio");
            } else {
                bool ok; // Variável para verificar se a conversão foi bem-sucedida.
                // Converte a string lida para float e divide por 1000 (o valor do arquivo é em miliCelsius).
                float temperature = line.toFloat(&ok) / 1000.0;
                if (ok) { // Se a conversão foi bem-sucedida.
                    MY_LOG_INFO("CPU_Temp", QString("Leitura da temperatura: %1 °C").arg(temperature, 0, 'f', 1));
                    emit tempUpdated(temperature); // Emite o sinal `tempUpdated` com a temperatura.
                } else {
                    MY_LOG_ERROR("CPU_Temp", QString("Não foi possível converter o conteúdo '%1' para número").arg(line));
                }
            }
            tempFile.close(); // Fecha o arquivo.
        } else {
            // Se não foi possível abrir o arquivo, registra um aviso.
            MY_LOG_ERROR("CPU_Temp", QString("Não foi possível abrir o arquivo de temperatura em: %1").arg(tempFilePath));
            MY_LOG_ERROR("CPU_Temp", "Verifique se o caminho esta correto para a sua placa");
        }
#endif
        m_tempReadTimer.restart(); // Reinicia o timer de leitura de temperatura.
    }

    // Calcula a velocidade em Km/h (velocidade em unidades/segundo * 3.6 para converter para Km/h).
    float speedkm = m_tractorSpeed * 3.6f;
    emit kmUpdated(speedkm); // Emite o sinal `kmUpdated` com a velocidade em Km/h.
    emit coordinatesUpdate(m_tractorPosition.x(), m_tractorPosition.z());

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
        MY_LOG_ERROR("Render", QString("Erro no shader do trator: %1").arg(m_tractorShaderProgram.log()));
        return; // Retorna se houver erro na compilação ou linkagem.
    }

    // Define os vértices do trator como um triângulo simples apontando para o Z negativo.
    GLfloat tractorVertices[] = {0.0f, 0.25f, -0.75, // Vértice superior
                                 -0.5, 0.25f, 0.25f, // Vértice inferior esquerdo
                                 0.5f, 0.25f, 0.25f}; // Vértice inferior direito

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
        MY_LOG_WARNING("Speed", "Recebido valor de velocidade invalido (inf ou nan)");
        m_tractorSpeed = 0.0f;
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

    float steeringNormalized = static_cast<float>(steeringValue - 50) / 50.0f;
    m_steeringAngle = steeringNormalized * MAX_STEERING_ANGLE;
}

void MyGLWidget::onGpsDataUpdate(const GpsData& data) {
    m_currentGpsData = data;

    //portal de qualidade RTK
    if (m_requiredRtkMode == "Com RTK") {
        bool isRtkQualityOk = (data.fixQuality == 4 || data.fixQuality == 5);
        bool isHdopOk = (data.hdop < 2.0 && data.hdop > 0);

        if (!isRtkQualityOk || !isHdopOk) {
            MY_LOG_WARNING("GPS_QualityGate", QString("Dado descartado no modo 'Com RTK'. Qualidade: %1, HDOP: %2")
                                                  .arg(data.fixQuality).arg(data.hdop));
            m_isRtkSignalLost = true;
            update();
            return;
        }
    }

    m_isRtkSignalLost = false;

    if (!m_currentGpsData.isValid) {
        MY_LOG_WARNING("GPS_Processor", "Dado GPS recebido inválido. Posição não atualizada.");
        return;
    }

    // 1. Defina o ponto de referência se ainda não tiver um.
    if (!m_hasReferenceCoordinate) {
        m_referenceCoordinate = QGeoCoordinate(data.latitude, data.longitude);
        m_hasReferenceCoordinate = true;
    }

    // 2. Calcule as coordenadas do mundo a partir do Lat/Lon (sem duplicação).
    QGeoCoordinate currentCoord(data.latitude, data.longitude);
    double distance = m_referenceCoordinate.distanceTo(currentCoord);
    double azimuth = m_referenceCoordinate.azimuthTo(currentCoord);
    double radAzimuth = qDegreesToRadians(azimuth);
    double deltaX_world = distance * qSin(radAzimuth);
    double deltaZ_world = -distance * qCos(radAzimuth);

    // 3. Atualize o perfil adaptativo ANTES de rodar o filtro.
    updateFilterParameters(data);

    // 4. Chame o filtro para processar a medição.
    if (m_immFilter) {
        m_immFilter->updateWithMeasurement(deltaX_world, deltaZ_world);
    }

    // 5. Atualize o estado visual do trator.
    m_lastGpsData = m_currentGpsData;
    checkMovementStatus();
}

//Verifica se o trator esta em linha reta ou fazendo curva
void MyGLWidget::checkMovementStatus() {
    // Requer pelo menos dois pontos de dados GPS para comparar
    if (!m_lastGpsData.isValid || !m_currentGpsData.isValid) {
        m_movimentStatus = "Aguardando dados GPS...";
        emit movementStatusUpdated(m_movimentStatus);
        return;
    }

    const float SPEED_THRESHOLD = 0.5f; // limite de velocidade em m/s
    const float HEADING_CHANGE_THRESHOLD = 2.0f; // Graus

    // 1. Usa a velocidade já filtrada (m_tractorCurrentSpeed), que é mais estável.
    if (m_tractorCurrentSpeed < SPEED_THRESHOLD) {
        // 2. GUARDA o resultado na variável de membro.
        m_movimentStatus = "Parado";
    } else {
        // Lógica de variação de rumo (continua a mesma)
        float headingDelta = m_currentGpsData.courseOverGround - m_lastGpsData.courseOverGround;
        if (headingDelta > 180.0f) headingDelta -= 360.0f;
        if (headingDelta < -180.0f) headingDelta += 360.0f;

        if (qAbs(headingDelta) > HEADING_CHANGE_THRESHOLD) {
            // 2. GUARDA o resultado na variável de membro.
            m_movimentStatus = "Fazendo Curva";
        } else {
            // 2. GUARDA o resultado na variável de membro.
            m_movimentStatus = "Em linha reta";
        }
    }

    // 3. EMITE o status final a partir da variável, uma única vez.
    emit movementStatusUpdated(m_movimentStatus);
}

void MyGLWidget::updateFilterParameters(const GpsData& data) {
    if (!m_immFilter) {
        return;
    }

    FilterProfile dynamicProfile;
    bool profileSet = false;

    // Se o status for "Parado", force o uso do perfil "Parado" e ignore o resto.
    if (m_movimentStatus == "Parado") {
        dynamicProfile = PREDEFINED_PROFILES["Parado"];
        // A incerteza da medição (R) ainda deve ser ajustada pela qualidade do sinal.
        dynamicProfile.R_measurement_uncertainty *= qMax(1.0f, data.hdop);
        profileSet = true;
        MY_LOG_DEBUG("Filter_Params", "Modo PARADO ativado. Forçando perfil de baixo Q.");
    }

    // Se o perfil não foi definido pelo modo "Parado", usa a lógica normal.
    if (!profileSet) {
        // --- 1. Cálculo Dinâmico de R (Incerteza da Medição) ---
        double base_R;
        switch (data.fixQuality) {
        case 4: base_R = 0.05; break;
        case 5: base_R = 0.2; break;
        case 2: base_R = 1.0; break;
        case 1: base_R = 5.0; break;
        default: base_R = 10.0; break;
        }
        dynamicProfile.R_measurement_uncertainty = base_R * qMax(1.0f, data.hdop);

        // --- 2. Cálculo Dinâmico de Q (Incerteza do Processo) ---
        float speedKmh = m_tractorCurrentSpeed * 3.6f;
        if (speedKmh < 1.0f) {
            dynamicProfile.Q_process_uncertainty = 0.0001;
        } else if (speedKmh > 15.0f) {
            dynamicProfile.Q_process_uncertainty = 0.01;
        } else {
            dynamicProfile.Q_process_uncertainty = 0.001;
        }
    }

    // Log e aplicação do perfil (esta parte continua a mesma)
    MY_LOG_DEBUG("Filter_Params", QString("Parâmetros Dinâmicos: R=%1, Q=%2 (Qualidade: %3, HDOP: %4, Status: %5)")
                                    .arg(dynamicProfile.R_measurement_uncertainty, 0, 'f', 4)
                                      .arg(dynamicProfile.Q_process_uncertainty, 0, 'f', 9)
                                      .arg(data.fixQuality)
                                      .arg(data.hdop, 0, 'f', 2)
                                      .arg(m_movimentStatus));

    m_immFilter->setProfile(dynamicProfile);
}

void MyGLWidget::onRtkModeChanged(const QString& newMode) {
    m_requiredRtkMode = newMode;
    MY_LOG_INFO("RTK_Mode", QString("Modo de operação alterado para: %1").arg(newMode));
    m_isRtkSignalLost = false;
    m_immFilter->reset(m_tractorPosition.x(), m_tractorPosition.z());
}

float MyGLWidget::calculateSignalConfidence(const GpsData& data) {
    float confidence = 1.0f; //começa com 100% de confiança

    //penalidade por geometria de satelite ruim (HDOP)
    if (data.hdop > 1.5f) confidence *= 0.9f; // HDOP aceitavel, mas não ideal.
    if (data.hdop > 2.5f) confidence *= 0.7f;
    if (data.hdop > 5.0f) confidence *= 0.4f;

    //penalidade por divercia entre fontes de HDOP (verificação de sanidade)
    if (qAbs(data.hdop - data.gsa_hdop) > 0.5) {
        confidence *= 0.08f;
    }

    //penalidade por sinal fraco (SNR - Relação Sinal-Ruído)
    if (data.usedSatellites.isEmpty()) {
        return 0.1f;
    } else {
        int totalSnr = 0;
        int validSnrCount = 0;
        for (int satId : data.usedSatellites) {
            if (data.satelliteSnr.contains(satId)) {
                totalSnr += data.satelliteSnr[satId];
                validSnrCount++;
            }
        }

        if (validSnrCount > 0) {
            float avgSnr = static_cast<float>(totalSnr) / validSnrCount;
            if (avgSnr < 40) confidence *= 0.9f; //sinal bom, mas nao excelente
            if (avgSnr < 35) confidence *= 0.7f; //Sinal Fraco, possivel multipercurso
        } else {
            confidence *= 0.5f; //nao conseguimos obter o SNR dos satelites usados
        }
    }
    return confidence;
}
