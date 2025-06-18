#include "camera.h" // Inclui o cabeçalho da classe camera.
#include <QtMath>   // Inclui funções matemáticas do Qt, como qDegreesToRadians, qRadiansToDegrees, atan2, asin.

/**
 * @brief Construtor da classe camera.
 *
 * Inicializa os ângulos de guinada (yaw) e arremesso (pitch), a posição da câmera,
 * o vetor de direção para frente e o vetor 'up' global (mundo).
 * Em seguida, chama updateCameraVectors() para calcular os vetores iniciais da câmera.
 */
camera::camera() :
    m_yaw(-90.0f), // Inicializa o ângulo de guinada em -90 graus, apontando para o eixo Z negativo.
    m_pitch(0.0f), // Inicializa o ângulo de arremesso em 0 graus (horizontal).
    m_position(0.0f, 20.0f, 30.0f), // Define a posição inicial da câmera no mundo.
    m_front(0.0f, 0.0f, -1.0f), // Define o vetor inicial 'para frente'.
    m_worldUp(0.0f, 1.0f, 0.0f) // Define o vetor 'up' global (Y positivo).
{
    updateCameraVectors(); // Calcula os vetores 'front', 'right' e 'up' com base nos ângulos iniciais.
}

/**
 * @brief Retorna a posição atual da câmera.
 * @return QVector3D representando a posição da câmera.
 */
QVector3D camera::position() const {
    return m_position; // Retorna o valor do membro m_position.
}

/**
 * @brief Configura a matriz de visão para "olhar" de uma posição para um alvo.
 * @param position A posição da câmera no espaço do mundo.
 * @param target O ponto no espaço do mundo para o qual a câmera está olhando.
 * @param up O vetor que define a direção "para cima" da câmera.
 *
 * Atualiza a posição da câmera e recalcula os vetores 'front', 'right', 'up' e
 * os ângulos de guinada e arremesso para alinhar a câmera com o novo alvo.
 */
void camera::lookAt(const QVector3D &position, const QVector3D &target, const QVector3D &up){
    m_position = position; // Atualiza a posição da câmera com a nova posição fornecida.

    // Calcula o vetor 'para frente' normalizado, apontando da posição da câmera para o alvo.
    m_front = (target - m_position).normalized();

    // Calcula o vetor 'para direita' usando o produto vetorial de 'front' e 'up' global, e normaliza.
    m_right = QVector3D::crossProduct(m_front, up).normalized();

    // Calcula o vetor 'para cima' local da câmera usando o produto vetorial de 'right' e 'front', e normaliza.
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();

    // Calcula o ângulo de guinada (yaw) a partir dos componentes X e Z do vetor 'front'.
    // `atan2` é usado para obter o ângulo correto em todos os quadrantes. Converte de radianos para graus.
    m_yaw = qRadiansToDegrees(atan2(m_front.z(), m_front.x()));

    // Calcula o ângulo de arremesso (pitch) a partir do componente Y do vetor 'front'.
    // `asin` (arcsen) é usado para obter o ângulo vertical. Converte de radianos para graus.
    m_pitch = qRadiansToDegrees(asin(m_front.y()));

    // Limita o ângulo de arremesso para evitar inversão da câmera (gimbal lock).
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
}

/**
 * @brief Configura a matriz de projeção da câmera como perspectiva.
 * @param fov Campo de visão vertical em graus.
 * @param aspect Razão de aspecto da viewport (largura / altura).
 * @param nearPlane Distância do plano de corte próximo.
 * @param farPlane Distância do plano de corte distante.
 *
 * Redefine a matriz de projeção para uma matriz identidade e, em seguida,
 * aplica a transformação de perspectiva com os parâmetros fornecidos.
 */
void camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    m_projectionMatrix.setToIdentity(); // Reseta a matriz de projeção para a identidade.
    // Aplica a transformação de perspectiva à matriz de projeção.
    m_projectionMatrix.perspective(fov, aspect, nearPlane, farPlane);
}

/**
 * @brief Retorna a matriz de visão atual da câmera.
 * @return QMatrix4x4 representando a matriz de visão.
 *
 * Cria uma nova matriz de visão e a configura usando a função lookAt() interna do Qt,
 * baseada na posição da câmera, ponto de foco (posição + frente) e vetor 'up'.
 */
