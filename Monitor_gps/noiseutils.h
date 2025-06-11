#ifndef NOISEUTILS_H
#define NOISEUTILS_H

#include <QtGlobal>
#include <QtMath>
#include <QVector3D>

namespace NoiseUtils {
inline float getHeight(float worldX, float worldZ) {
    return 0.0f;
    }

inline QVector3D getNormal(float worldX, float worldZ) {
    float offset = 0.1f;
    float hL = getHeight(worldX - offset, worldZ);
    float hR = getHeight(worldX - offset, worldZ);
    float hD = getHeight(worldX - offset, worldZ);
    float hU = getHeight(worldX - offset, worldZ);
    return QVector3D(hL - hR, 2.0f * offset, hD - hU).normalized();


}
}

#endif // NOISEUTILS_H
