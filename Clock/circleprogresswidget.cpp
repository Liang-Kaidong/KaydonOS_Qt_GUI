#include "circleprogresswidget.h"
#include <QPainter>
#include <QLinearGradient>
#include <QBrush>
#include <QtMath>
#include <QDebug>

CircleProgressWidget::CircleProgressWidget(QWidget *parent)
    : QWidget(parent)
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
}

CircleProgressWidget::~CircleProgressWidget()
{

}

void CircleProgressWidget::setCountdownTime(int h, int m, int s)
{
    hours = h;
    minutes = m;
    seconds = s;

    totalTime = (hours * 3600) + (minutes * 60) + seconds;
    currentTime = totalTime;

    //qDebug() << "Total Time: " << totalTime << "Current Time: " << currentTime;

    updateLabelTime();
    update();   /* 触发重新绘制 */
}

void CircleProgressWidget::startProgressTimer(int time)
{
    timer->start(time);
}

void CircleProgressWidget::stopProgressTimer()
{
    timer->stop();
}

void CircleProgressWidget::updateProgress()
{
    //qDebug() << "Current Time: " << currentTime;

    if (currentTime > 0) {
        currentTime--;
        //qDebug() << "Updated Time: " << currentTime;
        updateLabelTime();
        update();
    } else {
        timer->stop();
        emit countFinished();
    }
}

void CircleProgressWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

    /* 圆环参数 */
    int radius = 115;          // 圆环中心线半径
    int ringWidth = 20;        // 圆环宽度
    QPoint center(width() / 2, height() / 2);

    /* 1. 绘制完整纯色圆环（#3482FF） */
    QColor baseColor = QColor(52, 130, 255);    // #3482FF

    /* 圆角画笔 */
    QPen progressPen(baseColor, ringWidth);
    progressPen.setCapStyle(Qt::RoundCap);      // 圆角端点
    progressPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(progressPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);

    /* 2. 计算进度角度 */
    double progressAngle = 0.0;
    if (totalTime > 0) {
        progressAngle = 360.0 * (totalTime - currentTime) / totalTime;
    }

    /* 3. 绘制灰色进度圆弧（圆角） */
    QPen grayPen(Qt::gray, ringWidth);
    grayPen.setCapStyle(Qt::RoundCap);
    grayPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(grayPen);

    int startAngle = 90 * 16;               // 从顶部开始
    int spanAngle = progressAngle * 16;     // 顺时针

    QRectF arcRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    painter.drawArc(arcRect, startAngle, spanAngle);

    /* 4. 白色小球（居中在圆环内） */
    double radian = qDegreesToRadians(90 + progressAngle);
    int circleX = center.x() + radius * cos(radian);
    int circleY = center.y() - radius * sin(radian);

    painter.setPen(Qt::transparent);
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(circleX, circleY), 7, 7);
    QWidget::paintEvent(event);             // 调用基类的paintEvent方法
}

void CircleProgressWidget::setcountTimeLabel(QLabel *label)
{
    countTimeLabel = label;

    if (countTimeLabel) {
        updateLabelTime();
    }
}

void CircleProgressWidget::updateLabelTime()
{
    if (!countTimeLabel) {
        return;
    }

    int h = currentTime / 3600;
    int m = (currentTime % 3600) /60;
    int s = currentTime % 60;

    QString timeStr = QString("%1:%2:%3")
                        .arg(h, 2, 10, QChar('0'))
                        .arg(m, 2, 10, QChar('0'))
                        .arg(s, 2, 10, QChar('0'));

    countTimeLabel->setText(timeStr);
}
