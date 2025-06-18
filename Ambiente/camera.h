#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D> // Para representação de vetores 3D (posição, frente, cima, direita).
#include <QMatrix4x4> // Para representação de matrizes 4x4 (matrizes de visão e projeção).

// Classe: camera
// Descrição: Gerencia a posição, orientação e projeção da câmera no mundo 3D.
//            Esta classe fornece métodos para controlar a perspectiva da cena e
//            simular movimentos de câmera, como mover-se para frente, para os lados,
//            para cima, e rotacionar (guinada e arremesso).
class camera{
public:
    // Construtor: camera
    // Descrição: Inicializa a câmera com uma posição e orientação padrão.
    camera();

    // Método: lookAt
    // Descrição: Configura a matriz de visão da câmera para "olhar" de uma posição
    //            específica para um ponto alvo, com uma orientação "para cima" definida.
    // Parâmetros:
    //   - position: A posição do observador no espaço do mundo.
    //   - target: O ponto no espaço do mundo para o qual a câmera está olhando.
    //   - up: O vetor que define a direção "para cima" da câmera (geralmente (0, 1, 0) para o eixo Y).
    void lookAt(const QVector3D &position, const QVector3D &target, const QVector3D &up);

    // Método: setPerspective
    // Descrição: Configura a matriz de projeção da câmera para uma projeção em perspectiva.
    //            Isso define como os objetos 3D são mapeados para a tela 2D, criando a ilusão de profundidade.
    // Parâmetros:
    //   - fov: Campo de visão vertical em graus.
    //   - aspect: Razão de aspecto da viewport (largura / altura).
    //   - nearPlane: Distância do plano de corte próximo. Objetos mais próximos que isso não são renderizados.
    //   - farPlane: Distância do plano de corte distante. Objetos mais distantes que isso não são renderizados.
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane);

    // Método: position
    // Descrição: Retorna a posição atual da câmera no espaço do mundo.
    // Retorno: QVector3D - A posição da câmera.
    QVector3D position() const;

    // Método: viewMatrix
    // Descrição: Retorna a matriz de visão atual da câmera. A matriz de visão transforma
    //            coordenadas do espaço do mundo para o espaço da câmera.
    // Retorno: QMatrix4x4 - A matriz de visão.
    QMatrix4x4 viewMatrix() const;

    // Método: projectionMatrix
    // Descrição: Retorna a matriz de projeção atual da câmera. A matriz de projeção
    //            transforma coordenadas do espaço da câmera para o espaço de corte (clip space).
    // Retorno: QMatrix4x4 - A matriz de projeção.
    QMatrix4x4 projectionMatrix() const;

    // Métodos de movimento da câmera:
    // Descrição: Estes métodos ajustam a posição da câmera em relação à sua orientação atual.

    // Método: moveForward
    // Descrição: Move a câmera para frente (na direção do vetor `m_front`).
    // Parâmetros:
    //   - amount: A quantidade de unidades para mover.
    void moveForward(float amount);

    // Método: strafeRight
    // Descrição: Move a câmera para a direita (na direção do vetor `m_right`).
    // Parâmetros:
    //   - amount: A quantidade de unidades para mover.
    void strafeRight(float amount);

    // Método: moveUp
    // Descrição: Move a câmera para cima (na direção do vetor `m_worldUp`).
    // Parâmetros:
    //   - amount: A quantidade de unidades para mover.
    void moveUp(float amount);

    // Método: yaw
    // Descrição: Rotaciona a câmera em torno do eixo Y local (guinada).
    //            Afeta a direção horizontal da câmera.
    // Parâmetros:
    //   - degrees: A quantidade de graus para rotacionar.
    void yaw(float degrees);

    // Método: pitch
    // Descrição: Rotaciona a câmera em torno do eixo X local (arremesso).
    //            Afeta a direção vertical da câmera.
    // Parâmetros:
    //   - degrees: A quantidade de graus para rotacionar.
    void pitch(float degrees);

private:
    // Membro: m_yaw
    // Tipo: float
    // Descrição: O ângulo de guinada da câmera em graus. Controla a rotação horizontal.
    float m_yaw;

    // Membro: m_pitch
    // Tipo: float
    // Descrição: O ângulo de arremesso da câmera em graus. Controla a rotação vertical.
    float m_pitch;

    // Método: updateCameraVectors
    // Descrição: Método privado que recalcula os vetores `m_front`, `m_right` e `m_up`
    //            sempre que a orientação da câmera (yaw/pitch) é alterada.
    void updateCameraVectors();

    // Membro: m_position
    // Tipo: QVector3D
    // Descrição: A posição da câmera no espaço do mundo.
    QVector3D m_position;

    // Membro: m_front
    // Tipo: QVector3D
    // Descrição: O vetor unitário que aponta para a frente da câmera.
    QVector3D m_front;

    // Membro: m_up
    // Tipo: QVector3D
    // Descrição: O vetor unitário que aponta para cima em relação à câmera (eixo Y local).
    QVector3D m_up;

    // Membro: m_right
    // Tipo: QVector3D
    // Descrição: O vetor unitário que aponta para a direita da câmera (eixo X local).
    QVector3D m_right;

    // Membro: m_worldUp
    // Tipo: QVector3D
    // Descrição: O vetor unitário que define a direção global "para cima" no mundo
    //            (usado para calcular `m_right` e `m_up`). Geralmente (0, 1, 0).
    QVector3D m_worldUp;

    // Membro: m_projectionMatrix
    // Tipo: QMatrix4x4
    // Descrição: A matriz de projeção da câmera (perspectiva).
    QMatrix4x4 m_projectionMatrix;

    // Membro: m_viewMatrix
    // Tipo: QMatrix4x4
    // Descrição: A matriz de visão da câmera.
    QMatrix4x4 m_viewMatrix;
};

#endif // CAMERA_H
