#include "KalmanFilter1D.h"

#include <QtGlobal>

KalmanFilter1D::KalmanFilter1D(double processNoise, double measurementNoise)
    : processNoise(qBound(0.0001, processNoise, 10.0))
    , measurementNoise(qBound(0.0001, measurementNoise, 10.0))
    , estimate(0.0)
    , covariance(1.0)
    , hasEstimate(false)
{
}

QString KalmanFilter1D::name() const
{
    return QStringLiteral("一维卡尔曼滤波器");
}

QString KalmanFilter1D::parameterSummary() const
{
    return QStringLiteral("Q %1，R %2").arg(processNoise, 0, 'f', 4).arg(measurementNoise, 0, 'f', 4);
}

void KalmanFilter1D::reset()
{
    estimate = 0.0;
    covariance = 1.0;
    hasEstimate = false;
}

double KalmanFilter1D::process(double input, double dt)
{
    Q_UNUSED(dt)
    return processMeasurement(input);
}

double KalmanFilter1D::processFrame(const SignalFrame &frame, double dt)
{
    Q_UNUSED(dt)

    const double measurement = frame.hasAuxiliaryValue
        ? 0.55 * frame.noisyValue + 0.45 * frame.auxiliaryValue
        : frame.noisyValue;
    return processMeasurement(measurement);
}

double KalmanFilter1D::processMeasurement(double measurement)
{
    if (!hasEstimate) {
        estimate = measurement;
        hasEstimate = true;
        return estimate;
    }

    covariance += processNoise;
    const double kalmanGain = covariance / (covariance + measurementNoise);
    estimate += kalmanGain * (measurement - estimate);
    covariance = (1.0 - kalmanGain) * covariance;
    return estimate;
}
