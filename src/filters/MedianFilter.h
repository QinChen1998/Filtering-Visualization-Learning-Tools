#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include "FilterBase.h"

#include <QQueue>
#include <QVector>

class MedianFilter : public FilterBase
{
public:
    explicit MedianFilter(int windowSize);

    QString name() const override;
    QString parameterSummary() const override;
    void reset() override;
    double process(double input, double dt) override;

private:
    double median(QVector<double> values) const;

    int windowSize;
    QQueue<double> samples;
};

#endif // MEDIANFILTER_H
