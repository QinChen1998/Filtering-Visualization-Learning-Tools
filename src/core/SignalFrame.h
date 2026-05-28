#ifndef SIGNALFRAME_H
#define SIGNALFRAME_H

struct SignalFrame
{
    double timeSeconds = 0.0;
    double referenceValue = 0.0;
    double noisyValue = 0.0;
    double auxiliaryValue = 0.0;
    double filteredValue = 0.0;
    bool hasAuxiliaryValue = false;
};

#endif // SIGNALFRAME_H
