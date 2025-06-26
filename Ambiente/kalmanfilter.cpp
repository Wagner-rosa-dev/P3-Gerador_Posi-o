#include "kalmanfilter.h"
#include "logger.h"

//Construtor: incializa o filtro com um estado inicial
KalmanFilter::KalmanFilter(double initialX, double initialZ)
   : n_x(4), //Define a dimensao do estado
    n_z(2), // Define a dimensão da medição
    alpha(0.001), // Valores tipicos, ajuste fino pode ser necessario
    beta(2.0), //Valor ideal para contibuição Gaussiana
    kappa(0.0), //Geralmente 0 para sistemas de baixo dimensionalidade
    m_isInitialized(false) // inicializa o flag de inicializaçõa como falso

{
    reset(initialX, initialZ); // chama o metodo reset para configurar o filtro
}

void KalmanFilter::calculateWeights() {
    //Calcula lambda
    lambda = alpha * alpha * (n_x + kappa) - n_x;

    //Redimensiona e inicializa os vetores de pesos
    Wm.resize(2 * n_x + 1);
    Wc.resize(2 * n_x + 1);

    //Peso para o primeiro sigma point (o proprio estado médio)
    Wm(0) = lambda / (n_x + lambda);
    Wc(0) = lambda / (n_x + lambda) + (1 - alpha * alpha + beta);

    //Pesos para os demais sigma points
    for (int i = 1; i <= 2 * n_x; ++i) {
        Wm(i) = 1.0 / (2.0 * (n_x +lambda));
        Wc(i) = 1.0 / (2.0 * (n_x + lambda));
    }
    MY_LOG_DEBUG("kalman", QString("Pesos do UKF calculado. Lambda: %1").arg(lambda));
}




