#ifndef NOTCHFILTER_H
#define NOTCHFILTER_H

#include "FilterBase.h"

class NotchFilter : public FilterBase
{
public:
    NotchFilter(double centerFrequencyHz, double bandwidthHz);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    void updateCoefficients(double sampleRateHz);

    double centerFrequencyHz;
    double bandwidthHz;
    double b0;
    double b1;
    double b2;
    double a1;
    double a2;
    double x1;
    double x2;
    double y1;
    double y2;
    double configuredSampleRateHz;
};

#endif // NOTCHFILTER_H
