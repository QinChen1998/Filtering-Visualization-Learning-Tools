#ifndef COMPLEMENTARYFILTER_H
#define COMPLEMENTARYFILTER_H

#include "FilterBase.h"

class ComplementaryFilter : public FilterBase
{
public:
    explicit ComplementaryFilter(double alpha);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;
    double processFrame(const SignalFrame &frame, double dt) override;

private:
    double alpha;
    double lastOutput;
    double lastFastMeasurement;
    bool hasOutput;
};

#endif // COMPLEMENTARYFILTER_H
