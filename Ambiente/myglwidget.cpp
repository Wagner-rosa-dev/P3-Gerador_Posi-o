#include "myglwidget.h"
#include <QOpenGLContext>
#include <QDebug>
#include <QOpenGLShaderProgram>
#include "noiseutils.h"// Incluído para QOpenGLShaderProgram
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QElapsedTimer>
#include <cmath>

//Constantes com o código GLSL dos shaders
const char* terrainVertexShaderSource = R"(#version 300 es

// Terrain Vertex Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 v_worldPos;
out vec3 v_normal;

void main() {
    vec4 worldPos4 = modelMatrix * vec4(a_position, 1.0);
    gl_Position = projectionMatrix * viewMatrix * worldPos4;
    v_worldPos = worldPos4.xyz;
    v_normal = normalize(mat3(modelMatrix) * a_normal);
}
)";

const char* terrainFragmentShaderSource = R"(#version 300 es

// Terrain Fragment Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

precision mediump float;

in vec3 v_worldPos;
in vec3 v_normal;

out vec4 FragColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 objectBaseColor;

void main() {
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
    vec3 resultColor = (ambient + diffuse) * objectBaseColor;

    float heightFactor = clamp(v_worldPos.y / 20.0, 0.0, 1.0);
    resultColor = mix(resultColor, vec3(0.6, 0.5, 0.3), heightFactor * 0.5);

    FragColor = vec4(resultColor, 1.0);
}
)";

const char* lineVertexShaderSource = R"(#version 300 es

// Line Vertex Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

layout (location = 0) in vec3 a_position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main() {
    vec3 elevated_position = a_position + vec3(0.0, 0.2, 0.0);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(elevated_position, 1.0);
}
)";

const char* lineFragmentShaderSource = R"(#version 300 es

// Line Fragment Shader - TESTE_VERSAO_NOVA_SHADER_04_06_2025

precision mediump float; // Define a precisão padrão para floats

out vec4 FragColor;

uniform vec4 lineColor;

void main() {
    FragColor = lineColor; // Cor cinza para as bordas
}
)";

const char* tractorVertexShaderSource = R"(#version 300 es
layout (location = 0) in vec3 a_position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position, 1.0);
}
)";

const char* tractorFragmentShaderSource = R"(#version 300 es
precision mediump float;

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    m_tractorRotation(0),
    m_extraFunction(nullptr),
    m_tractorSpeed(0.0f),
    m_steeringValue(50)

{
    connect(&m_timer, &QTimer::timeout, this, &MyGLWidget::gameTick);
    m_timer.start(16); // Aproximadamente 60 FPS
    setFocusPolicy(Qt::StrongFocus);

    //nova logica do controlador
    m_speedController = new SpeedController(this);
    //Conecta o sinal da classe controladora ao novo slot
    connect(m_speedController, &SpeedController::speedUpdate, this, &MyGLWidget::onSpeedUpdate);
    //conecta o segundo potenciometro
    connect(m_speedController, &SpeedController::steeringUpdate, this, &MyGLWidget::onSteeringUpdate);
    //Inicia a escuta, a porta pode variar sempre verificar no linus qual a porta da usb
    m_speedController->startListening("/dev/ttyUSB0");
}

MyGLWidget::~MyGLWidget() {
    makeCurrent(); // Garante que o contexto OpenGL está ativo para limpeza
    // Objetos QOpenGL* (shaders, buffers, vao) são limpos por seus destrutores.
    doneCurrent();
}

void MyGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    qInfo() << "MYGLWIDGET_CPP EXECUTANDO - VERSAO SUPER NOVA 04_06_2025_1530";
    m_extraFunction = QOpenGLContext::currentContext()->extraFunctions();
    if (!m_extraFunction) {
        qWarning("QOpenGLExtraFunctions not available. UBOs and other advanced features might not work.");
    }

    glEnable(GL_DEPTH_TEST); // Habilita teste de profundidade
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Cor de fundo azul escura

    qInfo() << "Compilando Terrain Shaders (Versão de Teste 04/06/2025)...";
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, terrainVertexShaderSource)) {
        qWarning() << "Terrain Vertex Shader Compilation Error:" << m_terrainShaderProgram.log();
    }
    if (!m_terrainShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, terrainFragmentShaderSource)) {
        qWarning() << "Terrain Fragment Shader Compilation Error:" << m_terrainShaderProgram.log();
    }
    if (!m_terrainShaderProgram.link()) {
        qWarning() << "Terrain Shader Linker Error:" << m_terrainShaderProgram.log();
    } else {
        qInfo() << "Terrain Shaders linked successfully.";
    }

    qInfo() << "Compilando Line Shaders (Versão de Teste 04/06/2025)...";
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, lineVertexShaderSource)) {
        qWarning() << "Line Vertex Shader Compilation Error:" << m_lineShaderProgram.log();
    }
    if (!m_lineShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, lineFragmentShaderSource)) {
        qWarning() << "Line Fragment Shader Compilation Error:" << m_lineShaderProgram.log();
    }
    if (!m_lineShaderProgram.link()) {
        qWarning() << "Line Shader Linker Error:" << m_lineShaderProgram.log();
    } else {
        qInfo() << "Line Shaders linked successfully.";
    }

    setupLineQuadVAO();
    setupTractorGL();
    m_terrainManager.init(&m_worldConfig, &m_terrainShaderProgram, &m_lineShaderProgram, &m_lineQuadVao, &m_lineQuadVbo, this);

    m_tractorPosition = QVector3D(16.0f, 0.0f, 16.0f);
    m_tractorRotation = 0.0f;
    m_tractorPosition.setY(NoiseUtils::getHeight(m_tractorPosition.x(), m_tractorPosition.z()));

    m_frameCount = 0;
    m_fpsTime.start();
    m_tempReadTimer.start();
}

void MyGLWidget::setupLineQuadVAO() {

    const float thickness = 0.10f; //controle a espessura da linha aqui
    const float y_offset = 0.2f; //pequena elevação em relação ao terreno para evitar z-fighting

    const float s = m_worldConfig.chunkSize;
    const float t = thickness / 2.0f;

    GLfloat lineQuadVertices[] = {
        // duas bordas total de 12 vertices
        // os cantos agora sao calculados para se encontrarem
        //borda inferior
        0.0f, y_offset, -t,      s, y_offset, -t,       s, y_offset, t,
        0.0f, y_offset, -t,      s, y_offset, t,        0.0f, y_offset, t,
        //Lado 2 (X=S)
        s-t, y_offset, 0.0f,     s+t, y_offset, 0.0f,   s+t, y_offset, s,
        s-t, y_offset, 0.0f,     s+t, y_offset, s,      s-t, y_offset, s,

    };

    m_lineQuadVao.create();
    m_lineQuadVao.bind();

    m_lineQuadVbo.create();
    m_lineQuadVbo.bind();
    m_lineQuadVbo.allocate(lineQuadVertices, sizeof(lineQuadVertices));

    if (m_lineShaderProgram.isLinked()) {
        m_lineShaderProgram.enableAttributeArray(0);
        m_lineShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    } else {
        qWarning("Line shader program not linked, cannot set attribute buffers for line quad VAO.");
    }
    m_lineQuadVao.release();
    m_lineQuadVbo.release();
}

