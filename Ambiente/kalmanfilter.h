#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector2D>
#include <QDateTime>
#include <QtMath>

//Estrutura Matrix2x2 para operções de matriz 2x2
//Descrição: Esta estrutura auxiliar é fundamental para lidar corretamente com as matrizes
//           de medição (R) e inovação (S) do filtro de kalman, que são naturalmente 2x2
//           neste cenário (2 medições). QMatrix4x4 não é ideal para operações com sub-matrizes
//           ou inversão de matrizes singulares criadas por essa disparidade de dimensão;.
struct Matrix2x2 {
    float data[2][2];

    //Construtor padrãp, inicializa com zeros
    Matrix2x2() {
        data[0][0] = data[0][1] = data[1][0] = data[1][1] = 0.0f;
    }

    //Construtor com valores
    Matrix2x2(float m00, float m01, float m10, float m11) {
        data[0][0] = m00; data[0][1] = m01;
        data[1][0] = m10; data[1][1] = m11;
    }

    //Operador de adição
    Matrix2x2 operator+(const Matrix2x2& other) const {
        return Matrix2x2(data[0][0] + other.data[0][0], data[0][1] + other.data[0][1],
                         data[1][0] + other.data[1][0], data[1][1] + other.data[1][1]);
    }

    //Operador de subtração
    Matrix2x2 operator-(const Matrix2x2& other) const {
        return Matrix2x2(data[0][0] - other.data[0][0], data[0][1] - other.data[0][1],
                         data[1][0] - other.data[1][0], data[1][1] - other.data[1][1]);
    }

    //Operador de multiplicação por vetor 2D (Matrix2x2 * QVector2D)
    QVector2D operator*(const QVector2D& vec) const {
        return QVector2D(data[0][0] * vec.x() + data[0][1] * vec.y(),
                         data[1][0] * vec.x() + data[1][1] * vec.y());
    }

    //Método para inversão de matriz 2x2
    //Retorna a matriz inversa e define "invertible" como true/false
    Matrix2x2 inverted(bool* invertible = nullptr) const {
        float det = data[0][0] * data[1][1] - data[0][1] * data[1][0];
        if (qAbs(det) < 1e-9) { //Checagem para singularidade (determinante muito proximo do zero)
            if (invertible) *invertible = false;
            return Matrix2x2(); // Retorna matriz nula se singular
        }
        if (invertible) *invertible = true;
        float invDet = 1.0f / det;
        return Matrix2x2(data[1][1] * invDet, -data[0][1] * invDet,
                         -data[1][0] * invDet, data[0][0] * invDet);
    }
};




// Classe: KalmanFilter
// Descrição: Implementa um filtro de Kalman linear para estimar a posição e velocidade 2D
//            (X, Z, Vx, Vz) de um objeto a partir de medições ruidosas.
//            É ideal para suavizar dados de GPS e fornecer uma estimativa mais precisa
//            e preditiva do estado do trator.
class KalmanFilter{

public:
    // Construtor: KalmanFilter
    // Descrição: Inicializa o filtro de Kalman com os estados e covariâncias iniciais.
    // Parâmetros:
    //   - initialX: Posição X inicial no mundo.
    //   - initialZ: Posição Z inicial no mundo.
    KalmanFilter(double initialX, double initialZ);

    // Método: predict
    // Descrição: Executa a fase de predição do filtro de Kalman.
    //            Usa o modelo de movimento do sistema para prever o próximo estado
    //            e sua covariância com base no intervalo de tempo decorrido (dt).
    // Parâmetros:
    //   - dt: Delta de tempo em segundos desde a última atualização/predição.
    void predict(double dt);

    // Método: update
    // Descrição: Executa a fase de atualização do filtro de Kalman.
    //            Incorpora uma nova medição (posição X, Z do GPS) para refinar
    //            a estimativa do estado e sua covariância.
    // Parâmetros:
    //   - measuredX: Posição X medida pelo GPS.
    //   - measuredZ: Posição Z medida pelo GPS.
    void update(double measuredX, double measuredZ);

    // Método: getStatePosition
    // Descrição: Retorna a posição (X, Z) estimada pelo filtro de Kalman.
    // Retorno: QVector2D contendo as coordenadas X e Z estimadas.
    QVector2D getStatePosition() const;

    //Método: getStateVelocity
    //Descrição: Retorna a posição(X, Z) estimada pelo filtro de kalman.
    //Retorno: QVector2D contendo os componentes de velocidade Vx e Vz estimadas
    QVector2D getStateVelocity() const;

    //Método: reset
    //Descrição: Reinicia o filtro com um novo estado inicial.
    //           Util se a incertez ficar muito alta ou se houver um salto brusco nos dados
    void reset(double newX, double newZ);

private:
    //vetor de estado (x): [Px, Pz, Vx, Vz]
    //Px, Pz = posição X e Z
    //Vx, Vz = velocidade em X e Z
    QVector4D x;

    //Matriz de Covariancia de erro de estimativa (P)
    //Representa a incerteza da nossa estimativa do estado.
    //Diagonal alta significa alta incerteza
    QMatrix4x4 P;

    //Matriz de Transição de Estado (F)
    //Descreve como o estado do sistema muda entre intervalos de tempo
    QMatrix4x4 F;

    //Matriz de Medição (H)
    //Mapeia o vetor de estado para o vetor de medição
    //Medimos Px e Pz diretamente.
    //QMatrix4x4 H;

    //Matriz de Covariancia do Ruido do processo (Q)
    //Representa a incerteza inerente ao nosso modelo de movimento
    QMatrix4x4 Q;

    // --- NOVO: Matriz de Covariância do Ruído da Medição (R) ---
    // Tipo: Matrix2x2
    // Descrição: Representa a incerteza (ruído) das medições do sensor (GPS).
    //            É uma matriz 2x2, pois medimos apenas Px e Pz.
    Matrix2x2 m_R_measurement; //

    QDateTime m_lastMeasurementTime; //Para calcular dt

    bool m_isInitialized; // Flag apra indicar se o filtro foi inicializado

};

#endif // KALMANFILTER_H
