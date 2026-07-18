#include <QDebug>
#include <QPainter>
#include "monitor.h"
#include "ui_monitor.h"

Monitor::Monitor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Monitor)
    , cameraTimer(nullptr)
{
    ui->setupUi(this);

    /* 日期时间更新 */
    dateTimer = new QTimer(this);
    dateTimer->setInterval(1000);
    connect(dateTimer, &QTimer::timeout, this, &Monitor::updateDate);
    dateTimer->start();

    /* 距离检测 */
    warningTimer = new QTimer(this);
    warningTimer->setInterval(10);
    connect(warningTimer, &QTimer::timeout, this, &Monitor::warnUser);
    warningTimer->start();

    /* 蜂鸣器闪烁 */
    beepTimer = new QTimer(this);
    beepControl = new BeepControl(this);
    connect(beepTimer, &QTimer::timeout, this, &Monitor::beepTimerSlot);

    /* 相机画面：先创建控制器，判断成功再创建定时器 */
    controller = new CameraController(this);
    bool camOk = controller->startPreview();
    if(camOk){
        cameraTimer = new QTimer(this);
        connect(cameraTimer, &QTimer::timeout, this, &Monitor::updateFrame);
        cameraTimer->start(33);
        ui->viewLabel->clear();
    }else{
        qDebug() << "摄像头启动失败！不创建画面刷新定时器";
        // 无摄像头填充黑色背景（修复fill报错）
        QPixmap blankPix(ui->viewLabel->size());
        blankPix.fill(Qt::black);
        ui->viewLabel->setPixmap(blankPix);
    }
}

Monitor::~Monitor()
{
    beepTimer->stop();
    beepControl->setBeep(false);
    closeMonitor();
    delete ui;
}

void Monitor::updateDate()
{
    QString dateStr = QDateTime::currentDateTime().toString("yyyy.MM.dd");
    ui->dateLabel->setText(dateStr);
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->timeLabel->setText(timeStr);
}

void Monitor::warnUser()
{
    int distance = alsDistant.realDistantValue();
    ui->currentDistantPushButton->setText("当前距离 " + QString::number(distance) + "m");

    if (distance > 900) {
        if (beepTimer->isActive()) beepTimer->stop();
        beepControl->setBeep(false);
    } else if (distance < 50) {
        if (beepTimer->isActive()) beepTimer->stop();
        beepControl->setBeep(true);
    } else {
        int interval = 0;
        if (distance <= 100) interval = 100;
        else if (distance <= 300) interval = 200;
        else if (distance <= 500) interval = 400;
        else if (distance <= 700) interval = 600;
        else if (distance <= 900) interval = 700;

        if (!beepTimer->isActive()) {
            beepControl->setBeep(false);
            beepTimer->start(interval);
        } else if (beepTimer->interval() != interval) {
            beepTimer->setInterval(interval);
        }
    }
}

void Monitor::beepTimerSlot()
{
    beepControl->setBeep(!beepControl->getState());
}

void Monitor::updateFrame()
{
    // 双重防护：定时器存在 + 摄像头可用才读取帧
    if(!cameraTimer || !controller->cameraAvailable())
        return;

    QImage img = controller->getFrame();
    if (!img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(img)
                          .scaled(ui->viewLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap rounded(ui->viewLabel->size());
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rounded.rect(), 10, 10);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pixmap);
        ui->viewLabel->setPixmap(rounded);
    }
}

void Monitor::closeMonitor()
{
    if (dateTimer->isActive()) dateTimer->stop();
    if (warningTimer->isActive()) warningTimer->stop();
    if (beepTimer->isActive()) beepTimer->stop();
    if (cameraTimer && cameraTimer->isActive()) cameraTimer->stop();

    beepControl->setBeep(false);
    if (controller) controller->stopPreview();
    ui->viewLabel->clear();
    qDebug() << "Monitor已关闭";
}

void Monitor::openMonitor()
{
    if (!dateTimer->isActive()) dateTimer->start(1000);
    if (!warningTimer->isActive()) warningTimer->start(10);

    // 先判断摄像头是否可用，不可用直接返回
    if (!controller->cameraAvailable()) {
        qDebug() << "无摄像头，无法打开监控画面";
        return;
    }

    if (!controller->startPreview()) {
        qDebug() << "摄像头启动失败！";
        return;
    }

    if (cameraTimer && !cameraTimer->isActive()) {
        cameraTimer->start(33);
    }
    beepControl->setBeep(false);
    qDebug() << "Monitor已开启";
}
