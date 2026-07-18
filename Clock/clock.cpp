#include <QString>
#include <QDebug>
#include <QProcess>
#include "clock.h"
#include "ui_clock.h"

Clock::Clock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Clock)
{
    ui->setupUi(this);

    lastClickedButton = ui->alarmPushButton;

    gesture = new Gesture(ui->clockStackedWidget, this);
    gesture->addHorizontalScrollWidget({0, 1, 2, 3});
    gesture->addVerticalScrollWidget(ui->timeKeeperTextBrowser);

    displayTimer = new QTimer(this);
    displayTimer->setInterval(10);  /* 10ms刷新一次 */
    connect(displayTimer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    initstopWatch();

    /* ui->circleProgressWidget相当于容器 */
    circleProgressWidget = new CircleProgressWidget(ui->circleProgressWidget);
    circleProgressWidget->resize(ui->circleProgressWidget->size());
    initsandClock();

    timeoutWidget = new TimeoutWidget(this);
    timeoutWidget->setVisible(false);

    connect(circleProgressWidget, SIGNAL(countFinished()), this, SLOT(showTimeoutWidget()));    /* 非提升为 */
    connect(timeoutWidget, SIGNAL(hideTimeoutWidget()), this, SLOT(hideTimeoutWidget()));

    ntpProcess = new QProcess(this);
    rtcProcess = new QProcess(this);
    connect(ntpProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onNtpFinished(int, QProcess::ExitStatus)));
    connect(rtcProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRtcFinished(int, QProcess::ExitStatus)));

    ui->clockStackedWidget->setCurrentIndex(0);
}

Clock::~Clock()
{
    delete ui;
}

void Clock::updatePushButtonUI(QPushButton *clickedButton)
{
    /* 将上一个按钮的样式恢复 */
    if (lastClickedButton != nullptr) {
        lastClickedButton->setStyleSheet("border: none;"
                                         "text-align: centre;"
                                         "border-radius: 14px;");
    }

    /* 更新现在点击的按钮样式 */
    clickedButton->setStyleSheet("border: none;"
                                 "text-align: centre;"
                                 "background-color: white;"
                                 "border-radius: 14px;");

    /* 更新lastClickedButton为当前点击的按钮 */
    lastClickedButton = clickedButton;
}

void Clock::on_alarmPushButton_clicked()
{
    if (lastClickedButton != ui->alarmPushButton) {
        gesture->slidePageLeftToRight(0);
    } else {
        gesture->slidePageRightToLeft(0);
    }

    updatePushButtonUI(ui->alarmPushButton);
}

void Clock::on_clockPushButton_clicked()
{
    if (lastClickedButton == ui->stopWatchPushButton || lastClickedButton == ui->sandClockPushButton) {
        gesture->slidePageLeftToRight(1);
    } else {
        gesture->slidePageRightToLeft(1);
    }

    updatePushButtonUI(ui->clockPushButton);
}

void Clock::on_updateTimePushButton_clicked()
{
    //qDebug() << "开始同步时间...";
    ntpProcess->start("ntpdate ntp.aliyun.com");
}

void Clock::onNtpFinished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode != 0) {
        qDebug() << "ntpdate 失败";
        return;
    }

    qDebug() << "系统时间同步成功";

    /* 第二步：写RTC */
    rtcProcess->start("hwclock -w");
}

void Clock::onRtcFinished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == 0) {
        qDebug() << "RTC完成";
    } else {
        qDebug() << "RTC失败";
    }
}

void Clock::initstopWatch()
{
    /* 默认隐藏控制按钮 */
    ui->controlPushButton1->hide();
    ui->controlPushButton2->hide();

    /* 默认隐藏并清空计时板 */
    ui->timeKeeperTextBrowser->hide();
    ui->timeKeeperTextBrowser->clear();
    number = 0;

    /* 计时居中 */
    ui->timeLabel1->move(349, 160);
    ui->timeLabel2->move(565, 160);
    ui->timeLabel3->move(570, 160);

    /* 默认显示开始按钮 */
    ui->startPushButton->show();

    /* 计时标志位，没有在计时 */
    isTiming = false;

    /* 初始化计时 */
    ui->timeLabel1->setText("00:00:00");
    ui->timeLabel2->setText(".");
    ui->timeLabel3->setText("00");
    totalMs = 0;
    displayTimer->stop();
}

