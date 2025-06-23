#include "kalmanfilter.h"
#include <QDebug> // para mensagens de depuração
#include <QtMath> // Para qPow
#include "logger.h"

//Construtor: incializa o filtro com um estado inicial
KalmanFilter::KalmanFilter(double initialX, double initialZ)
    : m_isInitialized(false)
{
    reset(initialX, initialZ);
}

//Reseta o filtro para um novo estado inicial
void KalmanFilter::reset(double newX, double newZ) {
    // inicializa o vetor de estado (x)
    x = QVector4D(newX, newZ, 0.0, 0.0); // Posição inicial, velocidade inicial zero

    //inicializa a matriz de covariancia do erro de estimativa (P)
    //queremos alta incerteza inicial, entao colocamos valores grandes na diagonal.
    // por exemplo, 1000 para posição, e 100 para velocidade
    P.setToIdentity();
    P(0,0) = 1000.0; // Posição X
    P(1,1) = 1000.0; // Posição Z
    P(2,2) = 100.0; // Velocidade X
    P(3,3) = 100.0; // Velocidade Z

    //Matriz de medição (H)
    // Nosso sensor (GPS) mede Px e Pz. Então, H mapeia [Px, Pz, Vx, Vz] para [Px, Pz_medido]
    H.setToIdentity();
    H(2,2) = 0.0; // Não medimos Vx diretamente neste ponto (será 0 na medição)
    H(3,3) = 0.0; // Não medimos Vz diretamente neste ponto (será 0 na medição)
    // A medição será um QVector 2D(measuredX, measuredZ)

    //A matriz de Covariancia do ruido de medição (R)
    //Representa o quão ruidosa são nossas medições do GPS.
    //As especificações do GPS mencionam 2.0 metros de precição horizontal
    //Usamos o quadrado da precisão para a variancia.
    R.setToIdentity();
    float gpsNoise = 2.0f; // 2 metros de precisão do GPS
    R(0,0) = qPow(gpsNoise, 2); //Ruido na medição de Px
    R(1,1) = qPow(gpsNoise, 2); //Ruido de medição de Pz
    // se a medição do GPS for apenas Px e Pz, R seria 2x2
    // se estivessemos mdindo Vx e Vz do GPS, teriamos mais termos na R e H seria 4x4
    // por agora, H é 2x4 (mapeia 4 estados para 2 medição) e R é 2x2
    // para simplificar, vou considerar que nosso 'H' e 'R' são 4x4 e que as ultimas duas linhas/colunas
    //relacionandas a velocidade da medição são zero ou muito grande sse a medição de velocidade não for usada.
    // no entendo, é mais correto fazer a matriz H e R 2x4 e 2x2 respectivamente
    // para um modelo de kalmn linerar 2D com medições de posição (X, Z), H é uma matriz 2x4:
    //H = [1 0 0 0]
    //    [0 1 0 0]
    // e R é 2x2

    //Correção: redefinindo H e R para o caso de medir apenas Px, e pZ
    H = QMatrix4x4(); //Zera a matriz
    H(0,0) = 1.0; // Medimos Px
    H(1,1) = 1.0; // Medimos Pz

    R = QMatrix4x4(); // Zera a mtriz
    R(0,0) = qPow(gpsNoise, 2); //Ruido na medição de Px
    R(1,1) = qPow(gpsNoise, 2); // Ruido na medição de Pz
    //os outros termos de R(relacionados a Vx, Vz) não são aplicaveis pois não medimos diretamente.
    //Para um filtro de kalman de 4 estados com 2 medições (Px, Pz)
    //é ecessario que R seja 2x2 e H seja 2x4
    //as operações de matrizes precsiarão ser ajustadas para compatibilidade
    //no QMatrix4x4, se usarmos apenas os primeiros 2x2 e 2x4, funciona

    //Matrix de Covariancia do Ruido Do Processo (Q)
    //representa a iincertez ado nosso modelo de movimento
    //pequenos valores indicam que confiamos muito no nosso modelo (pouco aceleração desconhecida).
    //grandes valores indicam que o sistema pode mudar de forma inesperada
    float processNoisePos = 0.1; // ruido na posição
    float processNoiseVel = 0.01; // Ruido na velocidade (aceleração desconhecida
    Q.setToIdentity();
    Q(0,0) = qPow(processNoisePos, 2); //Ruido na previsão de Px
    Q(1,1) = qPow(processNoisePos, 2); //Ruido na previsão de Pz
    Q(2,2) = qPow(processNoiseVel, 2); //Ruido na previsão de Vx
    Q(3,3) = qPow(processNoiseVel, 2); //Ruido na previsão de Vz

    m_isInitialized = true;
    m_lastMeasurementTime = QDateTime::currentDateTime();
}

