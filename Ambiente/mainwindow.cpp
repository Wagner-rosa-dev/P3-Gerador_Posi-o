#include "mainwindow.h" // Inclui o cabeçalho da classe MainWindow.
#include "myglwidget.h" // Inclui o cabeçalho da classe MyGLWidget, o widget OpenGL.
#include <QLabel>       // Inclui QLabel para exibir texto na UI.
#include <QGridLayout>  // Inclui QGridLayout para organizar os widgets em uma grade.
#include <QFont>        // Inclui QFont para estilizar o texto dos labels.
#include <QVBoxLayout>  // Inclui QVBoxLayout para organizar widgets verticalmente.
#include "gpsfileplayer.h"
#include "kalmanfilter.h"
#include <QMessageBox>
#include "logger.h"

/**
 * @brief Construtor da classe MainWindow.
 * @param parent O QWidget pai desta janela.
 *
 * Configura a interface do usuário da janela principal, criando o widget OpenGL,
 * labels para exibir informações (FPS, temperatura, velocidade, coordenadas),
 * estilizando esses labels e organizando-os em um layout de grade.
 * Finalmente, conecta os sinais do MyGLWidget aos slots desta janela para atualização da UI.
 */
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent) // Chama o construtor da classe base QWidget.

{
    // 1. Cria os widgets que farão parte da janela:
    // Cria uma instância de MyGLWidget, que é a área de renderização OpenGL.
    m_glWidget = new MyGLWidget(this);
    // Cria QLabels para exibir informações.
    m_fpsLabel = new QLabel("FPS: --", this);
    m_tempLabel = new QLabel("CPU: -- °C", this);
    m_kmLabel = new QLabel("Velocidade: 0.0 km/h", this);
    m_lonlabel = new QLabel("Lon: 0.0",this);
    m_latLabel = new QLabel("Lat: 0.0", this);
    m_movementStatusLabel = new QLabel("Status: Parado", this); // novo label
    m_gpsFilePlayer = new GpsFilePlayer(this);
    m_kalmanFilter = new KalmanFilter(0.0, 0.0);
    m_lastGpsTimestamp = QDateTime();

    m_profileLabel = new QLabel("Perfil do filtro:", this);
    m_profileSelector = new QComboBox(this);
    m_profileSelector->addItem("Padrão");
    m_profileSelector->addItem("Veículo Lento");
    m_profileSelector->addItem("Veículo Àgil");




    //* 2. Estiliza os labels para que fiquem bem visíveis:
    QFont labelFont("Arial", 12, QFont::Bold); // Define a fonte para os labels.
    m_fpsLabel->setFont(labelFont); // Aplica a fonte ao label de FPS.
    m_fpsLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px"); // Estilo CSS para o label de FPS.
    m_tempLabel->setFont(labelFont); // Aplica a fonte ao label de temperatura.
    m_tempLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px"); // Estilo CSS para o label de temperatura.
    m_kmLabel->setFont(labelFont); // Aplica a fonte ao label de velocidade.
    m_kmLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px"); // Estilo CSS para o label de velocidade.
    m_latLabel->setFont(labelFont); // Aplica a fonte ao label de latitude.
    m_latLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px"); // Estilo CSS para o label de latitude.
    m_lonlabel->setFont(labelFont); // Aplica a fonte ao label de longitude.
    m_lonlabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px"); // Estilo CSS para o label de longitude.
    m_movementStatusLabel->setFont(labelFont); // Aplica a fonte ao label no status de movimento
    m_movementStatusLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px"); // Estilo CSS para o label de longitude.
    m_profileLabel->setFont(labelFont);
    m_profileLabel->setStyleSheet("color: white");
    m_profileSelector->setStyleSheet("color: black; background-color: white; padding: 2px");




    // Cria um layout de grade para sobrepor os widgets.
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0,0,0,0); // Remove margens do layout.

    // Adiciona a tela OpenGL ocupando todo o espaço da grade (linha 0, coluna 0).
    mainLayout->addWidget(m_glWidget, 0, 0);

    // Adiciona os labels na mesma célula da grade, mas com alinhamentos diferentes:
    mainLayout->addWidget(m_fpsLabel, 0, 0, Qt::AlignTop | Qt::AlignLeft);      // FPS no canto superior esquerdo.
    mainLayout->addWidget(m_tempLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);    // Temperatura no canto superior direito.
    mainLayout->addWidget(m_kmLabel, 0, 0, Qt::AlignBottom | Qt::AlignLeft);    // Velocidade no canto inferior esquerdo.
    mainLayout->addWidget(m_profileLabel, 6, 0);
    mainLayout->addWidget(m_profileSelector, 7, 0);

    // Cria um layout vertical para as coordenadas. e o status de movimento
    QVBoxLayout *coordStatusLayout = new QVBoxLayout;
    // Adiciona os labels de latitude e longitude a este layout vertical.
    coordStatusLayout->addWidget(m_latLabel);
    coordStatusLayout->addWidget(m_lonlabel);
    coordStatusLayout->addWidget(m_movementStatusLabel);
    // Adiciona o layout vertical inteiro (com os labels de coord) ao canto inferior direito.
    mainLayout->addLayout(coordStatusLayout, 0, 0, Qt::AlignBottom | Qt::AlignRight);

    // Define o layout principal para a janela.
    setLayout(mainLayout);

    // Sinais e Slots:
    // Conecta o sinal `fpsUpdated` do `m_glWidget` ao slot `updateFpsLabel` desta janela.
    connect(m_glWidget, &MyGLWidget::fpsUpdated, this, &MainWindow::updateFpsLabel);
    // Conecta o sinal `tempUpdated` do `m_glWidget` ao slot `updateTempLabel` desta janela.
    connect(m_glWidget, &MyGLWidget::tempUpdated, this, &MainWindow::updateTempLabel);
    // Conecta o sinal `kmUpdated` do `m_glWidget` ao slot `updateKmLabel` desta janela.
    connect(m_glWidget, &MyGLWidget::kmUpdated, this, &MainWindow::updateKmLabel);
    // Conecta o sinal `coordinatesUpdate` do `m_glWidget` ao slot `updateCoordinatesLabel` desta janela.
    connect(m_glWidget, &MyGLWidget::coordinatesUpdate, this, &MainWindow::updateCoordinatesLabel);
    //conecta o novo sinal do myglwidget ao slot desta janela
    connect(m_glWidget, &MyGLWidget::movementStatusUpdated, this, &MainWindow::updateMovementStatusLabel);
    //conecta o novo sinal do kalman filter depois de receber os dados do GPS
    connect(m_gpsFilePlayer, &GpsFilePlayer::gpsDataUpdate, this, &MainWindow::handleGpsDataUpdate);
    //Quando o usuario mudar a seleção, o sot onProfileChanged sera chamado
    connect(m_profileSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);

    resize(800, 600); // Define o tamanho inicial da janela.

    onProfileChanged(0);
}

