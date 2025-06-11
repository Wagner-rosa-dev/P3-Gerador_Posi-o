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

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:

    void gameTick();


private:
    void setupLineQuadVAO();
    void setupTractorGL();

    QElapsedTimer m_fpsTime;
    int m_frameCount;
    QElapsedTimer m_tempReadTimer;


    QTimer m_timer;
    camera m_camera;

    terrainmanager m_terrainManager;

    QOpenGLShaderProgram m_terrainShaderProgram;
    QOpenGLShaderProgram m_lineShaderProgram;

    QOpenGLExtraFunctions *m_extraFunction;

    QOpenGLVertexArrayObject m_lineQuadVao;
    QOpenGLBuffer m_lineQuadVbo;


    QOpenGLShaderProgram m_tractorShaderProgram;
    QOpenGLVertexArrayObject m_tractorVao;
    QOpenGLBuffer m_tractorVbo;

    QVector3D m_tractorPosition;
    float m_tractorRotation; // em graus
};

#endif // MYGLWIDGET_H