QMatrix4x4 camera::viewMatrix() const{
    QMatrix4x4 view; // Cria uma nova matriz 4x4.
    // Configura a matriz 'view' para simular a câmera olhando de 'm_position' para 'm_position + m_front',
    // com 'm_up' como a direção para cima.
    view.lookAt(m_position, m_position + m_front, m_up);
    return view; // Retorna a matriz de visão calculada.
}

/**
 * @brief Retorna a matriz de projeção atual da câmera.
 * @return QMatrix4x4 representando a matriz de projeção.
 */
QMatrix4x4 camera::projectionMatrix() const {
    return m_projectionMatrix; // Retorna o valor do membro m_projectionMatrix.
}

/**
 * @brief Move a câmera para frente.
 * @param amount A quantidade de unidades para mover.
 *
 * Adiciona um vetor proporcional ao vetor 'front' da câmera à sua posição.
 */
void camera::moveForward(float amount) {
    m_position += m_front * amount; // Move a posição da câmera na direção do vetor 'front' pelo 'amount'.
}

/**
 * @brief Move a câmera para a direita (strafe).
 * @param amount A quantidade de unidades para mover.
 *
 * Adiciona um vetor proporcional ao vetor 'right' da câmera à sua posição.
 */
void camera::strafeRight(float amount) {
    m_position += m_right * amount; // Move a posição da câmera na direção do vetor 'right' pelo 'amount'.
}

/**
 * @brief Move a câmera para cima.
 * @param amount A quantidade de unidades para mover.
 *
 * Adiciona um vetor proporcional ao vetor 'worldUp' global à sua posição.
 */
void camera::moveUp(float amount) {
    m_position += m_worldUp * amount; // Move a posição da câmera na direção do vetor 'worldUp' pelo 'amount'.
}

/**
 * @brief Rotaciona a câmera em torno do eixo Y local (guinada ou yaw).
 * @param degrees A quantidade de graus para rotacionar.
 *
 * Ajusta o ângulo de guinada e recalcula os vetores da câmera.
 */
void camera::yaw(float degrees) {
    m_yaw += degrees; // Adiciona os graus ao ângulo de guinada.
    updateCameraVectors(); // Recalcula os vetores 'front', 'right' e 'up' com o novo ângulo de guinada.
}

/**
 * @brief Rotaciona a câmera em torno do eixo X local (arremesso ou pitch).
 * @param degrees A quantidade de graus para rotacionar.
 *
 * Ajusta o ângulo de arremesso, limita-o para evitar inversão da câmera,
 * e recalcula os vetores da câmera.
 */
void camera::pitch(float degrees) {
    m_pitch += degrees; // Adiciona os graus ao ângulo de arremesso.
    // Limita o ângulo de arremesso para evitar que a câmera vire de cabeça para baixo.
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
    updateCameraVectors(); // Recalcula os vetores 'front', 'right' e 'up' com o novo ângulo de arremesso.
}

/**
 * @brief Atualiza os vetores 'front', 'right' e 'up' da câmera com base nos ângulos de guinada e arremesso.
 *
 * Esta função é chamada sempre que os ângulos de guinada ou arremesso são modificados,
 * garantindo que os vetores de orientação da câmera estejam sempre corretos.
 */
void camera::updateCameraVectors() {
    QVector3D front; // Declara um vetor temporário para o novo vetor 'front'.
    // Calcula o componente X do vetor 'front' usando trigonometria com yaw e pitch.
    front.setX(cos(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));

    // Calcula o componente Y do vetor 'front' usando trigonometria com pitch.
    front.setY(sin(qDegreesToRadians(m_pitch)));

    // Calcula o componente Z do vetor 'front' usando trigonometria com yaw e pitch.
    front.setZ(sin(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));

    m_front = front.normalized(); // Normaliza o vetor 'front' para que ele tenha comprimento 1.

    // Recalcula o vetor 'right' usando o produto vetorial entre 'front' e 'worldUp'. Normaliza.
    m_right = QVector3D::crossProduct(m_front, m_worldUp).normalized();

    // Recalcula o vetor 'up' usando o produto vetorial entre 'right' e 'front'. Normaliza.
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();
}