/**
 * @brief Destrutor da classe MainWindow.
 *
 * No Qt, a destruição dos objetos filhos geralmente é gerenciada automaticamente
 * quando o objeto pai é destruído, então o destrutor pode ser vazio.
 */
MainWindow::~MainWindow()
{}

/**
 * @brief Slot para atualizar o texto do label de FPS.
 * @param fps O valor inteiro de FPS.
 *
 * Formata o valor de FPS em uma string e define essa string como o texto do `m_fpsLabel`.
 */
void MainWindow::updateFpsLabel(int fps)
{
    m_fpsLabel->setText(QString("FPS: %1").arg(fps));
}

/**
 * @brief Slot para atualizar o texto do label de temperatura da CPU.
 * @param temp O valor float da temperatura em graus Celsius.
 *
 * Formata o valor da temperatura com uma casa decimal e define essa string
 * como o texto do `m_tempLabel`.
 */
void MainWindow::updateTempLabel(float temp)
{
    m_tempLabel->setText(QString("CPU: %1 °C").arg(temp, 0, 'f', 1));
}

/**
 * @brief Slot para atualizar o texto do label de velocidade em Km/h.
 * @param km O valor float da velocidade em Km/h.
 *
 * Formata o valor da velocidade com uma casa decimal e define essa string
 * como o texto do `m_kmLabel`.
 */
void MainWindow::updateKmLabel(float km)
{
    m_kmLabel->setText(QString("Velocidade: %1 Km/h").arg(km, 0, 'f', 1));
}

/**
 * @brief Slot para atualizar os textos dos labels de coordenadas (longitude e latitude).
 * @param lon O valor float da longitude.
 * @param lat O valor float da latitude.
 *
 * Formata os valores de longitude e latitude com uma casa decimal e define
 * essas strings como os textos dos `m_lonlabel` e `m_latLabel`, respectivamente.
 */
void MainWindow::updateCoordinatesLabel(float lon, float lat)
{
    m_lonlabel->setText(QString("Lon: %1").arg(lon, 0, 'f', 7));
    m_latLabel->setText(QString("Lat: %1").arg(lat, 0, 'f', 7));
}

void MainWindow::updateMovementStatusLabel(const QString& status)
{
    m_movementStatusLabel->setText(QString("Status: %1").arg(status));
}

void MainWindow::handleGpsDataUpdate(const GpsData& data) {
    if (!data.isValid) {
        return;
    }

    double dt_seconds = 0.0;
    if (m_lastGpsTimestamp.isValid()) {
        qint64 msDiff = m_lastGpsTimestamp.msecsTo(data.timestamp);
        dt_seconds = static_cast<double>(msDiff) / 1000.0;
    } else {
        dt_seconds = 0.016;
    }
    m_lastGpsTimestamp = data.timestamp;

    double measuredX = data.longitude;
    double measuredZ = data.latitude;

    m_kalmanFilter->predict(dt_seconds);
    m_kalmanFilter->update(measuredX, measuredZ);

    QVector2D estimatedPosition = m_kalmanFilter->getStatePosition();
    QVector2D estimatedVelocity = m_kalmanFilter->getStateVelocity();

    MY_LOG_DEBUG("MainWindow", QString("GPS Estimado: Px=%1 Pz=%2 Vx=%3 Vz=%4")
                                    .arg(estimatedPosition.x(), 0, 'f', 3)
                                    .arg(estimatedPosition.y(), 0, 'f', 3)
                                    .arg(estimatedVelocity.x(), 0, 'f', 3)
                                    .arg(estimatedVelocity.y(), 0, 'f', 3));

}

void MainWindow::onProfileChanged(int index)
{
    QString selectedProfileText = m_profileSelector->itemText(index);
    MY_LOG_INFO("UI", QString("Perfil do filtro alterado para: %1").arg(selectedProfileText));

    // Verifica se o perfil selecionado existe no nosso mapa de perfis
    if (PREDEFINED_PROFILES.contains(selectedProfileText)) {
        // Pega o perfil correspondente
        FilterProfile selectedProfile = PREDEFINED_PROFILES[selectedProfileText];

        // Pega o ponteiro para o filtro de Kalman através do MyGLWidget e define o novo perfil
        m_glWidget->getKalmanFilter()->setProfile(selectedProfile);
    }
}
