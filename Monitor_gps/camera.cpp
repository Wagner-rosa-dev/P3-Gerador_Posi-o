#include "camera.h"
#include <QtMath>

camera::camera() :
    m_position(0.0f, 20.0f, 30.0f),
    m_front(0.0f, 0.0f, -1.0f),
    m_worldUp(0.0f, 1.0f, 0.0f),
    m_yaw(-90.0f),
    m_pitch(0.0f)
{
    updateCameraVectors();
}



QVector3D camera::position() const {
    return m_position;
}

void camera::lookAt(const QVector3D &position, const QVector3D &target, const QVector3D &up){
    m_position = position;

    m_front = (target - m_position).normalized();
    m_right = QVector3D::crossProduct(m_front, up).normalized();
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();
    m_yaw = qRadiansToDegrees(atan2(m_front.z(), m_front.x()));
    m_pitch = qRadiansToDegrees(asin(m_front.y()));
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
}

void camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(fov, aspect, nearPlane, farPlane);
}

QMatrix4x4 camera::viewMatrix() const{
    QMatrix4x4 view;
    view.lookAt(m_position, m_position + m_front, m_up);
    return view;
}

QMatrix4x4 camera::projectionMatrix() const {
    return m_projectionMatrix;
}

void camera::moveForward(float amount) {
    m_position += m_front * amount;
}

void camera::strafeRight(float amount) {
    m_position += m_right * amount;
}

void camera::moveUp(float amount) {
    m_position += m_worldUp * amount;
}

void camera::yaw(float degrees) {
    m_yaw += degrees;
    updateCameraVectors();
}

void camera::pitch(float degrees) {
    m_pitch += degrees;
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
    updateCameraVectors();
}

void camera::updateCameraVectors() {
    QVector3D front;
    front.setX(cos(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
    front.setY(sin(qDegreesToRadians(m_pitch)));
    front.setZ(sin(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
    m_front = front.normalized();
    m_right = QVector3D::crossProduct(m_front, m_worldUp).normalized();
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();

}