void MyGLWidget::paintGL() {
    //logica da camera inteligente (segue o trator)
    float distancia = 12.0f; //dsitancia da camera
    float altura = 3.0f; // altura da camera

    float angleRad = qDegreesToRadians(m_tractorRotation);
    QVector3D tractorForward(sin(angleRad), 0.0f, -cos(angleRad));

    QVector3D cameraPos = m_tractorPosition - (tractorForward * distancia) + QVector3D(0.0f, altura, 0.0f);

    QVector3D cameraTarget = m_tractorPosition + QVector3D(0.0f, 1.0f, 0.0f);

    m_camera.lookAt(cameraPos, cameraTarget, QVector3D(0.0f, 1.0f, 0.0f));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool terrainShaderOk = m_terrainShaderProgram.isLinked();
    bool lineShaderOk = m_lineShaderProgram.isLinked();

    m_terrainManager.update(m_camera.position());

    // Renderiza o terreno
    if (terrainShaderOk) {
        m_terrainShaderProgram.bind();
        m_terrainShaderProgram.setUniformValue("projectionMatrix", m_camera.projectionMatrix());
        m_terrainShaderProgram.setUniformValue("viewMatrix", m_camera.viewMatrix());
        QVector3D sunDirection = QVector3D(-0.5f, -1.0f, -0.5f).normalized();
        m_terrainShaderProgram.setUniformValue("lightDirection", sunDirection);
        m_terrainShaderProgram.setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
        m_terrainShaderProgram.setUniformValue("objectBaseColor", QVector3D(0.4f, 0.6f, 0.2f)); //Verde claro

        //pede para o manager renderizar apenas o terreno
        m_terrainManager.render(&m_terrainShaderProgram, nullptr, this);

        m_terrainShaderProgram.release();
    }

    //renderiza as bordas dos chunks
    if (lineShaderOk) {
        m_lineShaderProgram.bind();
        m_lineShaderProgram.setUniformValue("projectionMatrix", m_camera.projectionMatrix());
        m_lineShaderProgram.setUniformValue("viewMatrix", m_camera.viewMatrix());

        m_lineShaderProgram.setUniformValue("lineColor", QColor(255, 255, 0, 255));

        //pede para o manager renderizar apenas as bordas
        m_terrainManager.render(nullptr, &m_lineShaderProgram, this);

        m_lineShaderProgram.release();
    }


    //Renderiza o trator
    if (m_tractorShaderProgram.isLinked()) {
        m_tractorShaderProgram.bind();
        QMatrix4x4 tractorModelMatrix;

        //Logica de orientação avançada
        //pega a normal do terreno na posição exata do trator
        QVector3D terrainNormal = NoiseUtils::getNormal(m_tractorPosition.x(), m_tractorPosition.z());

        //calcula o vetor para frente do trator baseado na rotação
        float angleRad = qDegreesToRadians(m_tractorRotation);
        QVector3D baseForward(sin(angleRad), 0.0f, -cos(angleRad));

        //Calcula os novos eixos de direção do trator, alinhados ao terreno.
        QVector3D tractorUp = terrainNormal;
        QVector3D tractorRight = QVector3D::crossProduct(baseForward, tractorUp).normalized();
        QVector3D tractorForward = QVector3D::crossProduct(tractorUp, tractorRight).normalized();

        //Cria a matriz de transformação completa
        QMatrix4x4 rotationMatrix;
        rotationMatrix.setColumn(0, tractorRight); //Eixo X do trator
        rotationMatrix.setColumn(1, tractorUp); //Eixo Y do trator
        rotationMatrix.setColumn(2, tractorForward); //Eixo Z do trator

        QMatrix4x4 translationMatrix;
        translationMatrix.translate(m_tractorPosition);

        tractorModelMatrix = translationMatrix * rotationMatrix;
        // fim da logica
        m_tractorShaderProgram.setUniformValue("projectionMatrix", m_camera.projectionMatrix());
        m_tractorShaderProgram.setUniformValue("viewMatrix", m_camera.viewMatrix());
        m_tractorShaderProgram.setUniformValue("modelMatrix", tractorModelMatrix);
        m_tractorVao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        m_tractorVao.release();
    }
    //Logica de calculo de fps
    m_frameCount++;
    if (m_fpsTime.elapsed() >= 1000) {
        float fps = m_frameCount / (m_fpsTime.elapsed() / 1000.0f);

        emit fpsUpdated(qRound(fps));

        m_frameCount = 0;
        m_fpsTime.restart();
    }
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        qWarning() << "Erro no OpenGl em tempo de execução" << err;
    }
}

void MyGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    m_camera.setPerspective(35.0f, static_cast<float>(w) / static_cast<float>(h > 0 ? h : 1), 0.1f, 1000.0f);
}

