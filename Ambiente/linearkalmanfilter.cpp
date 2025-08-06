#include "linearkalmanfilter.h"
#include "logger.h"
#include <QtMath>

linearkalmanfilter::linearkalmanfilter() : m_isInitialized(false) {
    int n = 4;

    int m = 2;

    m_x = Eigen::VectorXd(n);
    m_P = Eigen::MatrixXd(n, n);
    m_F = Eigen::MatrixXd(n, n);
    m_H = Eigen::MatrixXd(m, n);
    m_Q = Eigen::MatrixXd(n, n);
    m_R = Eigen::MatrixXd(m, m);

    m_H << 1, 0, 0, 0,
           0, 1, 0, 0;

}


void linearkalmanfilter::setNoise(double r_measurement_uncertainty, double q_process_uncertainty) {
    m_R << r_measurement_uncertainty, 0,
        0, r_measurement_uncertainty;

    m_Q = Eigen::MatrixXd::Identity(4, 4) * q_process_uncertainty;
}

void linearkalmanfilter::reset(double initialX, double initialZ, double initialVx, double initialVz){
    m_x << initialX, initialZ, initialVx, initialVz;

    m_P = Eigen::MatrixXd::Identity(4, 4) * 1000.0;

    m_isInitialized = true;
    MY_LOG_INFO("LinearKalman", "filtro de Kalman Linear Reiniciado");
}

void linearkalmanfilter::setState(const Eigen::VectorXd& state, const Eigen::MatrixXd& covariance) {
    m_x = state;
    m_P = covariance;
}

void linearkalmanfilter::predict(double dt) {
    if (!m_isInitialized) return;

    m_F << 1, 0, dt, 0,
           0, 1, 0, dt,
           0, 0, 1, 0,
           0, 0, 0, 1;

    m_x = m_F * m_x;
    m_P = m_F * m_P * m_F.transpose() + m_Q;
}

UpdateResult linearkalmanfilter::update(const Eigen::VectorXd& z_measurement) {
    if (!m_isInitialized) return {Eigen::VectorXd(), Eigen::MatrixXd()};

    // Os cálculos são os mesmos, mas agora guardamos 'y' e 'S' para o relatório.
    Eigen::VectorXd y = z_measurement - (m_H * m_x);
    Eigen::MatrixXd S = m_H * m_P * m_H.transpose() + m_R;

    // Verifica se a matriz S é invertível antes de prosseguir
    if (S.determinant() == 0) {
        MY_LOG_ERROR("LinearKalman", "Falha na atualização: Covariância da Inovação (S) é singular.");
        return {Eigen::VectorXd(), Eigen::MatrixXd()};
    }

    Eigen::MatrixXd K = m_P * m_H.transpose() * S.inverse();

    m_x = m_x + (K * y);
    long n = m_x.size();
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(n, n);
    m_P = (I - K * m_H) * m_P;

    // Preenche e retorna o relatório com a "surpresa" e a "incerteza".
    return {y, S};
}
