#ifndef BANDPASSFILTER_H
#define BANDPASSFILTER_H

#include "FilterBase.h"

class BandPassFilter : public FilterBase
{
public:
    BandPassFilter(double lowCutoffHz, double highCutoffHz);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    void updateCoefficients(double sampleRateHz);

    double lowCutoffHz;
    double highCutoffHz;
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

#endif // BANDPASSFILTER_H
