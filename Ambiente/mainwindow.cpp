#include "mainwindow.h" // Inclui o cabeçalho da classe MainWindow.
#include "myglwidget.h" // Inclui o cabeçalho da classe MyGLWidget, o widget OpenGL.
#include <QLabel>       // Inclui QLabel para exibir texto na UI.
#include <QGridLayout>  // Inclui QGridLayout para organizar os widgets em uma grade.
#include <QFont>        // Inclui QFont para estilizar o texto dos labels.
#include <QVBoxLayout>  // Inclui QVBoxLayout para organizar widgets verticalmente.
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
    m_lastGpsTimestamp = QDateTime();
    m_immStatusLabel = new QLabel("Filtro: --", this);
    m_rtkModeComboBox = new QComboBox(this);






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
    m_immStatusLabel->setFont(labelFont);
    m_immStatusLabel->setStyleSheet("color: yellow; background-color: rgba(0,0,0,100); padding: 2px");
    m_rtkModeComboBox->setFont(labelFont);
    m_rtkModeComboBox->addItem("Sem RTK");
    m_rtkModeComboBox->addItem("Com RTK");
    m_rtkModeComboBox->setStyleSheet("QComboBox { color: white; background-color: #333; selection-background-color: #555; }");




    // Cria um layout de grade para sobrepor os widgets.
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0,0,0,0); // Remove margens do layout.

    // Adiciona a tela OpenGL ocupando todo o espaço da grade (linha 0, coluna 0).
    mainLayout->addWidget(m_glWidget, 0, 0);

    // Adiciona os labels na mesma célula da grade, mas com alinhamentos diferentes:
    mainLayout->addWidget(m_fpsLabel, 0, 0, Qt::AlignTop | Qt::AlignLeft);      // FPS no canto superior esquerdo.
    mainLayout->addWidget(m_tempLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);    // Temperatura no canto superior direito.
    mainLayout->addWidget(m_kmLabel, 0, 0, Qt::AlignBottom | Qt::AlignLeft);    // Velocidade no canto inferior esquerdo.

    // Cria um layout vertical para as coordenadas. e o status de movimento
    QVBoxLayout *coordStatusLayout = new QVBoxLayout;
    // Adiciona os labels de latitude e longitude a este layout vertical.
    coordStatusLayout->addWidget(m_latLabel);
    coordStatusLayout->addWidget(m_lonlabel);
    coordStatusLayout->addWidget(m_movementStatusLabel);
    coordStatusLayout->addWidget(m_immStatusLabel);
    // Adiciona o layout vertical inteiro (com os labels de coord) ao canto inferior direito.
    mainLayout->addWidget(m_rtkModeComboBox, 0, 0, Qt::AlignTop | Qt::AlignHCenter);
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
    //sinal do filtro
    connect(m_glWidget, &MyGLWidget::immStatusUpdated, this, &MainWindow::updateImmStatus);
    //Sinal da mudança na combobox do RTK
    connect(m_rtkModeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::rtkModeChanged);
    connect(this, &MainWindow::rtkModeChanged, m_glWidget, &MyGLWidget::onRtkModeChanged);

    resize(800, 600); // Define o tamanho inicial da janela.

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

void MainWindow::updateImmStatus(const QString& status, double probReta, double probCurva) {
    m_immStatusLabel->setText(QString("Filtro: %1 (R: %2 C: %3%)")
                              .arg(status)
                              .arg(probReta, 0, 'f', 0)
                                  .arg(probCurva, 0, 'f', 0));
}



