#include "immfilter.h"
#include "logger.h"
#include <QtMath>

// --- Construtor ---
// Descrição: Cria as instâncias dos filtros e configura os parâmetros do MMI.
immfilter::immfilter() : m_isInitialized(false) {
    // 1. Instanciar nossos filtros especialistas
    m_fkl = new linearkalmanfilter();
    m_ukf = new KalmanFilter(); // O UKF que já existia

    // 2. Configurar a Matriz de Transição de Modo
    // Define a probabilidade de mudar de um modo para outro.
    // Linhas: modo anterior, Colunas: modo atual
    //       [ Reta -> Reta, Reta -> Curva ]
    //       [ Curva -> Reta, Curva -> Curva ]
    m_modeTransitionMatrix.resize(2, 2);
    m_modeTransitionMatrix << 0.98, 0.02,  // 98% de chance de continuar em reta, 2% de mudar para curva
        0.10, 0.90;  // 10% de chance de sair da curva para reta, 90% de continuar em curva

    // 3. Inicializar as Probabilidades de Modo
    // Começamos sem saber qual o modo correto, então damos 50% para cada.
    m_modeProbabilities.resize(2);
    m_modeProbabilities[0] = 0.5; // Probabilidade do FKL (Reta)
    m_modeProbabilities[1] = 0.5; // Probabilidade do UKF (Curva)

    // Prepara vetores para estados/covariâncias
    m_mixed_states.resize(2);
    m_mixed_covariances.resize(2);
    m_likelihoods.resize(2);

    m_x_fused.resize(4);
    m_P_fused.resize(4, 4);
}

immfilter::~immfilter() {
    delete m_fkl;
    delete m_ukf;
}

// --- reset ---
// Descrição: Reinicia o estado completo do filtro MMI.
//            Esta função simplesmente chama a rotina de inicialização para
//            redefinir os estados, covariâncias e probabilidades de modo.
void immfilter::reset(double initialX, double initialZ) {
    // A função initialize já faz tudo o que precisamos para um reset.
    // Reutilizá-la evita duplicação de código.
    initialize(initialX, initialZ);
    MY_LOG_INFO("IMMFilter", "Filtro MMI reiniciado via chamada de reset.");
}




// --- initialize ---
// Descrição: Inicializa ambos os filtros com a primeira posição.
void immfilter::initialize(double initialX, double initialZ) {
    m_fkl->reset(initialX, initialZ);
    m_ukf->reset(initialX, initialZ);

    // Pega o estado inicial de um dos filtros para o estado combinado
    m_x_fused = m_fkl->getState();
    m_P_fused = m_fkl->getCovariance();

    m_isInitialized = true;
    MY_LOG_INFO("IMMFilter", "Filtro MMI inicializado.");
}

// --- predict e update (Interface Pública) ---
// Descrição: Orquestram o ciclo do MMI.
QVector2D immfilter::predictSmoothPosition(double dt_since_last_tick) const {
    if (!m_isInitialized) {
        return QVector2D(0.0, 0.0);
    }
    // Pega a última posição e velocidade combinadas
    double px = m_x_fused(0);
    double pz = m_x_fused(1);
    double vx = m_x_fused(2);
    double vz = m_x_fused(3);

    // Retorna a posição extrapolada linearmente
    return QVector2D(px + vx * dt_since_last_tick, pz + vz * dt_since_last_tick);
}

void immfilter::updateWithMeasurement(double measuredX, double measuredZ) {
    if (!m_isInitialized) {
        initialize(measuredX, measuredZ);
        m_dtTimer.start(); // Inicia o timer na primeira medição
        return;
    }

    // Calcula o tempo real desde a última atualização do GPS
    double dt = m_dtTimer.restart() / 1000.0;
    if (dt < 0.001) dt = 0.001; // Evita dt zero ou negativo

    Eigen::VectorXd measurement(2);
    measurement << measuredX, measuredZ;

    // --- Ciclo Completo do MMI ---
    interaction();
    // Agora passamos o 'dt' correto para a etapa de filtragem
    filtering(dt, measurement);
    updateModeProbabilities(measurement);
    estimateCombination();
}

// --- interaction (Passo 1 do MMI) ---
// Descrição: Mistura os estados dos filtros. Antes de cada ciclo, cada filtro
// recebe uma "dica" do outro, ponderada pela probabilidade de transição.
void immfilter::interaction() {
    // Pega os estados e covariâncias atuais de cada filtro.
    QVector<Eigen::VectorXd> prev_states;
    QVector<Eigen::MatrixXd> prev_covariances;
    prev_states.append(m_fkl->getState());
    prev_states.append(m_ukf->getState());
    prev_covariances.append(m_fkl->getCovariance());
    prev_covariances.append(m_ukf->getCovariance());

    // 1. Calcula as probabilidades de modo previstas (antes da medição)
    Eigen::VectorXd predicted_mode_prob(2);
    predicted_mode_prob = m_modeTransitionMatrix.transpose() * m_modeProbabilities;

    // 2. Calcula as probabilidades de mixagem
    Eigen::MatrixXd mixing_prob(2, 2);
    for (int j = 0; j < 2; ++j) { // Para cada modo de destino j
        for (int i = 0; i < 2; ++i) { // Para cada modo de origem i
            mixing_prob(i, j) = (m_modeTransitionMatrix(i, j) * m_modeProbabilities[i]) / predicted_mode_prob(j);
        }
    }

    // 3. Calcula o estado e a covariância mixados para cada filtro
    for (int j = 0; j < 2; ++j) {
        m_mixed_states[j].setZero(4);
        m_mixed_covariances[j].setZero(4, 4);
        for (int i = 0; i < 2; ++i) {
            m_mixed_states[j] += mixing_prob(i, j) * prev_states[i];
        }
        for (int i = 0; i < 2; ++i) {
            Eigen::VectorXd diff = prev_states[i] - m_mixed_states[j];
            m_mixed_covariances[j] += mixing_prob(i, j) * (prev_covariances[i] + diff * diff.transpose());
        }
    }
}

