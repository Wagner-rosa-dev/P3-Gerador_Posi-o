#ifndef IMMFILTER_H
#define IMMFILTER_H

#include "linearkalmanfilter.h"
#include "kalmanfilter.h"
#include <QVector>
#include <Eigen>
#include <Dense>
#include <QElapsedTimer>

class immfilter
{
public:
    immfilter();
    ~immfilter();

    void initialize(double initialX, double initialZ);

    void reset(double initialX, double initialZ);

    void setProfile(const FilterProfile& profile);


    QVector2D getStatePosition() const;
    QVector2D getStateVelocity() const;

    const Eigen::VectorXd& getModeProbabilities() const { return m_modeProbabilities; }

    bool isInitialized() const { return m_isInitialized; }

    QVector2D predictSmoothPosition(double dt_since_last_tick) const;

    void updateWithMeasurement(double measuredX, double measuredZ);



private:
    void interaction();
    void filtering(double dt, const Eigen::VectorXd& measurement);
    void updateModeProbabilities(const Eigen::VectorXd& measurement);
    void estimateCombination();

    linearkalmanfilter* m_fkl;
    KalmanFilter* m_ukf;
    QVector<linearkalmanfilter*> m_filters;

    Eigen::VectorXd m_x_fused;
    Eigen::MatrixXd m_P_fused;

    Eigen::VectorXd m_modeProbabilities;
    Eigen::MatrixXd m_modeTransitionMatrix;

    QVector<double> m_likelihoods;

    QVector<Eigen::VectorXd> m_mixed_states;
    QVector<Eigen::MatrixXd> m_mixed_covariances;

    bool m_isInitialized;

    QElapsedTimer m_dtTimer;
};

#endif // IMMFILTER_H
