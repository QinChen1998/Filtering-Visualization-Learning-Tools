#include "EwmaFilter.h"

#include <QtGlobal>

EwmaFilter::EwmaFilter(double alpha)
    : alpha(qBound(0.001, alpha, 1.0))
    , lastOutput(0.0)
    , hasOutput(false)
{
}

QString EwmaFilter::name() const
{
    return QStringLiteral("EWMA 滤波器");
}

QString EwmaFilter::parameterSummary() const
{
    return QStringLiteral("alpha %1").arg(alpha, 0, 'f', 3);
}

void EwmaFilter::reset()
{
    lastOutput = 0.0;
    hasOutput = false;
}

double EwmaFilter::process(double input, double dt)
{
    Q_UNUSED(dt)

    if (!hasOutput) {
        lastOutput = input;
        hasOutput = true;
        return lastOutput;
    }

    lastOutput = alpha * input + (1.0 - alpha) * lastOutput;
    return lastOutput;
}
