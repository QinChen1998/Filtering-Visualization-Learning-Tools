#ifndef HAMPELFILTER_H
#define HAMPELFILTER_H

#include "FilterBase.h"

#include <QQueue>
#include <QVector>

class HampelFilter : public FilterBase
{
public:
    HampelFilter(int windowSize, double threshold);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    double median(QVector<double> values) const;

    int windowSize;
    double threshold;
    QQueue<double> samples;
};

#endif // HAMPELFILTER_H
