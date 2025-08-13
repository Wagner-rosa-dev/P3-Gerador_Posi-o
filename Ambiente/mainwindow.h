#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "worldconfig.h"

class QStackedWidget;
class MyGLWidget;
class QLabel;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showNextPage();
    void startMainApplication();

    // Slots para atualizar os labels da UI
    void updateFpsLabel(int fps);
    void updateKmLabel(float km);
    void updateCoordinatesLabel(float lon, float lat);
    void updateMovementStatusLabel(const QString& status);
    void updateImmStatus(const QString& status, double probReta, double probCurva);

private:
    QStackedWidget *m_stackedWidget;
    MyGLWidget* m_mainAppWidget;
    WorldConfig m_config;

    // Ponteiros para os labels que exibirão as informações
    QLabel *m_fpsLabel;
    QLabel *m_kmLabel;
    QLabel *m_latLabel;
    QLabel *m_lonLabel;
    QLabel *m_movementStatusLabel;
    QLabel *m_immStatusLabel;

    QWidget* createSplashPage();
    QWidget* createBarSizePage();
    QWidget* createSectionPage();
    QWidget* createSpacingPage();
    QWidget* createMainAppPage();

};

#endif // MAINWINDOW_H
