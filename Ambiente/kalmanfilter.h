#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include <QMatrix4x4>
#include <QVector4D>
#include <QVector2D>
#include <QDateTime>
#include <QtMath>
#include <Eigen>
#include <Dense>
#include <QDebug>
#include "filterprofiles.h"
#include "linearkalmanfilter.h"

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
    explicit KalmanFilter(double initialX = 0.0, double initialZ = 0.0);

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
    UpdateResult update(double measuredX, double measuredZ);

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
    void reset(double initialX = 0.0, double initialZ = 0.0);

    void setProfile(const FilterProfile& profile);

    const Eigen::VectorXd& getState() const { return m_state; }
    const Eigen::MatrixXd& getCovariance() const { return m_P; }

    void setState(const Eigen::VectorXd& state, const Eigen::MatrixXd& covariance);


    bool isInitialized() const {
        return m_isInitialized;
    }


signals:
    void filteredPositionReady(const QVector3D& filteredPosition);


private:
    //Parametros do UKF
    int n_x;        // Dimensão do vetor de estado (4: Px, Pz, Vx, Vz)
    int n_z;        // Dimensão do vetor de medição (2: Px, Pz)
    double alpha;   // Parâmetro de espalhamento dos sigma points (0 < alpha <= 1)
    double beta;    // Parâmetro para incorporar conhecimento sobre a distribuição (beta >= 0, beta=2 para Gaussiana)
    double kappa;   // Parâmetro secundário de espalhamento (kappa >= 0)
    double lambda;  // Parâmetro de escala para os sigma points

    //Vetores de pesos para a media (Wm) e covariancia (Wc) dos sigma points
    Eigen::VectorXd Wm;
    Eigen::VectorXd Wc;

    QDateTime m_lastMeasurementTime;
    bool m_isInitialized; // Flag paar indicar se o filtro foi inicializado

    //Funções auxiliares
    //Modelo de processo Nao-linear (f): como o estado evolui no tempo
    Eigen::VectorXd processModel(const Eigen::VectorXd& x_prev, double dt);

    //Modelo de medição nao-linear (h): como as medição sao obtidas do estado
    Eigen::VectorXd measurementModel(const Eigen::VectorXd& x_state);

    //Geração dos Sigma Points a partir da média e covariancia
    Eigen::MatrixXd generateSigmaPoints(const Eigen::VectorXd& x_mean, const Eigen::MatrixXd& P_cov);

    //Calcula os pesos dos sigma points
    void calculateWeights();

    int m_n; // Dimensão do estado (e.g., 4 para x, z, vx, vz)
    int m_m; // Dimensão da medição (e.g., 2 para x, z)

    // Matrizes e vetores do UKF
    Eigen::VectorXd m_state;     // Vetor de estado [x, z, vx, vz]'
    Eigen::MatrixXd m_P;         // Matriz de covariância do erro do estado
    Eigen::MatrixXd m_Q;         // Matriz de covariância do ruído do processo
    Eigen::MatrixXd m_R;         // Matriz de covariância do ruído da medição

};

#endif // KALMANFILTER_H
