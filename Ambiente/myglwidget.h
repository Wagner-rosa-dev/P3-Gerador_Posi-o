#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QOpenGLWidget>        // Classe base para um widget que renderiza gráficos OpenGL.
#include <QOpenGLFunctions>     // Para acesso às funções OpenGL ES 2.0.
#include <QOpenGLExtraFunctions> // Para acesso a funções OpenGL ES 3.0 e extensões.
#include <QOpenGLShaderProgram> // Para gerenciar programas de shader.
#include <QOpenGLBuffer>        // Para gerenciar buffers de vértices/índices.
#include <QOpenGLVertexArrayObject> // Para gerenciar Vertex Array Objects (VAOs).
#include <QTimer>               // Para agendar a atualização da lógica do jogo (game tick).
#include "camera.h"             // Inclui a definição da classe Camera.
#include "terrainmanager.h"     // Inclui a definição da classe TerrainManager.
#include <QElapsedTimer>        // Para medir o tempo (e.g., cálculo de FPS).
#include "speedcontroller.h"    // Inclui a definição da classe SpeedController.
#include "worldconfig.h"        // Inclui a estrutura WorldConfig.
#include <QGeoCoordinate>
#include "immfilter.h"
#include "gpsfileplayer.h"
#include "terraingrid.h"


// Estrutura: SceneMatrices
// Descrição: Uma estrutura para agrupar as matrizes de projeção e visão da cena.
//            Pode ser útil para passar essas matrizes juntas para funções ou shaders.
struct SceneMatrices {

    // Membro: projectionMatrix
    // Tipo: QMatrix4x4
    // Descrição: A matriz de projeção da câmera, responsável por transformar coordenadas 3D em 2D.
    QMatrix4x4 projectionMatrix;

    // Membro: viewMatrix
    // Tipo: QMatrix4x4
    // Descrição: A matriz de visão da câmera, responsável por posicionar e orientar a câmera no mundo.
    QMatrix4x4 viewMatrix;
};

// Classe: MyGLWidget
// Descrição: Este é o widget principal onde toda a renderização OpenGL ocorre.
//            Ele herda de QOpenGLWidget e QOpenGLFunctions, fornecendo um contexto OpenGL
//            e acesso facilitado às funções GL.
//            Gerencia a câmera, o terreno, a renderização de objetos (como o trator),
//            e interage com o SpeedController para atualizar a simulação com base em
//            dados externos. Também é responsável por emitir sinais de atualização da UI
//            (FPS, temperatura, velocidade, coordenadas).
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT // Macro necessária para classes com sinais e Slots do Qt.

public:
    // Construtor: MyGLWidget
    // Descrição: Inicializa o widget OpenGL, configura o timer para o loop do jogo,
    //            configura a política de foco para eventos de teclado e inicializa o
    //            SpeedController para comunicação serial.
    // Parâmetros:
    //   - parent: O objeto pai (QWidget) deste widget, para gerenciamento de memória.
    explicit MyGLWidget(const WorldConfig& config, QWidget *parent = nullptr);

    // Destrutor: ~MyGLWidget
    // Descrição: Garante que o contexto OpenGL seja liberado adequadamente antes da destruição
    //            dos objetos relacionados ao GL.
    ~MyGLWidget();

public slots:
    void onRtkModeChanged(const QString& newMode);



signals:
    // Sinais (Anúncios que a classe faz):
    // Descrição: Estes sinais são emitidos pelo MyGLWidget para notificar outras partes
    //            da aplicação (como a MainWindow) sobre atualizações de estado.

    // Sinal: fpsUpdated
    // Descrição: Emitido quando a taxa de quadros por segundo é atualizada.
    // Parâmetros:
    //   - fps: O valor inteiro do FPS.
    void fpsUpdated(int fps);

    // Sinal: tempUpdated
    // Descrição: Emitido quando a temperatura da CPU é lida e atualizada.
    // Parâmetros:
    //   - temp: O valor float da temperatura.
    void tempUpdated(float temp);

    // Sinal: kmUpdated
    // Descrição: Emitido quando a velocidade do trator (em Km/h) é atualizada.
    // Parâmetros:
    //   - km: O valor float da velocidade.
    void kmUpdated(float km);

    // Sinal: coordinatesUpdate
    // Descrição: Emitido quando as coordenadas de longitude e latitude do trator são atualizadas.
    // Parâmetros:
    //   - lon: O valor float da longitude.
    //   - lat: O valor float da latitude.
    void coordinatesUpdate(float lon, float lat);

    //novo sinal para status da linha reta/curva
    void movementStatusUpdated(const QString& status);

    void immStatusUpdated(const QString& status, double probReta, double probCurva);

protected:
    // Método: initializeGL
    // Descrição: Chamado uma vez para configurar o contexto OpenGL e carregar recursos
    //            (shaders, VAOs, VBOs, texturas, etc.).
    void initializeGL() override;

    // Método: paintGL
    // Descrição: Chamado repetidamente para desenhar a cena OpenGL.
    //            Contém a lógica de renderização principal (terreno, trator, etc.).
    void paintGL() override;

    // Método: resizeGL
    // Descrição: Chamado quando o widget é redimensionado.
    //            Responsável por ajustar a viewport e a matriz de projeção da câmera.
    // Parâmetros:
    //   - w: Nova largura do widget.
    //   - h: Nova altura do widget.
    void resizeGL(int w, int h) override;