void Clock::on_stopWatchPushButton_clicked()
{
    if (lastClickedButton == ui->sandClockPushButton) {
        gesture->slidePageLeftToRight(2);
    } else {
        gesture->slidePageRightToLeft(2);
    }

    updatePushButtonUI(ui->stopWatchPushButton);
}

void Clock::on_startPushButton_clicked()
{
    /* 隐藏开始按钮 */
    ui->startPushButton->hide();

    /* 展示两个控制按钮 */
    ui->controlPushButton1->show();
    ui->controlPushButton2->show();

    /* 如果开始计时 */
    ui->controlPushButton1->setIcon(QIcon(":/Icons/clock/Icons/flags.png"));  /* 控制按钮一为标记 */
    ui->controlPushButton2->setIcon(QIcon(":/Icons/clock/Icons/pause.png"));  /* 控制按钮二为暂停 */

    isTiming = true;
    displayTimer->start();
}

void Clock::on_controlPushButton2_clicked()
{
    if (isTiming == true) {
        ui->controlPushButton1->setIcon(QIcon(":/Icons/clock/Icons/stop.png"));  /* 控制按钮一为停止 */
        ui->controlPushButton2->setIcon(QIcon(":/Icons/clock/Icons/start.png"));  /* 控制按钮二为开始 */
        isTiming = false;
        displayTimer->stop();
    } else {
        ui->controlPushButton1->setIcon(QIcon(":/Icons/clock/Icons/flags.png"));  /* 控制按钮一为标记 */
        ui->controlPushButton2->setIcon(QIcon(":/Icons/clock/Icons/pause.png"));  /* 控制按钮二为暂停 */
        isTiming = true;
        displayTimer->start();
    }
}

void Clock::on_controlPushButton1_clicked()
{
    if (isTiming == true) {
        /* 这里写计时的函数 */
        number += 1;
        QString tempNumber;
        if (number <= 9) {
            tempNumber = "0" + QString::number(number);
        } else {
            tempNumber = QString::number(number);
        }

        QString tempTime;
        tempTime = ui->timeLabel1->text() + ui->timeLabel2->text() + ui->timeLabel3->text();

        /* 设置两端对齐 */
        QString showTimeKeeper = QString(
                "<table width='100%' style='border:none; margin:0; padding:0;'>"
                "<tr>"
                "<td align='left'>%1</td>"
                "<td align='right'>%2</td>"
                "</tr></table>"
            ).arg(tempNumber, tempTime);

        /* 展示计数板 */
        ui->timeKeeperTextBrowser->show();
        ui->timeKeeperTextBrowser->append(showTimeKeeper);

        /* 计时置顶 */
        ui->timeLabel1->move(349, 21);
        ui->timeLabel2->move(565, 21);
        ui->timeLabel3->move(570, 21);

    } else {
        /* 此时点击停止计时（重置） */
        /* 这里写重置计时的函数 */
        initstopWatch();
    }
}

void Clock::updateTimer()
{
    if (isTiming) {
        totalMs += 10;

        /* 计算小时/分钟/秒/毫秒 */
        int hours = totalMs / 3600000;
        int minutes = (totalMs % 3600000) / 60000;
        int seconds = (totalMs % 60000) / 1000;
        int milliseconds = (totalMs % 1000) / 10;

        ui->timeLabel1->setText(QString::asprintf("%02d:%02d:%02d", hours, minutes, seconds));
        ui->timeLabel3->setText(QString::asprintf("%02d", milliseconds));
    }
}

