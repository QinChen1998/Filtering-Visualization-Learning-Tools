#include "RobotArmWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>

#include <cmath>

RobotArmWidget::RobotArmWidget(QWidget *parent)
    : QWidget(parent)
    , rawAngle(0.0)
    , filteredAngle(0.0)
    , maxTrailPoints(180)
{
    setMinimumSize(420, 220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void RobotArmWidget::updatePose(double rawJointAngle, double filteredJointAngle)
{
    rawAngle = rawJointAngle;
    filteredAngle = filteredJointAngle;

    const double scale = qMin(width(), height()) * 0.25;
    rawTrail.append(computePose(rawAngle, scale).endEffector);
    filteredTrail.append(computePose(filteredAngle, scale).endEffector);
    while (rawTrail.size() > maxTrailPoints) {
        rawTrail.removeFirst();
    }
    while (filteredTrail.size() > maxTrailPoints) {
        filteredTrail.removeFirst();
    }
    update();
}

void RobotArmWidget::clearTrajectories()
{
    rawTrail.clear();
    filteredTrail.clear();
    rawAngle = 0.0;
    filteredAngle = 0.0;
    update();
}

void RobotArmWidget::setMaxTrailPoints(int points)
{
    maxTrailPoints = qBound(20, points, 2000);
}

void RobotArmWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(250, 251, 253));

    const QRectF bounds = rect().adjusted(12, 10, -12, -10);
    const QPointF origin(bounds.left() + bounds.width() * 0.5, bounds.top() + bounds.height() * 0.72);
    const double scale = qMin(bounds.width(), bounds.height()) * 0.26;

    painter.setPen(QColor(32, 41, 57));
    painter.drawText(bounds.topLeft() + QPointF(0, 14), QStringLiteral("机械臂姿态与末端轨迹"));

    painter.setPen(QPen(QColor(220, 226, 235), 1));
    painter.drawLine(QPointF(bounds.left(), origin.y()), QPointF(bounds.right(), origin.y()));

    drawTrail(painter, rawTrail, origin, QColor(31, 119, 180, 110));
    drawTrail(painter, filteredTrail, origin, QColor(37, 151, 88, 130));

    drawArm(painter, computePose(rawAngle, scale), origin, QColor(31, 119, 180), QStringLiteral("滤波前"));
    drawArm(painter, computePose(filteredAngle, scale), origin, QColor(37, 151, 88), QStringLiteral("滤波后"));

    const int legendX = static_cast<int>(bounds.right()) - 180;
    painter.setPen(QPen(QColor(31, 119, 180), 3));
    painter.drawLine(QPointF(legendX, bounds.top() + 12), QPointF(legendX + 22, bounds.top() + 12));
    painter.setPen(QColor(32, 41, 57));
    painter.drawText(QPointF(legendX + 28, bounds.top() + 16), QStringLiteral("滤波前"));
    painter.setPen(QPen(QColor(37, 151, 88), 3));
    painter.drawLine(QPointF(legendX + 92, bounds.top() + 12), QPointF(legendX + 114, bounds.top() + 12));
    painter.setPen(QColor(32, 41, 57));
    painter.drawText(QPointF(legendX + 120, bounds.top() + 16), QStringLiteral("滤波后"));
}

RobotArmWidget::ArmPose RobotArmWidget::computePose(double jointAngle, double scale) const
{
    const double shoulderAngle = -1.25 + jointAngle * 0.75;
    const double elbowAngle = 1.35 + std::sin(jointAngle * 0.8) * 0.32;
    const double upperLength = scale;
    const double lowerLength = scale * 0.78;

    ArmPose pose;
    pose.joint = QPointF(std::cos(shoulderAngle) * upperLength,
                         std::sin(shoulderAngle) * upperLength);
    pose.endEffector = pose.joint + QPointF(std::cos(shoulderAngle + elbowAngle) * lowerLength,
                                            std::sin(shoulderAngle + elbowAngle) * lowerLength);
    return pose;
}

QPointF RobotArmWidget::toWidgetPoint(const QPointF &modelPoint, const QPointF &origin) const
{
    return QPointF(origin.x() + modelPoint.x(), origin.y() + modelPoint.y());
}

void RobotArmWidget::drawTrail(QPainter &painter, const QVector<QPointF> &trail, const QPointF &origin, const QColor &color) const
{
    if (trail.size() < 2) {
        return;
    }

    QPainterPath path;
    path.moveTo(toWidgetPoint(trail.first(), origin));
    for (int i = 1; i < trail.size(); ++i) {
        path.lineTo(toWidgetPoint(trail.at(i), origin));
    }

    painter.setPen(QPen(color, 2));
    painter.drawPath(path);
}

void RobotArmWidget::drawArm(QPainter &painter, const ArmPose &pose, const QPointF &origin, const QColor &color, const QString &label) const
{
    const QPointF base = origin;
    const QPointF joint = toWidgetPoint(pose.joint, origin);
    const QPointF end = toWidgetPoint(pose.endEffector, origin);

    painter.setPen(QPen(color, 7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(base, joint);
    painter.drawLine(joint, end);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 247, 250));
    painter.drawEllipse(base, 8, 8);
    painter.drawEllipse(joint, 7, 7);
    painter.drawEllipse(end, 6, 6);
    painter.setPen(QPen(color.darker(130), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(base, 8, 8);
    painter.drawEllipse(joint, 7, 7);
    painter.drawEllipse(end, 6, 6);

    painter.setPen(color.darker(140));
    painter.drawText(end + QPointF(8, -6), label);
}
