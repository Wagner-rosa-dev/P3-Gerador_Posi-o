#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
//Declarações antecipadas para evitar inclusões circulares
class QLabel;
class MyGLWidget;
class MainWindow : public QWidget
{
    Q_OBJECT  // Macro necessaria para classes com sinais e Slots
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFpsLabel(int fps);
    void updateTempLabel(float temp);
    void updateKmLabel(float km);
    void updateCoordinatesLabel(float lon, float lat);

private:
    MyGLWidget *m_glWidget;
    QLabel *m_fpsLabel;
    QLabel *m_tempLabel;
    QLabel *m_kmLabel;
    QLabel *m_latLabel;
    QLabel *m_lonlabel;
};
#endif // MAINWINDOW_H
