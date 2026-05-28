#include "MovingAverageFilter.h"

#include <QtGlobal>

MovingAverageFilter::MovingAverageFilter(int windowSize)
    : windowSize(qBound(1, windowSize, 501))
    , sum(0.0)
{
}

QString MovingAverageFilter::name() const
{
    return QStringLiteral("滑动平均滤波器");
}

QString MovingAverageFilter::parameterSummary() const
{
    return QStringLiteral("窗口大小 %1").arg(windowSize);
}

void MovingAverageFilter::reset()
{
    samples.clear();
    sum = 0.0;
}

double MovingAverageFilter::process(double input, double dt)
{
    Q_UNUSED(dt)

    samples.enqueue(input);
    sum += input;
    while (samples.size() > windowSize) {
        sum -= samples.dequeue();
    }

    return sum / samples.size();
}
