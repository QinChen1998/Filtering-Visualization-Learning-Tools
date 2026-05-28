#ifndef FILTERBASE_H
#define FILTERBASE_H

#include "SignalFrame.h"

#include <QString>

class FilterBase
{
public:
    virtual ~FilterBase() = default;

    virtual QString name() const = 0;
    virtual QString parameterSummary() const = 0;
    virtual void reset() = 0;
    virtual double process(double input, double dt) = 0;
    virtual double processFrame(const SignalFrame &frame, double dt)
    {
        return process(frame.noisyValue, dt);
    }
};

#endif // FILTERBASE_H
