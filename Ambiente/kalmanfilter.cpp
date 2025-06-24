#include "kalmanfilter.h"
#include <QDebug> // para mensagens de depuração
#include <QtMath> // Para qPow
#include "logger.h"

//Construtor: incializa o filtro com um estado inicial
KalmanFilter::KalmanFilter(double initialX, double initialZ)
    : m_isInitialized(false) // inicializa o flag de inicializaçõa como falso
{
    reset(initialX, initialZ); // chama o metodo reset para configurar o filtro
}

//Reseta o filtro para um novo estado inicial
//Descrição: Inicializa todos os vetores e matrizes do filtro de kalman para um novo estado
//           isso é util no inicio da aplicação ou se o filtro perder a estabilidade
//           devido a dados muito ruidosos ou inconsistentes
void KalmanFilter::reset(double newX, double newZ) {
    // Vetor de estado (x): [Px, Pz, Vx, Vz]
    //Posição inicial fornceida, velocidade inicial zero
    x = QVector4D(newX, newZ, 0.0, 0.0);

    //inicializa a matriz de covariancia do erro de estimativa (P)
    //queremos alta incerteza inicial, entao colocamos valores grandes na diagonal.
    // por exemplo, 1000 para posição, e 100 para velocidade
    P.setToIdentity();
    P(0,0) = 100.0; // Posição X
    P(1,1) = 100.0; // Posição Z
    P(2,2) = 10.0; // Velocidade X
    P(3,3) = 10.0; // Velocidade Z

    //Matrix de Transição de estado (F)
    //Descreve como o estado do sistema muda entre intervalos de tempo (dt)
    //sera atualizado dinamicamente em predict()
    F.setToIdentity();


    //Matrix de Covariancia do Ruido Do Processo (Q)
    //representa a iincertez ado nosso modelo de movimento
    //pequenos valores indicam que confiamos muito no nosso modelo (pouco aceleração desconhecida).
    //grandes valores indicam que o sistema pode mudar de forma inesperada
    float processNoisePos = 0.01; // ruido na posição
    float processNoiseVel = 0.001; // Ruido na velocidade (aceleração desconhecida
    Q.setToIdentity();
    Q(0,0) = qPow(processNoisePos, 2); //Ruido na previsão de Px
    Q(1,1) = qPow(processNoisePos, 2); //Ruido na previsão de Pz
    Q(2,2) = qPow(processNoiseVel, 2); //Ruido na previsão de Vx
    Q(3,3) = qPow(processNoiseVel, 2); //Ruido na previsão de Vz

    //Matrix de Covariancia do ruido da medição (R)
    //Representa a incerteza (ruido) das medições do sensor (GPS)
    //Usamos o quadrado da precisão para a variancia
    //È uma matriz 2x2 para as medições X e Z
    //Não mais uma QMatrix4x4 completa, mas sim uma Matrix2x2 conceitual
    //O valor 'gpsNoise' é o desvio padrão da precisão horizontal do GPS
    float gpsNoise = 2.0f; //Ex: 2 metros de precisão do GPS (desvio padrão)
    m_R_measurement = Matrix2x2(qPow(gpsNoise, 2), 0.0f,
                                0.0f, qPow(gpsNoise, 2));


    m_isInitialized = true;
    m_lastMeasurementTime = QDateTime::currentDateTime();
    MY_LOG_INFO("Kalman", "Filtro de kalman reiniciado com sucesso");
}

