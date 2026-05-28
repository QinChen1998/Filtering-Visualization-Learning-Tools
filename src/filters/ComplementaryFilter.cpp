#include "ComplementaryFilter.h"

#include <QtGlobal>

ComplementaryFilter::ComplementaryFilter(double alpha)
    : alpha(qBound(0.001, alpha, 0.999))
    , lastOutput(0.0)
    , lastFastMeasurement(0.0)
    , hasOutput(false)
{
}

QString ComplementaryFilter::name() const
{
    return QStringLiteral("互补滤波器");
}

QString ComplementaryFilter::parameterSummary() const
{
    return QStringLiteral("互补系数 %1").arg(alpha, 0, 'f', 3);
}

void ComplementaryFilter::reset()
{
    lastOutput = 0.0;
    lastFastMeasurement = 0.0;
    hasOutput = false;
}

double ComplementaryFilter::process(double input, double dt)
{
    Q_UNUSED(dt)

    if (!hasOutput) {
        lastOutput = input;
        lastFastMeasurement = input;
        hasOutput = true;
        return lastOutput;
    }

    lastOutput = alpha * lastOutput + (1.0 - alpha) * input;
    lastFastMeasurement = input;
    return lastOutput;
}

double ComplementaryFilter::processFrame(const SignalFrame &frame, double dt)
{
    Q_UNUSED(dt)

    const double slowStableMeasurement = frame.noisyValue;
    const double fastResponsiveMeasurement = frame.hasAuxiliaryValue ? frame.auxiliaryValue : frame.noisyValue;

    if (!hasOutput) {
        lastOutput = slowStableMeasurement;
        lastFastMeasurement = fastResponsiveMeasurement;
        hasOutput = true;
        return lastOutput;
    }

    const double fastDelta = fastResponsiveMeasurement - lastFastMeasurement;
    const double prediction = lastOutput + fastDelta;
    lastOutput = alpha * prediction + (1.0 - alpha) * slowStableMeasurement;
    lastFastMeasurement = fastResponsiveMeasurement;
    return lastOutput;
}