void MyGLWidget::gameTick() {
    if (m_tempReadTimer.elapsed() >= 2000) { // a cada 2 segundos
        //qInfo() << "Timer de 2s atingido. tentando ler a temperatura..";
#ifdef Q_OS_LINUX
        QString tempFilePath = "/sys/class/thermal/thermal_zone0/temp";
        QFile tempFile(tempFilePath);

        if (tempFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&tempFile);
            QString line = in.readLine();

            //Verificar se a linha nao esta vazia
            if (line.trimmed().isEmpty()) {
                qWarning() << "Arquivo de temperatura esta vazio";
            } else {
                bool ok;
                float temperature = line.toFloat(&ok) / 1000.0;
                if (ok) {
                    //qInfo() << "Sucesso: leitura da temperatura concluida";
                    emit tempUpdated(temperature); //Emite o sinal com a temperatura
                } else {
                    qWarning() << "Erro: nao foi possivel converter o conteudo'" << line << "'para numero";
                }
            }
            tempFile.close();
        } else {
            //se caso o if falou e n foi possivel abrir o arquivo
            qWarning() << "erro: nao foi possivel abrir o arquivo de temperatura em:" << tempFilePath;
            qWarning() << "Verifique se o caminho esta correto para a sua placa";
        }
#endif
        m_tempReadTimer.restart();
    }

    //logica de direção (volante)
    const int deadZone = 2; //Zona morta no centro (48-52 nao faz nada)
    const float maxTurnRate = 2.0f; //Graus por frame na virada maxima

    if (m_steeringValue < 50 - deadZone) { // virando para a esquerda
        //normaliza o valor de 0 a 1(0 perto do centro, 1 na virada maxima)
        float turnFactor = (float)(50 - m_steeringValue) / (50.0f);
        m_tractorRotation -= maxTurnRate * turnFactor;
    } else if (m_steeringValue > 50 + deadZone) { // virando para a direita
        float turnFactor = (float)(m_steeringValue - 50) / (50.0f);
        m_tractorRotation += maxTurnRate * turnFactor;
    }



    //nova logica de movimento automatico
    if(m_tractorSpeed > 0.05f) {
        //Usamos um pequeno "dead zone" para evitar movimento por ruido
        //Calcula a distancia a ser movida neste frame
        //Velocidade(unidade/segundo) * tempo do frame (em segundos)
        //nosso timer e de 16ms ou seja 0.016s
        float distranceThisFrame = m_tractorSpeed * (16.0f / 1000.0f);

        //Pega o vetor para frente do trator
        float angleRad = qDegreesToRadians(m_tractorRotation);
        QVector3D tractorForward(sin(angleRad), 0.0f, -cos(angleRad));

        //Atualiza a posição do trator
        m_tractorPosition += tractorForward.normalized() * distranceThisFrame;
        //Atualiza a altura do trator para seguir o terreno
        m_tractorPosition.setY(NoiseUtils::getHeight(m_tractorPosition.x(), m_tractorPosition.z()));
    }

    float speedkm = m_tractorSpeed * 3.6f;
    emit kmUpdated(speedkm);
    emit coordinatesUpdate(m_tractorPosition.x(), m_tractorPosition.z());
    update(); // Reagenda paintGL()
}

void MyGLWidget::setupTractorGL() {
    if (!m_tractorShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, tractorVertexShaderSource) ||
        !m_tractorShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, tractorFragmentShaderSource) ||
        !m_tractorShaderProgram.link()) {
        qWarning() << "Erro no shader do trator:" << m_tractorShaderProgram.log();
        return;
    }

    GLfloat tractorVertices[] = {0.0f, 0.5f, -0.75, -0.5, 0.0f, 0.25f, 0.5f, 0.0f, 0.25f}; // Triangulo apontando para o Z negativo

    m_tractorVao.create();
    m_tractorVao.bind();
    m_tractorVbo.create();
    m_tractorVbo.bind();
    m_tractorVbo.allocate(tractorVertices, sizeof(tractorVertices));
    m_tractorShaderProgram.enableAttributeArray(0);
    m_tractorShaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
    m_tractorVao.release();
    m_tractorVbo.release();
}

void MyGLWidget::keyPressEvent(QKeyEvent *event) {
    QOpenGLWidget::keyPressEvent(event);
}

void MyGLWidget::onSpeedUpdate(float newSpeed)
{
    //simplesmente atualiza a variavel de velocidade da classe
    if (std::isfinite(newSpeed)) {
        m_tractorSpeed = newSpeed;
    } else {
        //se receber lixo, zeramos a velocidade para segurança
        m_tractorSpeed = 0.0f;
        qWarning() << "Recebido valor de velocidade invalido (inf ou nan)";
    }
}

void MyGLWidget::onSteeringUpdate(int steeringValue) {
    m_steeringValue = steeringValue;
}