//Reseta o filtro para um novo estado inicial
//Descrição: Inicializa todos os vetores e matrizes do filtro de kalman para um novo estado
//           isso é util no inicio da aplicação ou se o filtro perder a estabilidade
//           devido a dados muito ruidosos ou inconsistentes
void KalmanFilter::reset(double newX, double newZ) {
    //Redimensiona o vetor de estado para n_X dimensoes e inicializa com zeros
    x.resize(n_x);
    x.setZero();
    x(0) = newX; //Posição X inicial
    x(1) = newZ; //Posição Z inicial

    //Redimensiona a matriz de covariancia do erro de estimativa (P) para n_x por n_x
    // e inicializa com uma alta incerteza na diagonal
    P.resize(n_x, n_x);
    P.setIdentity(); // Define como matriz identidade
    P(0,0) = 100.0; // Incerteza na Posição X
    P(1,1) = 100.0; // Incerteza na Posição Z
    P(2,2) = 10.0;  //Incerteza na Velocidade X
    P(3,3) = 10.0; // Incerteza na Velocidade Z

    //Redimensiona a matriz de covariancia do ruido do processo (Q) para n_x por n_x
    //e inicializa (valroes baixos se o modelo de movimento for confiavel)
    Q.resize(n_x, n_x);
    Q.setZero(); // Inicializa com zeros

    //Velores para uido do processo (Q) - ajuste fino necessario
    //Esses valores representam a incerteza no nosso modelo de moviemnto
    double dt_nominal = 0.016; // Tempo nominal entre ataulizações
    double accel_noise_pos = 0.5 * qPow(dt_nominal, 2); // ruido de aceleração impactando posição
    double accel_noise_vel = dt_nominal; // Ruido de aceleração impactando velocidade

    Q(0,0) = qPow(accel_noise_pos, 2); //Q_Px
    Q(1,1) = qPow(accel_noise_pos, 2); //Q_Px
    Q(2,2) = qPow(accel_noise_vel, 2); //Q_Vx
    Q(3,3) = qPow(accel_noise_vel, 2); //Q_Vz

    //Redimensiona a matriz de covariancia do ruidmo de medição (R) para n_z por n_z
    // e inicializa (valores baseados na precisão do sensor, e.g., GPS)
    R.resize(n_z, n_z);
    R.setZero(); // Inicializa com zeros

    float gpsNoise = 2.0f; // 2 metros de desvio padrao do GPS
    R(0,0) = qPow(gpsNoise, 2); //Ruido na medição da Posição X
    R(1,1) = qPow(gpsNoise, 2); //Ruido na medição da Posição Z

    //Calcula os pesos dos sigma points com base nos parametros n_x, alpha, beta, kappa
    calculateWeights();

    m_isInitialized = true;
    m_lastMeasurementTime = QDateTime::currentDateTime();
    MY_LOG_INFO("kalman", "Filtro de kalman UKF reiniciado com sucesso");
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

    //gerar os sigmas points
    Eigen::MatrixXd X_sigma_points = generateSigmaPoints(x, P);

    //Matrix para armazenar os sigma points propagados pelo modelo de processo
    Eigen::MatrixXd X_predicted_sigma_point = Eigen::MatrixXd::Zero(n_x, 2 * n_x + 1);

    //propagar os sigma points atravez do modelo de processo
    //para cada sigm point, aplicamos o modelo de processo (processModel)
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        X_predicted_sigma_point.col(i) = processModel(X_sigma_points.col(i), dt);
    }

    //reconstruir a media e covariancia do estado predito

    //Media predita (x_pred): reconstruida a partir dos sigma points propagados e seus pesos Wm
    Eigen::VectorXd x_pred = Eigen::VectorXd::Zero(n_x);
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        x_pred += Wm(i) * X_predicted_sigma_point.col(i);
    }

    //Covariancia predita (P_pred): reconstruida a aprtir dos sigma points propagados
    Eigen::MatrixXd P_pred = Eigen::MatrixXd::Zero(n_x, n_x);
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        Eigen::VectorXd diff = X_predicted_sigma_point.col(i) - x_pred;
        P_pred += Wc(i) * diff * diff.transpose();
    }

    //Adicionar a covariancia do ruido do processo (Q) a covariancia predita
    P_pred += Q;

    //Atulaiza o estado e a covariancia internos da classe
    x = x_pred;
    P = P_pred;

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

    //Vetor de medição real (z_measured)
    Eigen::VectorXd z_measured = Eigen::VectorXd::Zero(n_z);
    z_measured(0) = measuredX;
    z_measured(1) = measuredZ;

    // Esta é a leitura de posição X/Z que vem do seu GPS.
    MY_LOG_DEBUG("Kalman_Update", QString("IN z_measured(X,Z): %1,%2").arg(measuredX, 0, 'f', 3).arg(measuredZ, 0, 'f', 3));

    //Gerar sigma points a aprtir do estado predito atual
    Eigen::MatrixXd X_sigma_points_pred = generateSigmaPoints(x, P);

    //MAtrix para armazenar os sigma points propagados pelo modelo de medição
    Eigen::MatrixXd Z_sigma_points = Eigen::MatrixXd::Zero(n_z, 2 * n_x +1);

    //propagar sigma points atravez do modelo de medição
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        Z_sigma_points.col(i) = measurementModel(X_sigma_points_pred.col(i));
    }

    //reconstruir Média da medição predita (z_pred)
    Eigen::VectorXd z_pred = Eigen::VectorXd::Zero(n_z);
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        z_pred += Wm(i) * Z_sigma_points.col(i);
    }

    //Reconstruir Covariancia da medição predita (P_zz)
    Eigen::MatrixXd P_zz = Eigen::MatrixXd::Zero(n_z, n_z);
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        Eigen::VectorXd diff_z = Z_sigma_points.col(i) - z_pred;
        P_zz += Wc(i) * diff_z * diff_z.transpose();
    }

    //Adicionar a covariancia do ruido de medição (R)
    P_zz += R;

    //calcular covariancia cruzada(P_zz)
    Eigen::MatrixXd P_xz = Eigen::MatrixXd::Zero(n_x, n_z);
    for (int i = 0; i < 2 * n_x + 1; ++i) {
        Eigen::VectorXd diff_x = X_sigma_points_pred.col(i) - x; // x é a média predita do estado
        Eigen::VectorXd diff_z = Z_sigma_points.col(i) - z_pred;
        P_xz += Wc(i) * diff_x * diff_z.transpose();
    }

    // 6. Calcular Ganho de Kalman (K)
    // K = P_xz * P_zz.inverse()
    // Usamos .llt().solve() para uma inversão mais robusta para matrizes SPD (Symmetric Positive Definite)
    Eigen::MatrixXd K;
    Eigen::LLT<Eigen::MatrixXd> lltOfP_zz(P_zz);
    if (lltOfP_zz.info() == Eigen::Success) {
        K = lltOfP_zz.solve(P_xz.transpose()).transpose(); // K = (P_zz^-1 * P_xz.transpose()).transpose() = P_xz * P_zz^-1
    } else {
        MY_LOG_ERROR("Kalman", "Falha na decomposição de Cholesky para P_zz. P_zz pode não ser SPD. Atualização ignorada.");
        return; // Não atualiza o estado se a inversão falhar
    }

    MY_LOG_DEBUG("Kalman_Update", QString("IN K(0,0)=%1 K(0,1)=%2 K(1,0)=%3 K(1,1)=%4")
                                      .arg(K(0,0), 0, 'f', 3).arg(K(0,1), 0, 'f', 3)
                                      .arg(K(1,0), 0, 'f', 3).arg(K(1,1), 0, 'f', 3));
    MY_LOG_DEBUG("Kalman_Update", QString("IN K(2,0)=%1 K(3,0)=%2 K(2,1)=%3 K(3,1)=%4")
                                      .arg(K(2,0), 0, 'f', 3).arg(K(3,0), 0, 'f', 3)
                                      .arg(K(2,1), 0, 'f', 3).arg(K(3,1), 0, 'f', 3));

    // Vetor de Inovação (y): Diferença entre medição real e medição predita
    Eigen::VectorXd y = z_measured - z_pred;

    MY_LOG_DEBUG("Kalman_Update", QString("IN y_innovation(X,Z): %1,%2")
                                      .arg(y(0), 0, 'f', 3).arg(y(1), 0, 'f', 3));

    // 7. Atualizar Estado (x) e Covariância (P)
    x = x + K * y;
    P = P - K * P_zz * K.transpose();

    MY_LOG_DEBUG("Kalman_Update", QString("OUT x_est: Px=%1 Pz=%2 Vx=%3 Vz=%4")
                                      .arg(x(0), 0, 'f', 3).arg(x(1), 0, 'f', 3)
                                      .arg(x(2), 0, 'f', 3).arg(x(3), 0, 'f', 3));
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

