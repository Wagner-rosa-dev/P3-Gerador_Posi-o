#include "camera.h"
#include <QtMath>

/**
 * @brief Construtor da câmera.
 * Inicializa a posição, orientação e vetores de direção da câmera.
 */
camera::camera() :
    m_yaw(-90.0f), // Aponta inicialmente para o eixo Z negativo.
    m_pitch(0.0f), // Horizontal.
    m_position(0.0f, 20.0f, 30.0f), // Posição inicial no mundo.
    m_front(0.0f, 0.0f, -1.0f), // Vetor inicial 'para frente'.
    m_worldUp(0.0f, 1.0f, 0.0f) // Vetor 'up' global (Y positivo).
{
    updateCameraVectors(); // Calcula vetores iniciais com base nos ângulos.
}

/**
 * @brief Obtém a posição atual da câmera.
 * @return QVector3D da posição.
 */
QVector3D camera::position() const {
    return m_position;
}

/**
 * @brief Configura a câmera para "olhar" de uma posição para um alvo.
 * Atualiza a posição e recalcula os vetores e ângulos para alinhar a câmera.
 * @param position Posição da câmera.
 * @param target Ponto para onde a câmera está olhando.
 * @param up Vetor que define a direção "para cima".
 */
void camera::lookAt(const QVector3D &position, const QVector3D &target, const QVector3D &up){
    m_position = position;

    m_front = (target - m_position).normalized();
    m_right = QVector3D::crossProduct(m_front, up).normalized();
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();

    // Recalcula ângulos a partir do novo vetor 'front'.
    m_yaw = qRadiansToDegrees(atan2(m_front.z(), m_front.x()));
    m_pitch = qRadiansToDegrees(asin(m_front.y()));

    // Limita o pitch para evitar inversão da câmera.
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
}

/**
 * @brief Configura a matriz de projeção da câmera como perspectiva.
 * @param fov Campo de visão vertical.
 * @param aspect Razão de aspecto da viewport.
 * @param nearPlane Distância do plano de corte próximo.
 * @param farPlane Distância do plano de corte distante.
 */
void camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(fov, aspect, nearPlane, farPlane);
}

/**
 * @brief Retorna a matriz de visão atual da câmera.
 * @return QMatrix4x4 da matriz de visão.
 */
QMatrix4x4 camera::viewMatrix() const{
    QMatrix4x4 view;
    // Simula a câmera olhando de 'm_position' para 'm_position + m_front'.
    view.lookAt(m_position, m_position + m_front, m_up);
    return view;
}

/**
 * @brief Retorna a matriz de projeção atual da câmera.
 * @return QMatrix4x4 da matriz de projeção.
 */
QMatrix4x4 camera::projectionMatrix() const {
    return m_projectionMatrix;
}

/**
 * @brief Move a câmera para frente.
 * @param amount Quantidade de unidades para mover.
 */
void camera::moveForward(float amount) {
    m_position += m_front * amount;
}

/**
 * @brief Move a câmera para a direita (strafe).
 * @param amount Quantidade de unidades para mover.
 */
void camera::strafeRight(float amount) {
    m_position += m_right * amount;
}

/**
 * @brief Move a câmera para cima.
 * @param amount Quantidade de unidades para mover.
 */
void camera::moveUp(float amount) {
    m_position += m_worldUp * amount;
}

/**
 * @brief Rotaciona a câmera em torno do eixo Y local (guinada/yaw).
 * @param degrees Quantidade de graus para rotacionar.
 */
void camera::yaw(float degrees) {
    m_yaw += degrees;
    updateCameraVectors();
}

/**
 * @brief Rotaciona a câmera em torno do eixo X local (arremesso/pitch).
 * @param degrees Quantidade de graus para rotacionar.
 */
void camera::pitch(float degrees) {
    m_pitch += degrees;
    // Limita o pitch para evitar inversão da câmera.
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
    updateCameraVectors();
}

/**
 * @brief Atualiza os vetores 'front', 'right' e 'up' da câmera.
 * Chamada quando os ângulos de guinada ou arremesso são modificados para manter a orientação correta.
 */
void camera::updateCameraVectors() {
    QVector3D front;
    // Calcula componentes X, Y, Z do vetor 'front' usando trigonometria com yaw e pitch.
    front.setX(cos(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
    front.setY(sin(qDegreesToRadians(m_pitch)));
    front.setZ(sin(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));

    m_front = front.normalized(); // Normaliza para comprimento 1.

    // Recalcula 'right' e 'up' para serem ortogonais a 'front' e entre si.
    m_right = QVector3D::crossProduct(m_front, m_worldUp).normalized();
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();
}
