#include "LowPassFilter.h"

#include <QtGlobal>

#include <cmath>

namespace {
constexpr double TwoPi = 6.28318530717958647692;
}

LowPassFilter::LowPassFilter(double cutoffHz)
    : cutoffHz(qBound(0.01, cutoffHz, 1000.0))
    , lastOutput(0.0)
    , hasOutput(false)
{
}

QString LowPassFilter::name() const
{
    return QStringLiteral("一阶低通滤波器");
}

QString LowPassFilter::parameterSummary() const
{
    return QStringLiteral("截止频率 %1 Hz").arg(cutoffHz, 0, 'f', 2);
}

void LowPassFilter::reset()
{
    lastOutput = 0.0;
    hasOutput = false;
}

double LowPassFilter::process(double input, double dt)
{
    if (!hasOutput) {
        lastOutput = input;
        hasOutput = true;
        return lastOutput;
    }

    const double rc = 1.0 / (TwoPi * cutoffHz);
    const double alpha = dt / (rc + dt);
    lastOutput += alpha * (input - lastOutput);
    return lastOutput;
}
