#ifndef LINEARKALMANFILTER_H
#define LINEARKALMANFILTER_H

#include <Eigen>
#include <Dense>
#include <QVector2D>

struct UpdateResult{
    Eigen::VectorXd innovation;
    Eigen::MatrixXd innovation_covariance;
};

class linearkalmanfilter
{
public:
    linearkalmanfilter();

    void setNoise(double r_measurement_uncertainty, double q_process_uncertainty);

    void reset(double initialX, double initialZ, double initialVx = 0.0, double initialVz = 0.0);

    void predict(double dt);

    UpdateResult update(const Eigen::VectorXd& z_measurement);

    const Eigen::VectorXd& getState() const { return m_x; }

    const Eigen::MatrixXd& getCovariance() const { return m_P; }

    void setState(const Eigen::VectorXd& state, const Eigen::MatrixXd& covariannce);

    bool isInitialized() const { return m_isInitialized; }

private:
    Eigen::VectorXd m_x;
    Eigen::MatrixXd m_P;
    Eigen::MatrixXd m_F;
    Eigen::MatrixXd m_H;
    Eigen::MatrixXd m_Q;
    Eigen::MatrixXd m_R;

    bool m_isInitialized;
};

#endif // LINEARKALMANFILTER_H
