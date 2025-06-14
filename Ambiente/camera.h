#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

class camera{
public:
    camera();
    void lookAt(const QVector3D &position, const QVector3D &target, const QVector3D &up);
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane);
    QVector3D position() const;
    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    //Metodos de movimento
    void moveForward(float amount);
    void strafeRight(float amount);
    void moveUp(float amount);
    void yaw(float degrees);
    void pitch(float degrees);
private:
    float m_yaw;
    float m_pitch;
    void updateCameraVectors();
    QVector3D m_position;
    QVector3D m_front;
    QVector3D m_up;
    QVector3D m_right;
    QVector3D m_worldUp;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
};
#endif // CAMERA_H