//modelo de processo Não Linear(f): Como o estado evolui no tempo
Eigen::VectorXd KalmanFilter::processModel(const Eigen::VectorXd& x_prev, double dt) {
    Eigen::VectorXd x_pred = x_prev; // começa com o estado anterior

    //Extrai os componentes do estado anterior para facilitar a leitura
    double px = x_prev(0);
    double pz = x_prev(1);
    double vx = x_prev(2);
    double vz = x_prev(3);

    //Predição da posição usando o modelo de velocidade constante
    double px_pred = px + vx * dt;
    double pz_pred = pz + vz * dt;

    //A velocidade é considerada constante (sem aceleração intrinseca ao modelo)
    double vx_pred = vx;
    double vz_pred = vz;

    //Atualiza o vetor de estado predito
    x_pred(0) = px_pred;
    x_pred(1) = pz_pred;
    x_pred(2) = vx_pred;
    x_pred(3) = vz_pred;

    return x_pred;
}

//Modelo de medição Nao-linear(h): como as medição sao obtidas do estado
Eigen::VectorXd KalmanFilter::measurementModel(const Eigen::VectorXd& x_state) {
    Eigen::VectorXd z_pred = Eigen::VectorXd::Zero(n_z); // Vetor de medição preditada


    //A medição (GPS) fornece diretamente a posição (Px, Pz)
    z_pred(0) = x_state(0); // Px do estado é a primeira medição
    z_pred(1) = x_state(1); // Pz do estado é a segunda medição

    return z_pred;
}

 //Geração dos Sigma Points a partir da média e covariancia
Eigen::MatrixXd KalmanFilter::generateSigmaPoints(const Eigen::VectorXd& x_mean, const Eigen::MatrixXd& P_cov) {
    //A matriz de sigma points tera n_x linhas (para cada componente do estado)
    //e (2 * n_x + 1) colunas (uma para cada sigma point)
    Eigen::MatrixXd sigma_points = Eigen::MatrixXd::Zero(n_x, 2 * n_x + 1);

    //Calcula a "raiz quadrada" da amtriz de covariancia escalonada
    //Eigen:LLT para cholesky decomposition (eficiente para matrizes simetricas positivas definidas)
    //Cuidado: (n_x + lambda) * P_cov pode nao ser exatamente simetrica positiva definida devido a pequenos erros numericos
    //E comum adicionar uma pequena pertubação a diagonal para garantir SPD.
    Eigen::MatrixXd P_scaled = P_cov * (n_x + lambda);

    P_scaled.diagonal().array() += 1e-9; //Adiciona um pequena valor a diagonal principal

    Eigen::LLT<Eigen::MatrixXd> lltOfP(P_scaled);
    Eigen::MatrixXd L; // L é a matriz triangular inferior

    if (lltOfP.info() == Eigen::Success) {
        L = lltOfP.matrixL();
    } else {
        MY_LOG_ERROR("Kalman", "Falha na decomposição de Cholesky ao gerar sigma points. Matriz P pode não ser positiva definida.");
        // Em um sistema real, você pode querer adicionar lógica para lidar com isso,
        // como reiniciar o filtro ou usar uma aproximação diferente.
        // Por enquanto, vamos retornar uma matriz de zeros para evitar falhas graves.
        return Eigen::MatrixXd::Zero(n_x, 2 * n_x + 1);
    }

    //Primeiro sigma point é a propria media
    sigma_points.col(0) = x_mean;

    //Gera os 2 * n_x sigma points restantes
    for (int i = 0; i < n_x; ++i) {
        //Sigma point i+1(x_mean + coluna i de L)
        sigma_points.col(i + 1) = x_mean + L.col(i);
        //Sigma point i+1+n_x (x_mean - coluna i de L)
        sigma_points.col(i + 1 + n_x) = x_mean - L.col(i);
    }

    return sigma_points;
}

