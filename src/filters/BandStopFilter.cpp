#include "BandStopFilter.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace {
constexpr double TwoPi = 6.28318530717958647692;
}

BandStopFilter::BandStopFilter(double lowCutoffHz, double highCutoffHz)
    : lowCutoffHz(qBound(0.1, lowCutoffHz, 1000.0))
    , highCutoffHz(qBound(this->lowCutoffHz + 0.1, highCutoffHz, 2000.0))
    , b0(1.0)
    , b1(0.0)
    , b2(0.0)
    , a1(0.0)
    , a2(0.0)
    , x1(0.0)
    , x2(0.0)
    , y1(0.0)
    , y2(0.0)
    , configuredSampleRateHz(0.0)
{
}

QString BandStopFilter::name() const
{
    return QStringLiteral("带阻滤波器");
}

QString BandStopFilter::parameterSummary() const
{
    return QStringLiteral("阻带 %1 - %2 Hz").arg(lowCutoffHz, 0, 'f', 2).arg(highCutoffHz, 0, 'f', 2);
}

void BandStopFilter::reset()
{
    x1 = 0.0;
    x2 = 0.0;
    y1 = 0.0;
    y2 = 0.0;
}

double BandStopFilter::process(double input, double dt)
{
    if (dt <= 0.0) {
        return input;
    }

    const double sampleRateHz = 1.0 / dt;
    if (!qFuzzyCompare(sampleRateHz, configuredSampleRateHz)) {
        updateCoefficients(sampleRateHz);
    }

    const double output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = std::isfinite(output) ? output : input;
    return y1;
}

void BandStopFilter::updateCoefficients(double sampleRateHz)
{
    configuredSampleRateHz = sampleRateHz;
    const double nyquist = sampleRateHz * 0.5;
    const double low = qBound(0.1, lowCutoffHz, nyquist * 0.90);
    const double high = qBound(low + 0.1, highCutoffHz, nyquist * 0.95);
    const double center = std::sqrt(low * high);
    const double bandwidth = std::max(0.1, high - low);
    const double quality = qMax(0.1, center / bandwidth);
    const double omega = TwoPi * center / sampleRateHz;
    const double alpha = std::sin(omega) / (2.0 * quality);
    const double cosOmega = std::cos(omega);
    const double a0 = 1.0 + alpha;

    b0 = 1.0 / a0;
    b1 = -2.0 * cosOmega / a0;
    b2 = 1.0 / a0;
    a1 = -2.0 * cosOmega / a0;
    a2 = (1.0 - alpha) / a0;
}
