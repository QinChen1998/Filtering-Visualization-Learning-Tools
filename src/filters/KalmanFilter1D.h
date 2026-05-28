#ifndef KALMANFILTER1D_H
#define KALMANFILTER1D_H

#include "FilterBase.h"

class KalmanFilter1D : public FilterBase
{
public:
    KalmanFilter1D(double processNoise, double measurementNoise);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;
    double processFrame(const SignalFrame &frame, double dt) override;

private:
    double processMeasurement(double measurement);

    double processNoise;
    double measurementNoise;
    double estimate;
    double covariance;
    bool hasEstimate;
};

#endif // KALMANFILTER1D_H
