#ifndef CIRCLEPROGRESSWIDGET_H
#define CIRCLEPROGRESSWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QLabel>

class CircleProgressWidget : public QWidget
{
    Q_OBJECT
signals:
    void countFinished();

public:
    explicit CircleProgressWidget(QWidget *parent = nullptr);

    void setcountTimeLabel(QLabel *label);
    void setCountdownTime(int h, int m, int s);
    void startProgressTimer(int time);
    void stopProgressTimer();

    ~CircleProgressWidget();

public slots:
    void updateProgress();
    void updateLabelTime();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int currentTime = 0;
    int totalTime = 0;
    QTimer *timer;

    QLabel *countTimeLabel = nullptr;
};

#endif // CIRCLEPROGRESSWIDGET_H
