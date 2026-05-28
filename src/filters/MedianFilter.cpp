#include "MedianFilter.h"

#include <QtGlobal>

#include <algorithm>

MedianFilter::MedianFilter(int windowSize)
    : windowSize(qBound(1, windowSize | 1, 501))
{
}

QString MedianFilter::name() const
{
    return QStringLiteral("中值滤波器");
}

QString MedianFilter::parameterSummary() const
{
    return QStringLiteral("窗口大小 %1").arg(windowSize);
}

void MedianFilter::reset()
{
    samples.clear();
}

double MedianFilter::process(double input, double dt)
{
    Q_UNUSED(dt)

    samples.enqueue(input);
    while (samples.size() > windowSize) {
        samples.dequeue();
    }

    return median(QVector<double>(samples.begin(), samples.end()));
}

double MedianFilter::median(QVector<double> values) const
{
    if (values.isEmpty()) {
        return 0.0;
    }

    std::sort(values.begin(), values.end());
    const int middle = values.size() / 2;
    if (values.size() % 2 == 1) {
        return values.at(middle);
    }

    return (values.at(middle - 1) + values.at(middle)) * 0.5;
}
