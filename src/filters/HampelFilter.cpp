#include "HampelFilter.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

HampelFilter::HampelFilter(int windowSize, double threshold)
    : windowSize(qBound(3, windowSize | 1, 501))
    , threshold(qBound(0.5, threshold, 10.0))
{
}

QString HampelFilter::name() const
{
    return QStringLiteral("Hampel 异常值滤波器");
}

QString HampelFilter::parameterSummary() const
{
    return QStringLiteral("窗口大小 %1，阈值 %2 MAD").arg(windowSize).arg(threshold, 0, 'f', 1);
}

void HampelFilter::reset()
{
    samples.clear();
}

double HampelFilter::process(double input, double dt)
{
    Q_UNUSED(dt)

    samples.enqueue(input);
    while (samples.size() > windowSize) {
        samples.dequeue();
    }

    QVector<double> values(samples.begin(), samples.end());
    const double center = median(values);
    QVector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) {
        deviations.append(std::abs(value - center));
    }

    const double mad = median(deviations);
    if (mad < 1e-9) {
        return input;
    }

    const double robustSigma = 1.4826 * mad;
    if (std::abs(input - center) > threshold * robustSigma) {
        return center;
    }

    return input;
}

double HampelFilter::median(QVector<double> values) const
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
