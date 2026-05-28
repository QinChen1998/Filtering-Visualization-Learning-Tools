#include "SignalChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>
#include <QStringList>

#include <algorithm>

SignalChartWidget::SignalChartWidget(QWidget *parent)
    : QWidget(parent)
    , maxSampleCount(500)
{
    setMinimumSize(420, 260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SignalChartWidget::appendFrame(const SignalFrame &frame)
{
    frames.append(frame);
    while (frames.size() > maxSampleCount) {
        frames.removeFirst();
    }
    update();
}

void SignalChartWidget::clear()
{
    frames.clear();
    update();
}

void SignalChartWidget::setMaxSamples(int samples)
{
    maxSampleCount = qBound(50, samples, 5000);
    while (frames.size() > maxSampleCount) {
        frames.removeFirst();
    }
    update();
}

void SignalChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(248, 250, 252));

    const QRectF bounds = rect().adjusted(12, 12, -12, -12);
    const QRectF plotRect = bounds.adjusted(58, 26, -18, -42);

    painter.setPen(QPen(QColor(32, 41, 57), 1));
    painter.drawText(bounds.topLeft() + QPointF(0, 14), QStringLiteral("实时信号曲线"));

    if (frames.isEmpty()) {
        painter.setPen(QColor(91, 104, 124));
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("点击播放后开始生成实时信号"));
        return;
    }

    double minTime = frames.first().timeSeconds;
    double maxTime = frames.last().timeSeconds;
    if (qFuzzyCompare(minTime, maxTime)) {
        maxTime = minTime + 1.0;
    }

    double minValue = frames.first().referenceValue;
    double maxValue = frames.first().referenceValue;
    bool hasAuxiliarySeries = false;
    for (const SignalFrame &frame : frames) {
        minValue = std::min({minValue, frame.referenceValue, frame.noisyValue, frame.filteredValue});
        maxValue = std::max({maxValue, frame.referenceValue, frame.noisyValue, frame.filteredValue});
        if (frame.hasAuxiliaryValue) {
            hasAuxiliarySeries = true;
            minValue = std::min(minValue, frame.auxiliaryValue);
            maxValue = std::max(maxValue, frame.auxiliaryValue);
        }
    }
    if (qFuzzyCompare(minValue, maxValue)) {
        minValue -= 1.0;
        maxValue += 1.0;
    }
    const double padding = (maxValue - minValue) * 0.12;
    minValue -= padding;
    maxValue += padding;

    painter.setPen(QPen(QColor(214, 222, 235), 1));
    for (int i = 0; i <= 4; ++i) {
        const double x = plotRect.left() + plotRect.width() * i / 4.0;
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
    }
    for (int i = 0; i <= 4; ++i) {
        const double y = plotRect.top() + plotRect.height() * i / 4.0;
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
    }

    painter.setPen(QPen(QColor(79, 93, 115), 1));
    painter.drawRect(plotRect);
    painter.drawText(QRectF(bounds.left(), plotRect.top() - 8, 52, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(maxValue, 'f', 2));
    painter.drawText(QRectF(bounds.left(), plotRect.bottom() - 10, 52, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(minValue, 'f', 2));
    painter.drawText(QRectF(plotRect.left(), plotRect.bottom() + 8, plotRect.width(), 18), Qt::AlignCenter, QStringLiteral("时间窗口 %1 s - %2 s").arg(minTime, 0, 'f', 1).arg(maxTime, 0, 'f', 1));

    drawSeries(painter, plotRect, minTime, maxTime, minValue, maxValue, 0, QColor(31, 119, 180));
    drawSeries(painter, plotRect, minTime, maxTime, minValue, maxValue, 1, QColor(214, 84, 73));
    drawSeries(painter, plotRect, minTime, maxTime, minValue, maxValue, 2, QColor(37, 151, 88));
    if (hasAuxiliarySeries) {
        drawSeries(painter, plotRect, minTime, maxTime, minValue, maxValue, 3, QColor(136, 92, 170));
    }

    const int legendY = static_cast<int>(bounds.top() + 2);
    const QStringList labels = {
        hasAuxiliarySeries ? QStringLiteral("真实值") : QStringLiteral("原始信号"),
        hasAuxiliarySeries ? QStringLiteral("传感器 A") : QStringLiteral("带噪信号"),
        hasAuxiliarySeries ? QStringLiteral("融合结果") : QStringLiteral("滤波输出"),
        QStringLiteral("传感器 B")
    };
    const QList<QColor> colors = {
        QColor(31, 119, 180),
        QColor(214, 84, 73),
        QColor(37, 151, 88),
        QColor(136, 92, 170)
    };
    const int legendCount = hasAuxiliarySeries ? 4 : 3;
    int legendX = static_cast<int>(bounds.right()) - (legendCount == 4 ? 350 : 260);
    painter.setFont(font());
    for (int i = 0; i < legendCount; ++i) {
        painter.setPen(QPen(colors.at(i), 3));
        painter.drawLine(QPointF(legendX, legendY + 8), QPointF(legendX + 22, legendY + 8));
        painter.setPen(QColor(32, 41, 57));
        painter.drawText(QPointF(legendX + 28, legendY + 12), labels.at(i));
        legendX += 86;
    }
}

QPointF SignalChartWidget::mapPoint(const QRectF &plotRect, double timeSeconds, double value, double minTime, double maxTime, double minValue, double maxValue) const
{
    const double xRatio = (timeSeconds - minTime) / (maxTime - minTime);
    const double yRatio = (value - minValue) / (maxValue - minValue);
    return QPointF(plotRect.left() + xRatio * plotRect.width(),
                   plotRect.bottom() - yRatio * plotRect.height());
}

void SignalChartWidget::drawSeries(QPainter &painter, const QRectF &plotRect, double minTime, double maxTime, double minValue, double maxValue, int seriesIndex, const QColor &color) const
{
    if (frames.size() < 2) {
        return;
    }

    QPainterPath path;
    path.moveTo(mapPoint(plotRect, frames.first().timeSeconds, valueAt(frames.first(), seriesIndex), minTime, maxTime, minValue, maxValue));
    for (int i = 1; i < frames.size(); ++i) {
        path.lineTo(mapPoint(plotRect, frames.at(i).timeSeconds, valueAt(frames.at(i), seriesIndex), minTime, maxTime, minValue, maxValue));
    }

    painter.setPen(QPen(color, 2));
    painter.drawPath(path);
}

double SignalChartWidget::valueAt(const SignalFrame &frame, int seriesIndex) const
{
    switch (seriesIndex) {
    case 0:
        return frame.referenceValue;
    case 1:
        return frame.noisyValue;
    case 2:
        return frame.filteredValue;
    case 3:
        return frame.hasAuxiliaryValue ? frame.auxiliaryValue : frame.filteredValue;
    default:
        return 0.0;
    }
}
