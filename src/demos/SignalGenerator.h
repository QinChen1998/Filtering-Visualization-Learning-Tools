#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include "SignalFrame.h"

class SignalGenerator
{
public:
    enum class SignalMode {
        Sine,
        Step,
        Ramp,
        Pulse,
        Mixed
    };

    enum class NoiseMode {
        None,
        Gaussian,
        Spike,
        Periodic,
        Mixed
    };

    SignalGenerator();

    void setSampleRate(double samplesPerSecond);
    void setSignalMode(SignalMode mode);
    void setNoiseMode(NoiseMode mode);
    void reset();

    double sampleRate() const;
    SignalMode signalMode() const;
    NoiseMode noiseMode() const;
    SignalFrame nextFrame();

private:
    double referenceSignal(double timeSeconds) const;
    double noiseSignal(double timeSeconds) const;
    double gaussianNoise() const;
    double spikeNoise() const;

    double samplesPerSecond;
    double currentTime;
    double previewFilteredValue;
    bool hasPreviewFilteredValue;
    SignalMode currentSignalMode;
    NoiseMode currentNoiseMode;
};

#endif // SIGNALGENERATOR_H