//Fase de Predição
void KalmanFilter::predict(double dt) {
    if (!m_isInitialized) {
        MY_LOG_WARNING("Kalman", "kalmanFilter não incializado. Chame resete() primeiro");
        return;
    }

    //novo log do estado antes da predição
    //x[0]=Px, x[1]=Pz, x[2]=Vx, x[3]=Vz.
    MY_LOG_DEBUG("kalman_predict", QString("IN x_antes_pred: Px=%1 Pz=%2 Vx=%3 Vz=%4")
                                        .arg(x[0], 0, 'f', 3).arg(x[1], 0, 'f', 3)
                                        .arg(x[2], 0, 'f', 3).arg(x[3], 0, 'f', 3));
    //log da incerteza (variancia da posição) ANTES da predição
    //P(0,0) é a variancia da posição X, P(1,1) é a variancia da posição Z
    //Valores altos siginifcam mais incerteza
    MY_LOG_DEBUG("kalman_Predict", QString("in P_antes_pred(0,0)=%1 P(1,1)=%2")
                                       .arg(P(0,0), 0, 'f', 3).arg(P(1,1), 0, 'f', 3));
    //Atualiza a matriz de transição de estado (F) com o delta de tempo
    //Px_novo = Px_atual + Vx_atual * dt
    //Pz_novo = Pz_atual + Vz_atual * dt
    //Vx_novo = Vx_atual
    //Vz_novo = Vz_atual
    F.setToIdentity();
    F(0,2) = dt; // Px depende de Vx * dt
    F(1,3) = dt; // Pz depende de Vz * dt

    //Predição do estado (x = F * x_anterior)
    x = F * x;

    //Predição da Covariancia (P = F * P_anterior * F^T + Q)
    P = F * P * F.transposed() + Q;

    //log do estado Depois da predição
    MY_LOG_DEBUG("kalman_Predict", QString("OUT x_pred: Px=%1 Pz=%2 Vx=%3 Vz=%4")
                                        .arg(x[0], 0, 'f', 3).arg(x[1], 0, 'f', 3)
                                        .arg(x[2], 0, 'f', 3).arg(x[3], 0, 'f', 3));
    //log da incerteza (variancias da posição) depois da predição
    MY_LOG_DEBUG("Kalman_predict", QString("OUT P_pred(0,0)=%1 P(1,1)=%2")
                                       .arg(P(0,0), 0, 'f', 3).arg(P(1,1), 0, 'f', 3));
}

