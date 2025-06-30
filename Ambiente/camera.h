#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

// Classe: camera
// Gerencia a posição, orientação e projeção da câmera no mundo 3D,
// permitindo controlar a perspectiva da cena e simular movimentos.
class camera{
public:
    // Construtor: Inicializa a câmera com posição e orientação padrão.
    camera();

    // Configura a câmera para "olhar" de uma posição específica para um ponto alvo.
    void lookAt(const QVector3D &position, const QVector3D &target, const QVector3D &up);

    // Configura a matriz de projeção da câmera (perspectiva).
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane);

    // Obtém a posição atual da câmera no espaço do mundo.
    QVector3D position() const;

    // Retorna a matriz de visão atual, transformando coordenadas do mundo para o espaço da câmera.
    QMatrix4x4 viewMatrix() const;

    // Retorna a matriz de projeção atual, transformando coordenadas da câmera para o espaço de corte.
    QMatrix4x4 projectionMatrix() const;

    // Métodos de movimento: Ajustam a posição da câmera.
    // Move a câmera para frente.
    void moveForward(float amount);
    // Move a câmera para a direita (strafe).
    void strafeRight(float amount);
    // Move a câmera para cima.
    void moveUp(float amount);

    // Métodos de rotação: Ajustam a orientação da câmera.
    // Rotaciona a câmera em torno do eixo Y local (guinada).
    void yaw(float degrees);
    // Rotaciona a câmera em torno do eixo X local (arremesso).
    void pitch(float degrees);

private:
    // Ângulo de guinada da câmera em graus (rotação horizontal).
    float m_yaw;
    // Ângulo de arremesso da câmera em graus (rotação vertical), limitado para evitar inversão.
    float m_pitch;

    // Recalcula os vetores `m_front`, `m_right` e `m_up` com base nos ângulos de guinada e arremesso.
    void updateCameraVectors();

    // Posição da câmera no espaço do mundo.
    QVector3D m_position;
    // Vetor unitário que aponta para a frente da câmera.
    QVector3D m_front;
    // Vetor unitário que aponta para cima em relação à câmera (eixo Y local).
    QVector3D m_up;
    // Vetor unitário que aponta para a direita da câmera (eixo X local).
    QVector3D m_right;
    // Vetor unitário que define a direção global "para cima" no mundo (geralmente (0, 1, 0)).
    QVector3D m_worldUp;
    // Matriz de projeção da câmera (perspectiva).
    QMatrix4x4 m_projectionMatrix;
};

#endif // CAMERA_H
