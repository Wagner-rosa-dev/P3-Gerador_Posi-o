#include "kalmanfilter.h"
#include <QDebug> // para mensagens de depuração
#include <QtMath> // Para qPow

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
        qWarning() << "KalmanFilter not initialized. Call reset() first";
        return;
    }

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
}

//FAse de atualização
void KalmanFilter::update(double measuredX, double measuredZ) {
    if (!m_isInitialized) {
        qWarning() << "KalmanFilter not initialized. Call reset() first.";
        return;
    }

    // Vetor de Medição (z): [Px_medido, Pz_medido]
    QVector2D z(measuredX, measuredZ);

    //Inovação (y = z - H * x)
    // A medição prevista (H*x) deve ser um QVector2D para compara com z.
    //Vamos extrair as partes relevantes de H*x
    QVector2D predictedMeasurement = QVector2D(x[0], x[1]); //Px, Pz do estado previsto
    QVector2D y = z - predictedMeasurement; // Vetor de inovação (diferença entre medição e previsão)

    //Inovação de Covariancia (S = H * P * H^T + R)
    //H é 2x4, P é 4x4, H^T é 4x2. (H * P * H^T) = (2x4 * 4x4 * 4x2) = 2x2
    //R é 2x2. S será 2x2
    QMatrix4x4 H_transposed = H.transposed();
    QMatrix4x4 S_4x4 = H * P * H_transposed + R; // Aqui S_4x4. mas só as 2x2 são válidas
    //Precisamos de uma matrix 2x2 para S para operações subsequentes
    // Vamos construir S_2x2 manualmente a partir de S_4x4 ou refatorar H e R para serem 2x4 e 2x2

    //Refatorando: Para ter certeza de que as operações de matrizes estão corretas em termos de dimensões,
    //vamos garantir que H e R sejam dimensionadas corretamente para as medições 2D
    //Para isso, a classe QMatrix4x4 não é ideal para operações com matrizes de dimensões mistas como 2x4
    //Teriamos que simular as operações de matriz manualmente ou usar uma biblioteca linear-algebra mais flexivel
    //No entendo, para manter a simplicidade e o uso do QtMatrix4x4, vamos manter H e R como 4x4
    //e extrair as submatrizes de forma "implicita" para os calculos que exigem 2x2

    //Vamos assumir que H e R sao 4x4, mas as medições só preenchem as primeiras 2 componenetes
    //Então, ao calcular S, apenas a submatriz 2x2 superior-esquerda de (H * P * H^T) é relevante
    //S_2x2 = (H_2x4 * P_4x4 * H_4x2^T) + R_2x2
    //É uma limitação do QMatrix4x4 para esta aplicação
    //Uma alternatica é uusar:
    //S = H * P * H_transposed
    //E então R

    //Cálculo do ganho de Kalman K: K = P * H^T * inv(S)
    //S_inv = S.inverted(); (precisaria ser 2x2)

    //Dada a limitação do QMatrix4x4 para submatrizes, a forma mais robusta e didática
    //para um filtro de Kalman 2D (posição) é representar H como 2x4 e R como 2x2
    //e, se necessario, implementar multiplicação de matrizes para essas dimensões
    //Porém, o objetivo é usar QMatrix4x4
    //Vamos Simplificar o 'H' para ser 'H = [1 0 0 0; 0 1 0 0; 0 0 0 0; 0 0 0 0]'
    //e o 'R' para ser 'R = [Rx 0 0 0; 0 Rz 0 0; 0 0 0 0; 0 0 0 0]'
    //E entao 'S = H * P * H^T + R' sera 4x4
    //os elementos S(0,0) e S(1,1) serão os mais relevantes

    //Calculando o ganho de Kalman K = P * H^T * S_inv
    //S = H * P * H^T + R;
    //O problema é que z é 2D, e H*x resulta em 2D. Mas P é 4x4
    //K deve ser 4x2. H é 2x4. H_transposed é 4x2.
    //P*H_transposed = 4x4 * 4x2 = 4x2
    //S = H * (P * H_transposed) + R. (2x4 * 4x2) + 2x2 = 2x2 + 2x2 = 2x2

    //Vamos reajustar a estrutura da atualização para usar submatrizes ou uma biblioteca que lide com isso
    //Para nao complicar com bibliotecas externas ou manipulação manual de arrays para simular matrizes de tamanhas diferentes
    //e mantendo QMatrix4x4 (que é 4x4), a abordagem amis comum é estender as medições para 4D com zeros para velocidade
    //Ou sejam se o GPS mede [Px, Pz], a medição 'z' se torna [Px, Pz, 0, 0]
    //E R é 4x4, onde as variancias da velocidade medidas são muito altas (ou os elementos são zero se não medidos)

    //Vamos reformular a Matriz H no construtor para ser 4x4
    //H(0,0)=1, H(1,1)=1, e o resto zero
    //assim, H*x será um QVector4D onde as duas primeiras componenetes são Px, Pz
    //E z (medição) tambem precisa ser um QVector4D
    //'z = QVector4D(measuredX, measuredZ, 0.0, 0.0);'
    //'y = z - H * x;'
    //Essa é uma forma de usar QMatrix4x4

    //Refatorando o construtor do KalmanFilter com a matriz H e R 4x4
    //H = [1 0 0 0]
    //    [0 1 0 0]
    //    [0 0 0 0]
    //    [0 0 0 0]
    //R = [Rx 0 0 0]
    //    [0 Rz 0 0]
    //    [0 0 Rv_x Rv_x_z_noise] <-- ruido para velocidade que nao é medida diretamente
    //    [0 0 Rv_z_x_noise Rv_z]

    //com as modificações acima no construtor, podemos prosseguir com o update()
    //Medição expandida para 4D
    QVector4D z_expanded(measuredX, measuredZ, 0.0, 0.0); // velocidade medidas são 0 para simplificar

    //Inovaçõa (y = z_expanded - H * x)
    QVector4D y_ = z_expanded - (H * x);

    //Inovção Covariancia(S = H * P * H^T + R)
    QMatrix4x4 S = H * P * H.transposed() + R;

    // Ganho de kalman (K = P * H^T * inv(S))
    QMatrix4x4 S_inverted = S.inverted();
    if (S_inverted.isIdentity()) { //Verificação simples para singularidade
        qWarning() << "Inovação de covariancia (S) é singular. Não é possivel calcular inverso";
        return;
    }
    QMatrix4x4 K = P * H.transposed() * S_inverted;

    //Atualização do estado (x = x + K * y)
    x = x + (K * y_);

    //Atualiza a Covariancia(P = (I - K * H) * P)
    QMatrix4x4 I;
    I.setToIdentity(); //Matriz identidade 4x4
    P = (I - K * H) * P;
}

//Retorna a posição (X, Z) estimada
QVector2D KalmanFilter::getStatePosition() const {
    return QVector2D(x[0], x[1]);
}

//Retorna a velocidade (Vx, Vz) estimada
QVector2D KalmanFilter::getStateVelocity() const {
    return QVector2D(x[2], x[3]);
}