// --- filtering (Passo 2 do MMI) ---
// Descrição: Roda cada filtro com suas condições iniciais mixadas.
void immfilter::filtering(double dt, const Eigen::VectorXd& measurement) {
    m_fkl->setState(m_mixed_states[0], m_mixed_covariances[0]);
    m_ukf->setState(m_mixed_states[1], m_mixed_covariances[1]);

    m_fkl->predict(dt);
    m_ukf->predict(dt);

    //captura o relatorio de cada filtro
    UpdateResult fkl_result = m_fkl->update(measurement);
    UpdateResult ukf_result = m_ukf->update(measurement(0), measurement(1));

    //calcula o likelihood(verossimilhança) a partir dos relatorios
    auto calculate_likelihood = [](const UpdateResult& result) {
        if (result.innovation.size() == 0) return 1e-9; // retorno invalido do filtro

        double k = result.innovation.rows(); //dimensao 2 para nós:x, z
        double detS = result.innovation_covariance.determinant();
        if (detS <= 0) return 1e-9; //Incerteza zero ou negativa, metamaticamente instavel

        //formula da densidade de probabilidade Gaussiana Multivariada
        double exponent = -0.5 * result.innovation.transpose() * result.innovation_covariance.inverse() * result.innovation;
        double constant = 1.0 / qSqrt(qPow(2 * M_PI, k) * detS);

        return constant * qExp(exponent);
    };

    m_likelihoods[0] = calculate_likelihood(fkl_result);
    m_likelihoods[1] = calculate_likelihood(ukf_result);

    //Adiciona um pequena valor para evitar o likelihood zero, que zeraria a probabilidade para sempre
    m_likelihoods[0] += 1e-9;
    m_likelihoods[1] += 1e-9;
}

// --- updateModeProbabilities (Passo 3 do MMI) ---
// Descrição: Atualiza a crença (probabilidade) de cada modo.
void immfilter::updateModeProbabilities(const Eigen::VectorXd& measurement) {
    double total_prob = 0;

    //Atualiza a probabilidade de cada modo multiplicando pela sua verossimilhança
    for (int i = 0; i < 2; ++i) {
        //A probabilidade prevista foi calculada na etapa de interação
        Eigen::VectorXd predicted_mode_prob = m_modeTransitionMatrix.transpose() * m_modeProbabilities;
        m_modeProbabilities[i] = m_likelihoods[i] * predicted_mode_prob(i);
        total_prob += m_modeProbabilities[i];
    }

    if (total_prob > 0) {
        for (int i = 0; i < 2; ++i) {
            m_modeProbabilities[i] /= total_prob;
        }
    } else  {
        //Se algo der muito errado, reseta para 50/50
        m_modeProbabilities[0] = 0.5;
        m_modeProbabilities[1] = 0.5;
    }
}

// --- estimateCombination (Passo 4 do MMI) ---
// Descrição: Combina os resultados dos filtros para gerar a saída final.
void immfilter::estimateCombination() {
    m_x_fused.setZero(4);
    m_P_fused.setZero(4, 4);

    // Pega os estados atualizados após a filtragem
    QVector<Eigen::VectorXd> updated_states;
    updated_states.append(m_fkl->getState());
    updated_states.append(m_ukf->getState());
    // Soma ponderada dos estados
    for (int i = 0; i < 2; ++i) {
        m_x_fused += m_modeProbabilities[i] * updated_states[i];
    }

    // Soma ponderada das covariâncias
    QVector<Eigen::MatrixXd> updated_covariances;
    updated_covariances.append(m_fkl->getCovariance());
    updated_covariances.append(m_ukf->getCovariance());

    for (int i = 0; i < 2; ++i) {
        Eigen::VectorXd diff = updated_states[i] - m_x_fused;
        m_P_fused += m_modeProbabilities[i] * (updated_covariances[i] + diff * diff.transpose());
    }
}


// --- Funções de acesso ao resultado ---
QVector2D immfilter::getStatePosition() const {
    return QVector2D(m_x_fused(0), m_x_fused(1));
}

QVector2D immfilter::getStateVelocity() const {
    return QVector2D(m_x_fused(2), m_x_fused(3));
}

void immfilter::setProfile(const FilterProfile& profile) {
    if (m_fkl) {
        // O Filtro de Kalman Linear espera os valores de ruído diretamente.
        m_fkl->setNoise(profile.R_measurement_uncertainty, profile.Q_process_uncertainty);
    }
    if (m_ukf) {
        // O UKF já tem um método que aceita a struct FilterProfile.
        m_ukf->setProfile(profile);
    }
    MY_LOG_INFO("IMMFilter", "Perfis de ruído atualizados nos filtros internos.");

}


