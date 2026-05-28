#ifndef ROBOTARMWIDGET_H
#define ROBOTARMWIDGET_H

#include <QPointF>
#include <QVector>
#include <QWidget>

class RobotArmWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RobotArmWidget(QWidget *parent = nullptr);

    void updatePose(double rawJointAngle, double filteredJointAngle);
    void clearTrajectories();
    void setMaxTrailPoints(int points);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct ArmPose {
        QPointF joint;
        QPointF endEffector;
    };

    ArmPose computePose(double jointAngle, double scale) const;
    QPointF toWidgetPoint(const QPointF &modelPoint, const QPointF &origin) const;
    void drawTrail(QPainter &painter, const QVector<QPointF> &trail, const QPointF &origin, const QColor &color) const;
    void drawArm(QPainter &painter, const ArmPose &pose, const QPointF &origin, const QColor &color, const QString &label) const;

    double rawAngle;
    double filteredAngle;
    QVector<QPointF> rawTrail;
    QVector<QPointF> filteredTrail;
    int maxTrailPoints;
};

#endif // ROBOTARMWIDGET_H