//Fase de Predição
//Descrição: Utiliza o modelo de movimento do sistema para prever o proximo estado
//           e sua covariancia com base no intervalo de tempo decorrido (dt)
void KalmanFilter::predict(double dt) {
    if (!m_isInitialized) {
        MY_LOG_WARNING("Kalman", "kalmanFilter não incializado. Chame resete() primeiro");
        return;
    }

    //Garante que o dt seja positivo e razoavel para evitar instabilidade
    if (dt <= 0) dt = 0.016; // Tempo minimo de 16ms (aprox. 60 FPS) se dt for zero ou negativo

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
//Descrição: Incorpora uma nova medição (posição X, Z do GPS) para refinar
//           e estimativa do estado e sua covariancia.
void KalmanFilter::update(double measuredX, double measuredZ) {
    if (!m_isInitialized) {
        MY_LOG_WARNING("Kalman", "KalmanFilter não inicializado. Chame reset() primeiro.");
        return;
    }

    //Vetor de medição (z) - 2x1: [measuredX, measuredZ]
    QVector2D z_measured(measuredX, measuredZ);


    // --- NOVO: Log da medição recebida (X, Z do mundo) ---
    // Esta é a leitura de posição X/Z que vem do seu GPS.
    MY_LOG_DEBUG("Kalman_Update", QString("IN z_measured(X,Z): %1,%2").arg(measuredX, 0, 'f', 3).arg(measuredZ, 0, 'f', 3));

    //A matriz H (medição) é conceitualmente 2x4:
    //H = [1 0 0 0]
    //    [0 1 0 0]
    //A medição prevista (H * x_previsto) é simplesmente [Px_previsto, Pz_previsto
    QVector2D predictedMeasurement = QVector2D(x[0], x[1]);

    //Inovação (y): A diferença entre o que o filtro previo e o que o sensor mediu
    //é um vetor 2x1
    QVector2D y = z_measured - predictedMeasurement;



    // --- NOVO: Log da inovação (diferença entre medido e previsto) ---
    MY_LOG_DEBUG("Kalman_Update", QString("IN y_innovation(X,Z): %1,%2")
                                      .arg(y.x(), 0, 'f', 3).arg(y.y(), 0, 'f', 3));

    //Matriz de Covariancia de Inovação (S)
    //S = H * P * H_transposto + R
    //H (2x4), P (4x4), H_transposto (4x2) -> H*P*H_transposto é (2x2)
    //R(2x2)
    //O resultado de H*P*H_transposto é a sub-matrix 2x2 superior esquerda de P
    Matrix2x2 HP_HT_sub; // Representa o resultado de H*P*H_transposto (que é o top-left)
    HP_HT_sub.data[0][1] = P(0,0); HP_HT_sub.data[0][1] = P(0,1);
    HP_HT_sub.data[1][0] = P(1,0); HP_HT_sub.data[1][1] = P(1,1);

    //S é uma Matrix2x2
    bool invertible_S;
    Matrix2x2 S = HP_HT_sub + m_R_measurement; //m_R_measurement ja é Matrix2x2
    Matrix2x2 S_inverted = S.inverted(&invertible_S);

    if (!invertible_S) {
        MY_LOG_ERROR("Kalman", "Covariância da inovação (S) é singular. Não é possível calcular inverso. Aumente Q ou R.");
        // Opcional: Resetar o filtro ou aumentar a incerteza para tentar recuperar.
        return;
    }

    //ganho de kalman (K)
    //K = P * H_transposto * S_inversa
    //P (4x4), H_transposto (4x2 conceitual), S_inversa (2x2) -> K (4x2)
    //A multiplicação P * H_transposto resulta nas duas primeiras coluans de P
    //VAmos preencher uma QMatrix4x4 K_mat, onde as udas ultimas colunas serão zero
    QMatrix4x4 K_mat;
    K_mat.fill(0.0f); // Inicializa com zeros

    for (int i = 0; i < 4; ++i) { // Linhas de K (para estados Px, Pz, Vx, Vz)
        // Multiplicação da linha i de P com as duas colunas de H_transposto (que são os dois primeiros elementos do vetor da base canônica)
        // e então por S_inversa (2x2)
        // K_mat(i, j) = (P(i,0)*H_T_col_0[0] + P(i,1)*H_T_col_0[1] + ...) * S_inv(0,j) + (...) * S_inv(1,j)
        // Como H_T é [1 0; 0 1; 0 0; 0 0], P * H_T é apenas a submatriz 4x2 das duas primeiras colunas de P.
        // K_mat(i,0) = (P(i,0) * S_inverted.data[0][0]) + (P(i,1) * S_inverted.data[1][0]);
        // K_mat(i,1) = (P(i,0) * S_inverted.data[0][1]) + (P(i,1) * S_inverted.data[1][1]);
        // Simplificado, a coluna j de K é P.column(0) * S_inverted.data[0][j] + P.column(1) * S_inverted.data[1][j]
        K_mat(i,0) = P(i,0) * S_inverted.data[0][0] + P(i,1) * S_inverted.data[1][0];
        K_mat(i,1) = P(i,0) * S_inverted.data[0][1] + P(i,1) * S_inverted.data[1][1];
    }

    // NOVO: Log do Ganho de Kalman (K)
    MY_LOG_DEBUG("Kalman_Update", QString("IN K(0,0)=%1 K(0,1)=%2 K(1,0)=%3 K(1,1)=%4")
                                      .arg(K_mat(0,0), 0, 'f', 3).arg(K_mat(0,1), 0, 'f', 3)
                                      .arg(K_mat(1,0), 0, 'f', 3).arg(K_mat(1,1), 0, 'f', 3));
    MY_LOG_DEBUG("Kalman_Update", QString("IN K(2,0)=%1 K(3,0)=%2 K(2,1)=%3 K(3,1)=%4")
                                      .arg(K_mat(2,0), 0, 'f', 3).arg(K_mat(3,0), 0, 'f', 3)
                                      .arg(K_mat(2,1), 0, 'f', 3).arg(K_mat(3,1), 0, 'f', 3));

    // Atualização do Estado: x = x + K * y
    // K (4x2), y (2x1) -> K*y (4x1)
    // A multiplicação é a soma das colunas de K ponderadas pelos elementos de y.
    x[0] = x[0] + K_mat(0,0)*y.x() + K_mat(0,1)*y.y();
    x[1] = x[1] + K_mat(1,0)*y.x() + K_mat(1,1)*y.y();
    x[2] = x[2] + K_mat(2,0)*y.x() + K_mat(2,1)*y.y();
    x[3] = x[3] + K_mat(3,0)*y.x() + K_mat(3,1)*y.y();

    // Atualização da Covariância: P = (I - K * H) * P
    // K (4x2), H (2x4 conceitual) -> K*H (4x4)
    // K*H resultará em uma matriz 4x4 onde as duas últimas colunas são zero.
    QMatrix4x4 KH_product;
    KH_product.fill(0.0f); // Inicializa com zeros
    for (int i = 0; i < 4; ++i) { // Linhas
        KH_product(i,0) = K_mat(i,0); // Primeira coluna de KH é a primeira coluna de K
        KH_product(i,1) = K_mat(i,1); // Segunda coluna de KH é a segunda coluna de K
    }
    // As colunas 2 e 3 de KH_product permanecem zero, como esperado para K*H

    QMatrix4x4 I;
    I.setToIdentity(); // Matriz identidade 4x4

    P = (I - KH_product) * P; // Atualiza a matriz de covariância


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
