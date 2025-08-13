#include "mainwindow.h"
#include "myglwidget.h"
#include "worldconfig.h"
#include "logger.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QSpinBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // Define um layout principal para a MainWindow
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Cria o nosso "baralho de cartas"
    m_stackedWidget = new QStackedWidget(this);

    // Adiciona o baralho ao layout principal
    mainLayout->addWidget(m_stackedWidget);

    // --- CRIA E ADICIONA AS TELAS AO BARALHO ---
    m_stackedWidget->addWidget(createSplashPage());     // Página 0
    m_stackedWidget->addWidget(createBarSizePage());    // Página 1
    m_stackedWidget->addWidget(createSectionPage());   // Página 2
    m_stackedWidget->addWidget(createSpacingPage());    // Página 3

    // A tela principal da aplicação será criada depois
    m_mainAppWidget = nullptr;

    // Timer para a tela de splash ir para a próxima página
    QTimer::singleShot(2000, this, &MainWindow::showNextPage);

    // Define o layout para a janela
    setLayout(mainLayout);
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateFpsLabel(int fps) { if(m_fpsLabel) m_fpsLabel->setText(QString("FPS: %1").arg(fps)); }
void MainWindow::updateKmLabel(float km) { if(m_kmLabel) m_kmLabel->setText(QString("Velocidade: %1 Km/h").arg(km, 0, 'f', 1)); }
void MainWindow::updateCoordinatesLabel(float lon, float lat) {
    if(m_lonLabel) m_lonLabel->setText(QString("Lon: %1").arg(lon, 0, 'f', 7));
    if(m_latLabel) m_latLabel->setText(QString("Lat: %1").arg(lat, 0, 'f', 7));
}
void MainWindow::updateMovementStatusLabel(const QString& status) { if(m_movementStatusLabel) m_movementStatusLabel->setText(QString("Status: %1").arg(status)); }
void MainWindow::updateImmStatus(const QString& status, double probReta, double probCurva) {
    if(m_immStatusLabel) m_immStatusLabel->setText(QString("Filtro: %1 (R: %2% C: %3%)")
                                  .arg(status)
                                  .arg(probReta, 0, 'f', 0)
                                  .arg(probCurva, 0, 'f', 0));
}

QWidget* MainWindow::createMainAppPage()
{
    QWidget* page = new QWidget(this);
    QGridLayout* layout = new QGridLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    m_mainAppWidget = new MyGLWidget(m_config, page);

    QFont labelFont("Arial", 12, QFont::Bold);
    m_fpsLabel = new QLabel("FPS: --", page);
    m_kmLabel = new QLabel("Velocidade: 0.0 km/h", page);
    m_lonLabel = new QLabel("Lon: 0.0", page);
    m_latLabel = new QLabel("Lat: 0.0", page);
    m_movementStatusLabel = new QLabel("Status: --", page);
    m_immStatusLabel = new QLabel("Filtro: --", page);

    for (QLabel* label : {m_fpsLabel, m_kmLabel, m_lonLabel, m_latLabel, m_movementStatusLabel, m_immStatusLabel}) {
        label->setFont(labelFont);
        label->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px");
    }

    connect(m_mainAppWidget, &MyGLWidget::fpsUpdated, this, &MainWindow::updateFpsLabel);
    connect(m_mainAppWidget, &MyGLWidget::kmUpdated, this, &MainWindow::updateKmLabel);
    connect(m_mainAppWidget, &MyGLWidget::coordinatesUpdate, this, &MainWindow::updateCoordinatesLabel);
    connect(m_mainAppWidget, &MyGLWidget::movementStatusUpdated, this, &MainWindow::updateMovementStatusLabel);
    connect(m_mainAppWidget, &MyGLWidget::immStatusUpdated, this, &MainWindow::updateImmStatus);

    layout->addWidget(m_mainAppWidget, 0, 0, 1, 3);
    layout->addWidget(m_fpsLabel, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(m_kmLabel, 0, 0, Qt::AlignBottom | Qt::AlignLeft);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_latLabel);
    rightLayout->addWidget(m_lonLabel);
    rightLayout->addWidget(m_movementStatusLabel);
    rightLayout->addWidget(m_immStatusLabel);
    rightLayout->addStretch();
    layout->addLayout(rightLayout, 0, 2, Qt::AlignBottom | Qt::AlignRight);

    return page;
}

