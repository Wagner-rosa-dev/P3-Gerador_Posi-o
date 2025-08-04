#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget> // Classe base para a janela principal da aplicação.
#include "kalmanfilter.h"
#include "gpsfileplayer.h"
#include <QComboBox>

// Declarações antecipadas para evitar inclusões circulares
// Descrição: Permitem que MainWindow declare ponteiros para QLabel e MyGLWidget
//            sem precisar incluir seus arquivos de cabeçalho completos aqui,
//            melhorando os tempos de compilação.
class QLabel;
class MyGLWidget;

// Classe: MainWindow
// Descrição: Esta classe representa a janela principal da aplicação.
//            Ela herda de QWidget e é responsável por configurar o layout da UI,
//            incorporar o widget OpenGL (MyGLWidget) e exibir informações
//            como FPS, temperatura da CPU, velocidade e coordenadas em QLabel's.
//            Também gerencia a atualização dessas informações através de sinais e slots.
class MainWindow : public QWidget
{
    Q_OBJECT  // Macro necessária para classes com sinais e Slots do Qt.

public:
    // Construtor: MainWindow
    // Descrição: Inicializa a janela principal, cria e configura os widgets (GLWidget e Labels),
    //            organiza-os em um layout e conecta os sinais dos dados do MyGLWidget
    //            aos slots de atualização dos Labels.
    // Parâmetros:
    //   - parent: O objeto pai (QWidget) desta janela, para gerenciamento de memória.
    MainWindow(QWidget *parent = nullptr);

    // Destrutor: ~MainWindow
    // Descrição: Limpeza de recursos, embora a maioria dos widgets filhos sejam
    //            automaticamente destruídos pelo sistema de parentesco do Qt.
    ~MainWindow();





private slots:

    // Slot: updateFpsLabel
    // Descrição: Atualiza o texto do QLabel que exibe a taxa de quadros por segundo (FPS).
    // Parâmetros:
    //   - fps: O valor inteiro de FPS a ser exibido.
    void updateFpsLabel(int fps);

    // Slot: updateTempLabel
    // Descrição: Atualiza o texto do QLabel que exibe a temperatura da CPU.
    // Parâmetros:
    //   - temp: O valor float da temperatura em graus Celsius.
    void updateTempLabel(float temp);

    // Slot: updateKmLabel
    // Descrição: Atualiza o texto do QLabel que exibe a velocidade do trator em Km/h.
    // Parâmetros:
    //   - km: O valor float da velocidade em Km/h.
    void updateKmLabel(float km);

    // Slot: updateCoordinatesLabel
    // Descrição: Atualiza o texto dos QLabels que exibem as coordenadas de longitude e latitude.
    // Parâmetros:
    //   - lon: O valor float da longitude.
    //   - lat: O valor float da latitude.
    void updateCoordinatesLabel(float lon, float lat);

    //novo slot para o status de movimento
    void updateMovementStatusLabel(const QString& status);

    void handleGpsDataUpdate(const GpsData& data);

    void onProfileChanged(int index);

private:
    // Membro: m_glWidget
    // Tipo: MyGLWidget*
    // Descrição: Ponteiro para o widget OpenGL onde a cena 3D é renderizada.
    MyGLWidget *m_glWidget;

    // Membro: m_fpsLabel
    // Tipo: QLabel*
    // Descrição: QLabel para exibir a taxa de quadros por segundo (FPS).
    QLabel *m_fpsLabel;

    // Membro: m_tempLabel
    // Tipo: QLabel*
    // Descrição: QLabel para exibir a temperatura da CPU.
    QLabel *m_tempLabel;

    // Membro: m_kmLabel
    // Tipo: QLabel*
    // Descrição: QLabel para exibir a velocidade do trator em Km/h.
    QLabel *m_kmLabel;

    // Membro: m_latLabel
    // Tipo: QLabel*
    // Descrição: QLabel para exibir a coordenada de latitude do trator.
    QLabel *m_latLabel;

    // Membro: m_lonlabel
    // Tipo: QLabel*
    // Descrição: QLabel para exibir a coordenada de longitude do trator.
    QLabel *m_lonlabel;

    //novo label
    QLabel *m_movementStatusLabel;

    KalmanFilter* m_kalmanFilter;
    QDateTime m_lastGpsTimestamp;
    GpsFilePlayer *m_gpsFilePlayer;

    QLabel *m_profileLabel;
    QComboBox *m_profileSelector;




};

#endif // MAINWINDOW_H