void Clock::initsandClock()
{
    ui->hoursSetLineEdit->setText("00");
    ui->minutesSetLineEdit->setText("00");
    ui->secondsSetLineEdit->setText("00");

    isCounting = false;
    isCountPause = false;

    circleProgressWidget->stopProgressTimer();
    ui->controlPushButton4->setIcon(QIcon(":/Icons/clock/Icons/pause.png"));

    gesture->addHorizontalScrollWidget({0, 1, 2, 3});
    ui->clockStackedWidget->setCurrentIndex(3);

    ui->countTimeLabel->raise();
}

void Clock::on_sandClockPushButton_clicked()
{
    if (isCounting == true) {
        gesture->slidePageRightToLeft(4);
    } else {
        gesture->slidePageRightToLeft(3);
    }
    updatePushButtonUI(ui->sandClockPushButton);
}

void Clock::on_timeSetPushButton1_clicked()
{
    ui->hoursSetLineEdit->setText("00");
    ui->minutesSetLineEdit->setText("05");
    ui->secondsSetLineEdit->setText("00");
}

void Clock::on_timeSetPushButton2_clicked()
{
    ui->hoursSetLineEdit->setText("00");
    ui->minutesSetLineEdit->setText("10");
    ui->secondsSetLineEdit->setText("00");
}

void Clock::on_timeSetPushButton3_clicked()
{
    ui->hoursSetLineEdit->setText("00");
    ui->minutesSetLineEdit->setText("30");
    ui->secondsSetLineEdit->setText("00");
}

void Clock::on_timeSetPushButton4_clicked()
{
    ui->hoursSetLineEdit->setText("01");
    ui->minutesSetLineEdit->setText("00");
    ui->secondsSetLineEdit->setText("00");
}

void Clock::on_timeSetPushButton5_clicked()
{
    ui->hoursSetLineEdit->setText("03");
    ui->minutesSetLineEdit->setText("00");
    ui->secondsSetLineEdit->setText("00");
}

void Clock::on_startSetTimePushButton_clicked()
{
    int hours = ui->hoursSetLineEdit->text().toInt();       // 将QString转换为int
    int minutes = ui->minutesSetLineEdit->text().toInt();   // 将QString转换为int
    int seconds = ui->secondsSetLineEdit->text().toInt();   // 将QString转换为int

    if (hours == 0 && minutes == 0 && seconds == 0) {
        return;
    }

    isCounting = true;
    if (isCounting == true) {
        gesture->addHorizontalScrollWidget({0, 1, 2, 4});
        ui->clockStackedWidget->setCurrentIndex(4);
    }

    circleProgressWidget->setCountdownTime(hours, minutes, seconds);
    circleProgressWidget->startProgressTimer(1000);
    circleProgressWidget->setcountTimeLabel(ui->countTimeLabel);

    ui->countTimeLabel->setText(ui->hoursSetLineEdit->text() + ":" + ui->minutesSetLineEdit->text() + ":" + ui->secondsSetLineEdit->text());
}

void Clock::on_controlPushButton3_clicked()
{
   ui->clockStackedWidget->setCurrentIndex(3);
   initsandClock();
}

void Clock::on_controlPushButton4_clicked()
{
    if (isCountPause == false) {
        ui->controlPushButton4->setIcon(QIcon(":/Icons/clock/Icons/start.png"));
        circleProgressWidget->stopProgressTimer();
        isCountPause = true;
    } else {
        ui->controlPushButton4->setIcon(QIcon(":/Icons/clock/Icons/pause.png"));
        circleProgressWidget->startProgressTimer(1000);
        isCountPause = false;
    }
}

void Clock::showTimeoutWidget()
{
    timeoutWidget->setVisible(true);
}

void Clock::hideTimeoutWidget()
{
    timeoutWidget->setVisible(false);
    initsandClock();
}

void Clock::on_clockStackedWidget_currentChanged(int arg1)
{
    arg1 = ui->clockStackedWidget->currentIndex();

    if (arg1 == 0) {
        updatePushButtonUI(ui->alarmPushButton);
    } else if (arg1 == 1) {
        updatePushButtonUI(ui->clockPushButton);
    } else if (arg1 == 2) {
        updatePushButtonUI(ui->stopWatchPushButton);
    } else {
        updatePushButtonUI(ui->sandClockPushButton);
    }
}