void MainWindow::startMainApplication()
{
    if (!m_mainAppWidget) {
        QWidget* mainPage = createMainAppPage();
        m_stackedWidget->addWidget(mainPage);
    }
    m_stackedWidget->setCurrentWidget(m_mainAppWidget->parentWidget());
    showFullScreen();

    MY_LOG_INFO("SetupWizard", QString("Configuração finalizada e aplicação principal iniciada."));
}

void MainWindow::showNextPage()
{
    int nextIndex = m_stackedWidget->currentIndex() + 1;
    if (nextIndex < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(nextIndex);
    } else {
        startMainApplication();
    }
}

QWidget* MainWindow::createSplashPage()
{
    QLabel* splashScreen = new QLabel("Seu logo Aqui", this);
    splashScreen->setAlignment(Qt::AlignCenter);
    splashScreen->setStyleSheet("background-color: #34495e; color: white; font-size: 48px;");
    return splashScreen;
}

QWidget* MainWindow::createBarSizePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel* label = new QLabel("Qual o tamanho da barra (em metros)?", page);
    label->setStyleSheet("font-size: 24px; color: #ecf0f1;");

    QLineEdit* lineEdit = new QLineEdit(page);
    lineEdit->setValidator(new QDoubleValidator(0.1, 100.0, 2, lineEdit));
    lineEdit->setStyleSheet("font-size: 24px; padding: 10px;");
    lineEdit->setPlaceholderText("Ex: 12.5");

    QPushButton* button = new QPushButton("Confirmar", page);
    button->setStyleSheet("font-size: 22px; padding: 15px; background-color: #27ae60; color: white;");

    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addWidget(button);
    page->setStyleSheet("background-color: #34495e;");

    connect(button, &QPushButton::clicked, this, &MainWindow::showNextPage);
    connect(lineEdit, &QLineEdit::textChanged, this, [this](const QString& text){
         m_config.toolWidth = text.toFloat();
    });

    return page;
}

QWidget* MainWindow::createSectionPage() // Corrigido para "Sections"
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel* label = new QLabel("Quantas seções?", page);
    label->setStyleSheet("font-size: 24px; color: #ecf0f1;");

    QSpinBox* spinBox = new QSpinBox(page);
    spinBox->setRange(1, 7);
    spinBox->setStyleSheet("font-size: 24px; padding: 10px;");
    spinBox->setValue(1);

    QPushButton* button = new QPushButton("Confirmar", page);
    button->setStyleSheet("font-size: 22px; padding: 15px; background-color: #27ae60; color: white;");

    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addWidget(button);
    page->setStyleSheet("background-color: #34495e;");

    connect(button, &QPushButton::clicked, this, &MainWindow::showNextPage);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value){
         m_config.sectionCount = value;
    });

    return page;
}

QWidget* MainWindow::createSpacingPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel* label = new QLabel("Espaçamento das seções (em cm)?", page);
    label->setStyleSheet("font-size: 24px; color: #ecf0f1;");

    QLineEdit* lineEdit = new QLineEdit(page);
    lineEdit->setValidator(new QDoubleValidator(1.0, 200.0, 1, lineEdit));
    lineEdit->setStyleSheet("font-size: 24px; padding: 10px;");
    lineEdit->setPlaceholderText("Ex: 50.5");

    QPushButton* button = new QPushButton("Confirmar e Iniciar", page);
    button->setStyleSheet("font-size: 22px; padding: 15px; background-color: #2980b9; color: white;");

    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addWidget(button);
    page->setStyleSheet("background-color: #34495e;");

    connect(button, &QPushButton::clicked, this, &MainWindow::showNextPage);
    connect(lineEdit, &QLineEdit::textChanged, this, [this](const QString& text){
         m_config.sectionSpacing = text.toFloat();
    });

    return page;
}
