#ifndef EWMAFILTER_H
#define EWMAFILTER_H

#include "FilterBase.h"

class EwmaFilter : public FilterBase
{
public:
    explicit EwmaFilter(double alpha);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    double alpha;
    double lastOutput;
    bool hasOutput;
};

#endif // EWMAFILTER_H
