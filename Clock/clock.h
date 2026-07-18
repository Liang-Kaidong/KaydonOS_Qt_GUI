#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QProcess>
#include "gesture.h"
#include "circleprogresswidget.h"
#include "timeoutwidget.h"

namespace Ui {
class Clock;
}

class Clock : public QWidget
{
    Q_OBJECT
public:
    explicit Clock(QWidget *parent = nullptr);
    ~Clock();

private slots:
    void on_alarmPushButton_clicked();
    void on_clockPushButton_clicked();
    void on_stopWatchPushButton_clicked();
    void on_sandClockPushButton_clicked();

    void updatePushButtonUI(QPushButton *clickedButton);
    void on_clockStackedWidget_currentChanged(int arg1);

    void on_startPushButton_clicked();
    void on_controlPushButton2_clicked();
    void on_controlPushButton1_clicked();
    void updateTimer();

    void on_timeSetPushButton1_clicked();
    void on_timeSetPushButton2_clicked();
    void on_timeSetPushButton3_clicked();
    void on_timeSetPushButton4_clicked();
    void on_timeSetPushButton5_clicked();
    void on_startSetTimePushButton_clicked();
    void on_controlPushButton3_clicked();
    void on_controlPushButton4_clicked();

    void showTimeoutWidget();
    void hideTimeoutWidget();

    void on_updateTimePushButton_clicked();

    void onNtpFinished(int, QProcess::ExitStatus);
    void onRtcFinished(int, QProcess::ExitStatus);

private:
    Ui::Clock *ui;
    Gesture *gesture;
    QPushButton *lastClickedButton = nullptr;       /* 用指针保存按钮 */

    QTimer *displayTimer;        /* 计时器 */
    void initstopWatch();        /* 初始化计时功能 */
    bool isTiming = false;       /* 标记正在计时 */
    qint64 totalMs = 0;          /* 总毫秒数 */
    int number = 0;              /* 计数 */

    CircleProgressWidget *circleProgressWidget;
    TimeoutWidget *timeoutWidget;
    void initsandClock();
    bool isCounting = false;    /* 标记正在倒计时 */
    bool isCountPause = false;  /* 标记暂停倒计时 */

    QProcess *ntpProcess;
    QProcess *rtcProcess;
};

#endif // CLOCK_H
