#include "mainwindow.h"
#include "myglwidget.h"
#include <QLabel>
#include <QGridLayout>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)

{
    //1 Cria os widget que farão parte da janela
    m_glWidget = new MyGLWidget(this);
    m_fpsLabel = new QLabel("FPS: --", this);
    m_tempLabel = new QLabel("CPU: -- °C", this);
    m_kmLabel = new QLabel("Velovidade: 0.0 km/h", this);

    //2 Estiliza os labels para que fiquem bem visiveis
    QFont labelFont("Arial", 12, QFont::Bold);
    m_fpsLabel->setFont(labelFont);
    m_fpsLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px");
    m_tempLabel->setFont(labelFont);
    m_tempLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px");
    m_kmLabel->setFont(labelFont);
    m_kmLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,100); padding: 2px");


    //Cria um layout de grade par asobrepor os widgets
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0,0,0,0); //Remove margens

    //Adiciona a tela OpenGL ocupando todo o espaço da grade
    mainLayout->addWidget(m_glWidget, 0, 0);

    //Adiciona os labels na mesma celula da grade, mas com alinhamento diferente
    mainLayout->addWidget(m_fpsLabel, 0, 0, Qt::AlignTop | Qt::AlignLeft);
    mainLayout->addWidget(m_tempLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);
    mainLayout->addWidget(m_kmLabel, 0, 0, Qt::AlignBottom | Qt::AlignLeft);

    //Define o layout principal para a janela
    setLayout(mainLayout);

    //Sinais e Slots
    //Conecta o sinal fpsUpdated do m_glWdiget ao slot updateFpsLabel desta janela
    connect(m_glWidget, &MyGLWidget::fpsUpdated, this, &MainWindow::updateFpsLabel);
    //Conecta o sinal tempUpdated do m_glWidget ao slote updatetempLabel desta janela
    connect(m_glWidget, &MyGLWidget::tempUpdated, this, &MainWindow::updateTempLabel);
    //Conecta o sinal tempUpdated do m_glWidget ao slote updatetempLabel desta janela
    connect(m_glWidget, &MyGLWidget::kmUpdated, this, &MainWindow::updateKmLabel);

    resize(800, 600);
}

MainWindow::~MainWindow()
{}

void MainWindow::updateFpsLabel(int fps)
{
    m_fpsLabel->setText(QString("FPS: %1").arg(fps));
}

void MainWindow::updateTempLabel(float temp)
{
    m_tempLabel->setText(QString("CPU: %1 °C").arg(temp, 0, 'f', 1));
}

void MainWindow::updateKmLabel(float km)
{
    m_kmLabel->setText(QString("Velocidade: %1 Km/h").arg(km, 0, 'f', 1));
}

