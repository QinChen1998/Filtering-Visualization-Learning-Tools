#ifndef SIGNALCHARTWIDGET_H
#define SIGNALCHARTWIDGET_H

#include "SignalFrame.h"

#include <QVector>
#include <QWidget>

class SignalChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SignalChartWidget(QWidget *parent = nullptr);

    void appendFrame(const SignalFrame &frame);
    void clear();
    void setMaxSamples(int samples);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPointF mapPoint(const QRectF &plotRect, double timeSeconds, double value, double minTime, double maxTime, double minValue, double maxValue) const;
    void drawSeries(QPainter &painter, const QRectF &plotRect, double minTime, double maxTime, double minValue, double maxValue, int seriesIndex, const QColor &color) const;
    double valueAt(const SignalFrame &frame, int seriesIndex) const;

    QVector<SignalFrame> frames;
    int maxSampleCount;
};

#endif // SIGNALCHARTWIDGET_H
