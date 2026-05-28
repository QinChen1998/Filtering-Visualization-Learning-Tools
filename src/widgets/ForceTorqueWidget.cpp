#include "ForceTorqueWidget.h"

#include "EwmaFilter.h"
#include "HampelFilter.h"
#include "MedianFilter.h"
#include "MovingAverageFilter.h"

#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>
#include <QStringList>

#include <algorithm>

ForceTorqueWidget::ForceTorqueWidget(QWidget *parent)
    : QWidget(parent)
    , movingAverageFilter(std::make_unique<MovingAverageFilter>(11))
    , ewmaFilter(std::make_unique<EwmaFilter>(0.18))
    , medianFilter(std::make_unique<MedianFilter>(9))
    , hampelFilter(std::make_unique<HampelFilter>(9, 3.0))
    , maxSampleCount(420)
{
    setMinimumSize(420, 190);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ForceTorqueWidget::~ForceTorqueWidget() = default;

void ForceTorqueWidget::appendSample(double timeSeconds, double trueForce, double measuredForce, double dt)
{
    ForceSample sample;
    sample.timeSeconds = timeSeconds;
    sample.trueForce = trueForce;
    sample.measuredForce = measuredForce;
    sample.movingAverage = movingAverageFilter->process(measuredForce, dt);
    sample.ewma = ewmaFilter->process(measuredForce, dt);
    sample.median = medianFilter->process(measuredForce, dt);
    sample.hampel = hampelFilter->process(measuredForce, dt);

    samples.append(sample);
    while (samples.size() > maxSampleCount) {
        samples.removeFirst();
    }
    update();
}

void ForceTorqueWidget::clear()
{
    samples.clear();
    movingAverageFilter->reset();
    ewmaFilter->reset();
    medianFilter->reset();
    hampelFilter->reset();
    update();
}

void ForceTorqueWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(251, 252, 250));

    const QRectF bounds = rect().adjusted(12, 10, -12, -10);
    const QRectF plotRect = bounds.adjusted(58, 26, -18, -34);
    painter.setPen(QColor(32, 41, 57));
    painter.drawText(bounds.topLeft() + QPointF(0, 14), QStringLiteral("力/力矩传感器滤波对比"));

    if (samples.isEmpty()) {
        painter.setPen(QColor(91, 104, 124));
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("播放后显示接触突变、短时冲击和随机尖峰"));
        return;
    }

    double minTime = samples.first().timeSeconds;
    double maxTime = samples.last().timeSeconds;
    if (qFuzzyCompare(minTime, maxTime)) {
        maxTime = minTime + 1.0;
    }

    double minValue = samples.first().measuredForce;
    double maxValue = samples.first().measuredForce;
    for (const ForceSample &sample : samples) {
        minValue = std::min({minValue, sample.trueForce, sample.measuredForce, sample.movingAverage, sample.ewma, sample.median, sample.hampel});
        maxValue = std::max({maxValue, sample.trueForce, sample.measuredForce, sample.movingAverage, sample.ewma, sample.median, sample.hampel});
    }
    if (qFuzzyCompare(minValue, maxValue)) {
        minValue -= 1.0;
        maxValue += 1.0;
    }
    const double padding = (maxValue - minValue) * 0.12;
    minValue -= padding;
    maxValue += padding;

    painter.setPen(QPen(QColor(218, 226, 235), 1));
    for (int i = 0; i <= 4; ++i) {
        const double x = plotRect.left() + plotRect.width() * i / 4.0;
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
    }
    for (int i = 0; i <= 3; ++i) {
        const double y = plotRect.top() + plotRect.height() * i / 3.0;
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
    }
    painter.setPen(QPen(QColor(79, 93, 115), 1));
    painter.drawRect(plotRect);

    const QList<QColor> colors = {
        QColor(35, 88, 158),
        QColor(205, 74, 65),
        QColor(42, 142, 83),
        QColor(143, 101, 42),
        QColor(128, 84, 166),
        QColor(43, 149, 157)
    };
    for (int i = 0; i < colors.size(); ++i) {
        drawSeries(painter, plotRect, i, colors.at(i), minTime, maxTime, minValue, maxValue);
    }

    const QStringList labels = {
        QStringLiteral("真实力"),
        QStringLiteral("测量"),
        QStringLiteral("均值"),
        QStringLiteral("EWMA"),
        QStringLiteral("中值"),
        QStringLiteral("Hampel")
    };
    int legendX = static_cast<int>(bounds.right()) - 372;
    const int legendY = static_cast<int>(bounds.top() + 2);
    for (int i = 0; i < labels.size(); ++i) {
        painter.setPen(QPen(colors.at(i), 3));
        painter.drawLine(QPointF(legendX, legendY + 8), QPointF(legendX + 18, legendY + 8));
        painter.setPen(QColor(32, 41, 57));
        painter.drawText(QPointF(legendX + 22, legendY + 12), labels.at(i));
        legendX += 62;
    }
}

QPointF ForceTorqueWidget::mapPoint(const QRectF &plotRect, double timeSeconds, double value, double minTime, double maxTime, double minValue, double maxValue) const
{
    const double xRatio = (timeSeconds - minTime) / (maxTime - minTime);
    const double yRatio = (value - minValue) / (maxValue - minValue);
    return QPointF(plotRect.left() + xRatio * plotRect.width(),
                   plotRect.bottom() - yRatio * plotRect.height());
}

void ForceTorqueWidget::drawSeries(QPainter &painter, const QRectF &plotRect, int seriesIndex, const QColor &color, double minTime, double maxTime, double minValue, double maxValue) const
{
    if (samples.size() < 2) {
        return;
    }

    QPainterPath path;
    path.moveTo(mapPoint(plotRect, samples.first().timeSeconds, valueAt(samples.first(), seriesIndex), minTime, maxTime, minValue, maxValue));
    for (int i = 1; i < samples.size(); ++i) {
        path.lineTo(mapPoint(plotRect, samples.at(i).timeSeconds, valueAt(samples.at(i), seriesIndex), minTime, maxTime, minValue, maxValue));
    }

    painter.setPen(QPen(color, seriesIndex == 1 ? 1 : 2));
    painter.drawPath(path);
}

double ForceTorqueWidget::valueAt(const ForceSample &sample, int seriesIndex) const
{
    switch (seriesIndex) {
    case 0:
        return sample.trueForce;
    case 1:
        return sample.measuredForce;
    case 2:
        return sample.movingAverage;
    case 3:
        return sample.ewma;
    case 4:
        return sample.median;
    case 5:
        return sample.hampel;
    default:
        return 0.0;
    }
}
