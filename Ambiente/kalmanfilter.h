#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector2D>
#include <QDateTime>

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
    QMatrix4x4 H;

    //Matriz de Covariancia do Ruido do processo (Q)
    //Representa a incerteza inerente ao nosso modelo de movimento
    QMatrix4x4 Q;

    //Matriz de Covariancia do ruido da medição (R)
    //Representa a incerteza (ruido) das medições do sensor (GPS)
    QMatrix4x4 R;

    QDateTime m_lastMeasurementTime; //Para calcular dt

    bool m_isInitialized; // Flag apra indicar se o filtro foi inicializado
};

#endif // KALMANFILTER_H
