#ifndef MOVINGAVERAGEFILTER_H
#define MOVINGAVERAGEFILTER_H

#include "FilterBase.h"

#include <QQueue>

class MovingAverageFilter : public FilterBase
{
public:
    explicit MovingAverageFilter(int windowSize);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    int windowSize;
    QQueue<double> samples;
    double sum;
};

#endif // MOVINGAVERAGEFILTER_H
