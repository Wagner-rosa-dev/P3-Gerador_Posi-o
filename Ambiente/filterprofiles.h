#ifndef FILTERPROFILES_H
#define FILTERPROFILES_H

#include <QMap>
#include <QString>

struct FilterProfile {
    double R_measurement_uncertainty;
    double Q_process_uncertainty;
};

static const QMap<QString, FilterProfile> PREDEFINED_PROFILES = {
{
    "Padrão",
    {0.1, 0.001} //R = 0.1, Q = 0.001
},
{
    "Veículo Lento",
    {0.5, 0.0001}
},
{
    "Veículo Àgil",
    {0.05, 0.01}
},
{
     "Parado", {0.8, 1e-9}
}
};

#endif // FILTERPROFILES_H