//FAse de atualização
void KalmanFilter::update(double measuredX, double measuredZ) {
    if (!m_isInitialized) {
        MY_LOG_WARNING("Kalman", "KalmanFilter não inicializado. Chame reset() primeiro.");
        return;
    }

    QVector2D z_2d(measuredX, measuredZ); // Medição 2D (não usada diretamente para o cálculo)

    // --- NOVO: Log da medição recebida (X, Z do mundo) ---
    // Esta é a leitura de posição X/Z que vem do seu GPS.
    MY_LOG_DEBUG("Kalman_Update", QString("IN z_measured(X,Z): %1,%2").arg(measuredX, 0, 'f', 3).arg(measuredZ, 0, 'f', 3));

    QVector4D z_expanded(measuredX, measuredZ, 0.0, 0.0); // Medição expandida para 4D

    // Inovação: A diferença entre o que o filtro previu e o que o sensor mediu.
    QVector4D y = z_expanded - (H * x); // 'y' é a inovação, use 'y' para evitar conflito com 'y_'

    // --- NOVO: Log da inovação (diferença entre medido e previsto) ---
    // Valores altos em y[0] e y[1] indicam que a previsão do filtro e a medição do GPS estão muito diferentes.
    MY_LOG_DEBUG("Kalman_Update", QString("IN y_innovation(X,Z,Vx,Vz): %1,%2,%3,%4")
                                      .arg(y[0], 0, 'f', 3).arg(y[1], 0, 'f', 3)
                                      .arg(y[2], 0, 'f', 3).arg(y[3], 0, 'f', 3));

    QMatrix4x4 S = H * P * H.transposed() + R; // Covariância da inovação

    QMatrix4x4 S_inverted = S.inverted();
    if (S_inverted.isIdentity()) { // isIdentity() é uma forma simples de verificar se é singular
        MY_LOG_ERROR("Kalman", "Inovação de covariancia (S) é singular. Não é possível calcular inverso.");
        return;
    }
    QMatrix4x4 K = P * H.transposed() * S_inverted; // Ganho de Kalman

    // --- NOVO: Log do Ganho de Kalman (K) ---
    // Os elementos da diagonal principal (K(0,0), K(1,1)) indicam o quanto o filtro
    // ajustará sua estimativa de posição com base na nova medição.
    // Valores próximos de 1.0 significam que o filtro confia muito na medição.
    // Valores próximos de 0.0 significam que o filtro confia mais na sua previsão.
    MY_LOG_DEBUG("Kalman_Update", QString("IN K(0,0)=%1 K(0,1)=%2 K(1,0)=%3 K(1,1)=%4")
                                      .arg(K(0,0), 0, 'f', 3).arg(K(0,1), 0, 'f', 3)
                                      .arg(K(1,0), 0, 'f', 3).arg(K(1,1), 0, 'f', 3));
    MY_LOG_DEBUG("Kalman_Update", QString("IN K(2,2)=%1 K(3,3)=%2")
                                      .arg(K(2,2), 0, 'f', 3).arg(K(3,3), 0, 'f', 3));


    x = x + (K * y); // Atualização do Estado: O estado é corrigido usando o ganho de Kalman e a inovação.

    QMatrix4x4 I;
    I.setToIdentity();
    P = (I - K * H) * P; // Atualização da Covariância: A incerteza diminui após a atualização.

    // --- NOVO: Log do estado FINAL (Px, Pz, Vx, Vz) após atualização ---
    // Esta é a estimativa mais precisa do filtro para a posição e velocidade.
    MY_LOG_DEBUG("Kalman_Update", QString("OUT x_est: Px=%1 Pz=%2 Vx=%3 Vz=%4")
                                      .arg(x[0], 0, 'f', 3).arg(x[1], 0, 'f', 3)
                                      .arg(x[2], 0, 'f', 3).arg(x[3], 0, 'f', 3));
    // --- NOVO: Log da incerteza (variâncias da posição) FINAL após atualização ---
    // P(0,0) e P(1,1) devem ser menores que os valores de 'P_antes_pred' da fase de predição.
    MY_LOG_DEBUG("Kalman_Update", QString("OUT P_est(0,0):%1 P_est(1,1):%2")
                                      .arg(P(0,0), 0, 'f', 3).arg(P(1,1), 0, 'f', 3));
}
//Retorna a posição (X, Z) estimada
QVector2D KalmanFilter::getStatePosition() const {
    return QVector2D(x[0], x[1]);
}

//Retorna a velocidade (Vx, Vz) estimada
QVector2D KalmanFilter::getStateVelocity() const {
    return QVector2D(x[2], x[3]);
}
