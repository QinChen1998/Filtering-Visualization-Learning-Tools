#ifndef FORCETORQUEWIDGET_H
#define FORCETORQUEWIDGET_H

#include <QVector>
#include <QWidget>

#include <memory>

class EwmaFilter;
class HampelFilter;
class MedianFilter;
class MovingAverageFilter;
class QPainter;

class ForceTorqueWidget : public QWidget
{
public:
    explicit ForceTorqueWidget(QWidget *parent = nullptr);
    ~ForceTorqueWidget() override;

    void appendSample(double timeSeconds, double trueForce, double measuredForce, double dt);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct ForceSample {
        double timeSeconds = 0.0;
        double trueForce = 0.0;
        double measuredForce = 0.0;
        double movingAverage = 0.0;
        double ewma = 0.0;
        double median = 0.0;
        double hampel = 0.0;
    };

    QPointF mapPoint(const QRectF &plotRect, double timeSeconds, double value, double minTime, double maxTime, double minValue, double maxValue) const;
    void drawSeries(QPainter &painter, const QRectF &plotRect, int seriesIndex, const QColor &color, double minTime, double maxTime, double minValue, double maxValue) const;
    double valueAt(const ForceSample &sample, int seriesIndex) const;

    QVector<ForceSample> samples;
    std::unique_ptr<MovingAverageFilter> movingAverageFilter;
    std::unique_ptr<EwmaFilter> ewmaFilter;
    std::unique_ptr<MedianFilter> medianFilter;
    std::unique_ptr<HampelFilter> hampelFilter;
    int maxSampleCount;
};

#endif // FORCETORQUEWIDGET_H
