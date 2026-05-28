#ifndef FILTERBASE_H
#define FILTERBASE_H

#include <QString>

class FilterBase
{
public:
    virtual ~FilterBase() = default;

    virtual QString name() const = 0;
    virtual QString parameterSummary() const = 0;
    virtual void reset() = 0;
    virtual double process(double input, double dt) = 0;
};

#endif // FILTERBASE_H
