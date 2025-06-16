#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QTimer>
#include "camera.h"
#include "terrainmanager.h"
#include <QElapsedTimer>
#include "speedcontroller.h"

struct SceneMatrices {
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;
};
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    MyGLWidget(QWidget *parent = nullptr);
    ~MyGLWidget();
signals:
    //Sinais sao como anuncios que a classe faz
    void fpsUpdated(int fps);
    void tempUpdated(float temp);
    void kmUpdated(float km);
    void coordinatesUpdate(float lon, float lat);
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void keyPressEvent(QKeyEvent *event) override;
private slots:
    void gameTick();
    void onSpeedUpdate(float newSpeed); //slot para receber a velocidade
    void onSteeringUpdate(int steeringValue);
private:
    void setupLineQuadVAO();
    void setupTractorGL();
    float m_tractorRotation; // em graus
    int m_frameCount;
    QTimer m_timer;
    camera m_camera;
    terrainmanager m_terrainManager;
    QElapsedTimer m_fpsTime;
    QElapsedTimer m_tempReadTimer;
    QVector3D m_tractorPosition;
    QOpenGLExtraFunctions *m_extraFunction;
    QOpenGLVertexArrayObject m_lineQuadVao;
    QOpenGLVertexArrayObject m_tractorVao;
    QOpenGLBuffer m_lineQuadVbo;
    QOpenGLBuffer m_tractorVbo;
    QOpenGLShaderProgram m_terrainShaderProgram;
    QOpenGLShaderProgram m_lineShaderProgram;
    QOpenGLShaderProgram m_tractorShaderProgram;
    SpeedController *m_speedController;
    float m_tractorSpeed;
    int m_steeringValue;
};
#endif // MYGLWIDGET_H
