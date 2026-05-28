#include "SignalGenerator.h"

#include <QRandomGenerator>
#include <QtMath>

#include <cmath>

namespace {
constexpr double TwoPi = 6.28318530717958647692;
}

SignalGenerator::SignalGenerator()
    : samplesPerSecond(100.0)
    , currentTime(0.0)
    , previewFilteredValue(0.0)
    , hasPreviewFilteredValue(false)
    , currentSignalMode(SignalMode::Sine)
    , currentNoiseMode(NoiseMode::Gaussian)
{
}

void SignalGenerator::setSampleRate(double samplesPerSecond)
{
    this->samplesPerSecond = qBound(1.0, samplesPerSecond, 1000.0);
}

void SignalGenerator::setSignalMode(SignalMode mode)
{
    currentSignalMode = mode;
    reset();
}

void SignalGenerator::setNoiseMode(NoiseMode mode)
{
    currentNoiseMode = mode;
    reset();
}

void SignalGenerator::reset()
{
    currentTime = 0.0;
    previewFilteredValue = 0.0;
    hasPreviewFilteredValue = false;
}

double SignalGenerator::sampleRate() const
{
    return samplesPerSecond;
}

SignalGenerator::SignalMode SignalGenerator::signalMode() const
{
    return currentSignalMode;
}

SignalGenerator::NoiseMode SignalGenerator::noiseMode() const
{
    return currentNoiseMode;
}

SignalFrame SignalGenerator::nextFrame()
{
    const double reference = referenceSignal(currentTime);
    const double noisy = reference + noiseSignal(currentTime);
    const double alpha = 0.12;
    if (!hasPreviewFilteredValue) {
        previewFilteredValue = noisy;
        hasPreviewFilteredValue = true;
    } else {
        previewFilteredValue = alpha * noisy + (1.0 - alpha) * previewFilteredValue;
    }

    SignalFrame frame;
    frame.timeSeconds = currentTime;
    frame.referenceValue = reference;
    frame.noisyValue = noisy;
    frame.filteredValue = previewFilteredValue;

    currentTime += 1.0 / samplesPerSecond;
    return frame;
}

double SignalGenerator::referenceSignal(double timeSeconds) const
{
    switch (currentSignalMode) {
    case SignalMode::Sine:
        return qSin(TwoPi * 0.7 * timeSeconds);
    case SignalMode::Step:
        return std::floor(std::fmod(timeSeconds, 6.0) / 3.0) > 0.0 ? 1.0 : -0.35;
    case SignalMode::Ramp:
        return std::fmod(timeSeconds, 5.0) / 2.5 - 1.0;
    case SignalMode::Pulse:
        return std::fmod(timeSeconds, 2.0) < 0.18 ? 1.15 : -0.15;
    case SignalMode::Mixed:
        return 0.75 * qSin(TwoPi * 0.35 * timeSeconds)
               + 0.25 * qSin(TwoPi * 1.8 * timeSeconds)
               + (std::fmod(timeSeconds, 8.0) > 4.0 ? 0.35 : -0.15);
    }

    return 0.0;
}

double SignalGenerator::noiseSignal(double timeSeconds) const
{
    switch (currentNoiseMode) {
    case NoiseMode::None:
        return 0.0;
    case NoiseMode::Gaussian:
        return gaussianNoise() * 0.16;
    case NoiseMode::Spike:
        return spikeNoise();
    case NoiseMode::Periodic:
        return 0.20 * qSin(TwoPi * 8.0 * timeSeconds);
    case NoiseMode::Mixed:
        return gaussianNoise() * 0.10
               + spikeNoise()
               + 0.14 * qSin(TwoPi * 8.0 * timeSeconds);
    }

    return 0.0;
}

double SignalGenerator::gaussianNoise() const
{
    const double u1 = qMax(QRandomGenerator::global()->generateDouble(), 1e-9);
    const double u2 = QRandomGenerator::global()->generateDouble();
    return qSqrt(-2.0 * qLn(u1)) * qCos(TwoPi * u2);
}

double SignalGenerator::spikeNoise() const
{
    if (QRandomGenerator::global()->generateDouble() > 0.018) {
        return 0.0;
    }

    const double sign = QRandomGenerator::global()->generateDouble() < 0.5 ? -1.0 : 1.0;
    return sign * (0.75 + QRandomGenerator::global()->generateDouble() * 0.75);
}