private slots:
    // Slot Privado: gameTick
    // Descrição: O slot principal do loop do jogo, acionado pelo timer.
    //            Contém a lógica de atualização da simulação (movimento do trator,
    //            cálculo de FPS, leitura de temperatura, atualização do terreno).
    void gameTick();

    // Slot Privado: onSpeedUpdate
    // Descrição: Slot que recebe atualizações de velocidade do SpeedController.
    //            Atualiza a velocidade interna do trator.
    // Parâmetros:
    //   - newSpeed: A nova velocidade recebida.
    void onSpeedUpdate(float newSpeed);

    // Slot Privado: onSteeringUpdate
    // Descrição: Slot que recebe atualizações de valor de direção (esterçamento) do SpeedController.
    //            Atualiza o valor de esterçamento interno do trator.
    // Parâmetros:
    //   - steeringValue: O novo valor de esterçamento recebido.
    void onSteeringUpdate(int steeringValue);

    //novo slot para receber os dados GPS
    void onGpsDataUpdate(const GpsData& data);

private:
    // Método Privado: setupTractorGL
    // Descrição: Configura os shaders, VAO e VBO para renderizar o modelo do trator.
    void setupTractorGL();

    void checkMovementStatus();

    float calculateSignalConfidence(const GpsData& data);

    void updateFilterParameters(const GpsData& data);

    // Membro: m_tractorRotation
    // Tipo: float
    // Descrição: A rotação atual do trator em graus (em torno do eixo Y).
    float m_tractorRotation;

    // Membro: m_frameCount
    // Tipo: int
    // Descrição: Contador de quadros para o cálculo de FPS.
    int m_frameCount;

    // Membro: m_timer
    // Tipo: QTimer
    // Descrição: Timer para controlar o loop do jogo (gameTick).
    QTimer m_timer;

    // Membro: m_camera
    // Tipo: camera
    // Descrição: Objeto da câmera que define a visão do mundo.
    camera m_camera;

    // Membro: m_worldConfig
    // Tipo: WorldConfig
    // Descrição: Configurações do mundo (tamanho de chunk, LOD, etc.).
    WorldConfig m_worldConfig;

    // Membro: m_terrainManager
    // Tipo: terrainmanager
    // Descrição: Objeto responsável por gerenciar o terreno 3D.
    terrainmanager m_terrainManager;

    // Membro: m_terrainGrid
    // Tipo: TerrainGrid
    // Descrição: Novo objeto responsável por gerenciar e renderizar a grade do terreno.
    TerrainGrid m_terrainGrid; // Adicionado: Nova instância de TerrainGrid

    // Membro: m_fpsTime
    // Tipo: QElapsedTimer
    // Descrição: Timer para medir o tempo decorrido para calcular o FPS.
    QElapsedTimer m_fpsTime;

    // Membro: m_tempReadTimer
    // Tipo: QElapsedTimer
    // Descrição: Timer para controlar a frequência de leitura da temperatura da CPU.
    QElapsedTimer m_tempReadTimer;

    // Membro: m_tractorPosition
    // Tipo: QVector3D
    // Descrição: A posição atual do trator no espaço do mundo.
    QVector3D m_tractorPosition;

    // Membro: m_extraFunction
    // Tipo: QOpenGLExtraFunctions*
    // Descrição: Ponteiro para funções OpenGL ES 3.0 e extensões.
    QOpenGLExtraFunctions *m_extraFunction;


    // Membro: m_tractorVao
    // Tipo: QOpenGLVertexArrayObject
    // Descrição: VAO para o modelo 3D do trator.
    QOpenGLVertexArrayObject m_tractorVao;


    // Membro: m_tractorVbo
    // Tipo: QOpenGLBuffer
    // Descrição: VBO para o modelo 3D do trator.
    QOpenGLBuffer m_tractorVbo;

    float m_tractorCurrentSpeed;
    float m_tractorTargetSpeed;
    float m_steeringAngle;

    const float WHEELBASE = 3.0f;
    const float MAX_STEERING_ANGLE = 0.5;

    QElapsedTimer m_gameTickTimer;

    // Membro: m_terrainShaderProgram
    // Tipo: QOpenGLShaderProgram
    // Descrição: Programa de shader para renderizar o terreno.
    QOpenGLShaderProgram m_terrainShaderProgram;

    // Membro: m_lineShaderProgram
    // Tipo: QOpenGLShaderProgram
    // Descrição: Programa de shader para renderizar as linhas (bordas dos chunks).
    QOpenGLShaderProgram m_lineShaderProgram;

    // Membro: m_tractorShaderProgram
    // Tipo: QOpenGLShaderProgram
    // Descrição: Programa de shader para renderizar o trator.
    QOpenGLShaderProgram m_tractorShaderProgram;

    // Membro: m_speedController
    // Tipo: SpeedController*
    // Descrição: Ponteiro para o controlador de velocidade que se comunica com a porta serial.
    SpeedController *m_speedController;

    GpsFilePlayer *m_gpsFilePlayer;

    // Membro: m_tractorSpeed
    // Tipo: float
    // Descrição: A velocidade atual do trator, controlada externamente.
    float m_tractorSpeed;

    // Membro: m_steeringValue
    // Tipo: int
    // Descrição: O valor de esterçamento (direção) do trator, controlado externamente.
    //            Geralmente de um potenciômetro (0-100 ou similar).
    int m_steeringValue;


    //Variaveis para o GPS
    GpsData m_currentGpsData;
    GpsData m_lastGpsData;
    //precisaremos de um ponto de referencia para transformar lat\lon em X\Z do mundo
    QGeoCoordinate m_referenceCoordinate;
    bool m_hasReferenceCoordinate;
    float m_currentHeading; // Rumo atual do trator (do GPS)

    immfilter *m_immFilter;

    QString m_requiredRtkMode;
    bool m_isRtkSignalLost;

    QString m_movimentStatus;

};

#endif // MYGLWIDGET_H
