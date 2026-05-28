#ifndef LOWPASSFILTER_H
#define LOWPASSFILTER_H

#include "FilterBase.h"

class LowPassFilter : public FilterBase
{
public:
    explicit LowPassFilter(double cutoffHz);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    double cutoffHz;
    double lastOutput;
    bool hasOutput;
};

#endif // LOWPASSFILTER_H
