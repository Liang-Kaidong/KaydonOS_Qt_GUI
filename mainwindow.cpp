#include <QDebug>
#include <QLineEdit>
#include <QDateTime>
#include <QTimer>
#include <QProcess>
#include <QMouseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"

int previousPage = 0;   /* 初始化上一页为桌面页 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("KaydonOS");

    /* 手势管理 */
    gesture = new Gesture(ui->desktopStackedWidget, this);
    ui->desktopStackedWidget->setCurrentIndex(0);
    gesture->addHorizontalScrollWidget({0, 1});                     /* 桌面页(page0, page1)允许拖动翻页 */
    gesture->addVerticalScrollWidget(ui->allAppsScrollArea);        /* 注册允许纵向滑动的控件（例如 selectionScrollArea） */

    /* 音量控制模块初始化 */
    voiceControl = new VoiceControl(this);
    voiceControl->initAudioHardware();
    ui->voiceControlSlider->setValue(voiceControl->currentVolume); /* 同步当前音量 */
    if (ui->voiceControlSlider->value() == 0) {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
    }
    ui->voiceControlSlider->setPageStep(0);

    /* 插入系统设置页面 */
    systemSettingPage = new SystemSetting(this, voiceControl);
    ui->desktopStackedWidget->insertWidget(3, systemSettingPage);

    /* 插入音乐播放页面 */
    musicPlayerPage = new MusicPlayer(this, voiceControl);
    ui->desktopStackedWidget->insertWidget(4, musicPlayerPage);

    /* 插入天气页面 */
    weatherPage = new Weather(this);
    ui->desktopStackedWidget->insertWidget(5, weatherPage);

    /* 插入时钟页面 */
    clockPage = new Clock(this);
    ui->desktopStackedWidget->insertWidget(6, clockPage);

    /* 插入视频播放器页面 */
    videoPlayerPage = new VideoPlayer(this, voiceControl);
    ui->desktopStackedWidget->insertWidget(7, videoPlayerPage);

    /* 插入相机页面 */
    cameraPage = new Camera(this);
    openCameraTimer = new QTimer(this);
    ui->desktopStackedWidget->insertWidget(8, cameraPage);
    cameraPage->closeCamera();
    openCameraTimer->setInterval(1500);
    connect(openCameraTimer, SIGNAL(timeout()), this, SLOT(onCameraAPPOpen()));

    /* 插入倒车监控页面 */
    monitorPage = new Monitor(this);
    openMonitorTimer = new QTimer(this);
    ui->desktopStackedWidget->insertWidget(9, monitorPage);
    monitorPage->closeMonitor();
    openMonitorTimer->setInterval(1500);
    connect(openMonitorTimer, SIGNAL(timeout()), this, SLOT(onMonitorAPPOpen()));

    /* 插入计算器页面 */
    caculatorPage = new Caculator(this);
    ui->desktopStackedWidget->insertWidget(10, caculatorPage);

    /* 插入相册页面 */
    galleryPage = new Gallery(this);
    ui->desktopStackedWidget->insertWidget(11, galleryPage);

    /* 插入日历页面 */
    calendarPage = new Calendar(this);
    ui->desktopStackedWidget->insertWidget(12, calendarPage);

    /* 插入录音页面 */
    recorderPage = new Recorder(this);
    ui->desktopStackedWidget->insertWidget(13, recorderPage);

    /* 插入性能监控工具页面 */
    performanceToolPage = new PerformanceTool(this);
    performanceToolPage->setMainWindow(this);
    ui->desktopStackedWidget->insertWidget(14, performanceToolPage);

    /*
     * 打印页面索引（调试用）
     * qDebug() << "页面索引：";
     * qDebug() << "程序启动，当前索引：" << ui->desktopStackedWidget->currentIndex();
     * qDebug() << "desktopPage0：0";
     * qDebug() << "desktopPage1：1";
     * qDebug() << "allAppsPage：2";
     * qDebug() << "systemSettingPage：" << ui->desktopStackedWidget->indexOf(systemSettingPage);
     * qDebug() << "musicPlayerPage" << ui->desktopStackedWidget->indexOf(musicPlayerPage);
     * qDebug() << "weatherPage" << ui->desktopStackedWidget->indexOf(weatherPage);
     * qDebug() << "clockPage" << ui->desktopStackedWidget->indexOf(clockPage);
     * qDebug() << "videoPlayerPage" << ui->desktopStackedWidget->indexOf(videoPlayerPage);
     * qDebug() << "cameraPage" << ui->desktopStackedWidget->indexOf(cameraPage);
     * qDebug() << "monitorPage" << ui->desktopStackedWidget->indexOf(monitorPage);
     * qDebug() << "caculator" << ui->desktopStackedWidget->indexOf(caculatorPage);
     * qDebug() << "gallery" << ui->desktopStackedWidget->indexOf(galleryPage);
     * qDebug() << "calendar" << ui->desktopStackedWidget->indexOf(calendarPage);
     * qDebug() << "recorder" << ui->desktopStackedWidget->indexOf(recorderPage);
     * qDebug() << "performanceTool" << ui->desktopStackedWidget->indexOf(performanceToolPage);
     */

    connect(ui->desktopStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(updatePageIndicator(int)));

    /* 初始化定时器 */
    timeTimer = new QTimer(this);
    timeTimer->setInterval(1000);
    connect(timeTimer, SIGNAL(timeout()), this, SLOT(updateCurrentTime()));
    updateCurrentTime();
    timeTimer->start();

    /* 初始化页面指示器样式 */
    updatePageIndicator(0);

    /* 自定义消息提示 */
    exitMessageBox = new ExitMessageBox(this);

    /* 接受来自各应用发送给主页面的音量改变信号 */
    connect(systemSettingPage, SIGNAL(onSystemSettingVolumeChanged(int)), this, SLOT(onSystemSettingVolumeChanged(int)));
    connect(musicPlayerPage, SIGNAL(onMusicPlayerVolumeChanged(int)), this, SLOT(onMusicPlayerVolumeChanged(int)));
    connect(videoPlayerPage, SIGNAL(onVideoPlayerVolumeChanged(int)), this, SLOT(onVideoPlayerVolumeChanged(int)));

    /* 发生来自主界面给各应用的音量改变信号 */
    connect(this, SIGNAL(onMainWindowsVolumeChanged(int)), systemSettingPage, SLOT(onMainWindowsVolumeChanged(int)));
    connect(this, SIGNAL(onMainWindowsVolumeChanged(int)), musicPlayerPage, SLOT(onMainWindowsVolumeChanged(int)));
    connect(this, SIGNAL(onMainWindowsVolumeChanged(int)), videoPlayerPage, SLOT(onMainWindowsVolumeChanged(int)));

    /* 初始化空调温度 */
    QString currentTemperatureText = ui->temperatureLineEdit->text();
    currentTemperatureText = "25℃";
    ui->temperatureLineEdit->setText(currentTemperatureText);
    //qDebug() << "当前温度 " << currentTemperatureText;
    ui->temperatureLineEdit->setEnabled(false); /* QLineEdit当前版本无法设置文本不可被选中，强制不允许与该控件交互 */

    /* 初始化DHT11数据 */
    dht11Driver = new DHT11Driver(this);
    dht11Driver->initDHT11Driver();
    dht11Timer = new QTimer(this);
    dht11Timer->setInterval(15000);
    connect(dht11Timer, SIGNAL(timeout()), this, SLOT(onDHT11Timeout()));
    dht11Timer->start();
    onDHT11Timeout();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* 当前APP */
QString MainWindow::currentRunningAppName()
{
    int idx = ui->desktopStackedWidget->currentIndex();

    if (idx == 0) return "Desktop0";
    if (idx == 1) return "Desktop1";
    if (idx == 2) return "AllApps";
    if (idx == ui->desktopStackedWidget->indexOf(systemSettingPage)) return "SystemSetting";
    if (idx == ui->desktopStackedWidget->indexOf(musicPlayerPage)) return "MusicPlayer";
    if (idx == ui->desktopStackedWidget->indexOf(weatherPage)) return "Weather";
    if (idx == ui->desktopStackedWidget->indexOf(clockPage)) return "Clock";
    if (idx == ui->desktopStackedWidget->indexOf(videoPlayerPage)) return "VideoPlayer";
    if (idx == ui->desktopStackedWidget->indexOf(cameraPage)) return "Camera";
    if (idx == ui->desktopStackedWidget->indexOf(monitorPage)) return "Monitor";
    if (idx == ui->desktopStackedWidget->indexOf(caculatorPage)) return "Calculator";
    if (idx == ui->desktopStackedWidget->indexOf(galleryPage)) return "Gallery";
    if (idx == ui->desktopStackedWidget->indexOf(calendarPage)) return "Calendar";
    if (idx == ui->desktopStackedWidget->indexOf(recorderPage)) return "Recorder";
    if (idx == ui->desktopStackedWidget->indexOf(performanceToolPage)) return "PerformanceTool";

    return "Unknown";
}

bool MainWindow::isDesktopPage()
{
    int currentIndex = ui->desktopStackedWidget->currentIndex();
    return (currentIndex == 0 || currentIndex == 1);
}

void MainWindow::updateCurrentTime()
{
    QDateTime currentDataTime = QDateTime::currentDateTime();
    ui->timeTimerEdit->setText(currentDataTime.toString("HH:mm:ss"));
    ui->timeTimerEdit2->setText(currentDataTime.toString("HH:mm"));

    QStringList weekDays = {"周日","周一", "周二", "周三", "周四", "周五", "周六"};
    int dayOfWeek = currentDataTime.date().dayOfWeek();
    QString weekDayText = weekDays[dayOfWeek % 7];

    int hour = currentDataTime.time().hour();
    QString periodText;
    if (hour >= 6 && hour < 12) {
        periodText = "早上";
    } else if (hour >= 12 && hour < 18) {
        periodText = "中午";
    } else {
        periodText = "晚上";
    }

    ui->weekTimeLabel->setText(QString("%1%2").arg(weekDayText).arg(periodText));
    ui->dateLabel->setText(currentDataTime.date().toString("MM/dd"));
}

/* 显示当前温度和湿度 */
void MainWindow::onDHT11Timeout()
{
    QString dht11Data = dht11Driver->getDHT11Data();
    ui->DHT11Datalabel->setText(dht11Data);
}

/* 页面指示器更新 */
void MainWindow::updatePageIndicator(int currentIndex)
{
    if (currentIndex == 0) {
        ui->desktopPageIndicator->setStyleSheet("border-image: url(:/Pics/Desktop/Icons/desktopPageIndicator0.png);");
    } else if (currentIndex == 1) {
        ui->desktopPageIndicator->setStyleSheet("border-image: url(:/Pics/Desktop/Icons/desktopPageIndicator1.png);");
    }
}

/*********************************************************************************************************
 * 导航栏部分
 ********************************************************************************************************/

/* 结束程序按钮 */
void MainWindow::on_exitAppPushButton_clicked()
{
    int currentIndex = ui->desktopStackedWidget->currentIndex();

    if (currentIndex != 0 && currentIndex != 1 && currentIndex != 2) {
        /* 直接使用返回值判断 */
        exitMessageBox->setString("确定结束该程序吗？", "该程序正在运行中，点击确定后会强制结束该程序的所有进程", "取消", "确定");
        int ret = exitMessageBox->executeModal();
        if (ret == 1) { /* OK */
            if (currentIndex == ui->desktopStackedWidget->indexOf(systemSettingPage)) {
                gesture->closeApp(0);
                systemSettingPage->resetPage();
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(musicPlayerPage)) {
                gesture->closeApp(0);
                musicPlayerPage->resetPage();
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(weatherPage)) {
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(clockPage)) {
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(videoPlayerPage)) {
                gesture->closeApp(0);
                videoPlayerPage->resetAPP();
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(cameraPage)) {
                cameraPage->closeCamera();
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(monitorPage)) {
                monitorPage->closeMonitor();
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(caculatorPage)) {
                caculatorPage->resetAPP();
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(galleryPage)) {
                galleryPage->resetAPP();
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(calendarPage)) {
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(recorderPage)) {
                recorderPage->closeRecorder();
                gesture->closeApp(0);
            } else if (currentIndex == ui->desktopStackedWidget->indexOf(performanceToolPage)) {
                performanceToolPage->closePerformanceTool();
                gesture->closeApp(0);
            }
        } else { /* Cancel */
            //qDebug() << "取消退出";
        }
    } else {
        //qDebug() << "当前是桌面页，无需操作";
    }
}

/* 打开系统设置按钮(桌面page1的设置) */
void MainWindow::on_systemSettingPushButton_clicked()
{
    int currentIndex = ui->desktopStackedWidget->currentIndex();

    if (currentIndex == 3) {
        //qDebug() << "当前已在设置页面";
    } else {
        gesture->openApp(3); // 打开系统设置页面
        //qDebug() << "跳转到系统设置页面，索引：" << ui->desktopStackedWidget->currentIndex();
    }

    /* 同步修改后的音量参数 */
    emit onMainWindowsVolumeChanged(voiceControl->currentVolume);

    previousPage = 3;
}

/******************************音量设置相关******************************/
/* 音量图标被点击 */
void MainWindow::on_voiceControlPushButton_clicked()
{
    if (voiceControl->currentVolume !=0) {
        voiceControl->lastVolume = ui->voiceControlSlider->value();     /* 先保存静音前的音量 */
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");   /* 设置静音图标 */
        qDebug() << "已静音";
        qDebug() << "当前音量：" << voiceControl->currentVolume << "%";
        ui->voiceControlSlider->setValue(0);    /* 滑轨UI置0 */
        voiceControl->applyVolume(0);           /* 实际将音量置0 */
        emit onMainWindowsVolumeChanged(0);     /* 发送来自MainWindows的信号 */
    } else {
        if (voiceControl->currentVolume == voiceControl->lastVolume) {
            /* currentVolume = lastVolume = 0时，给出图标更新提醒 */
            static int icon = 0;
            if (icon == 0) {
                ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
                icon = 1;
            } else {
                ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
                icon = 0;
            }
            return;
        }
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
        qDebug() << "取消静音";
        qDebug() << "当前音量：" << voiceControl->currentVolume << "%";
        ui->voiceControlSlider->setValue(voiceControl->lastVolume);     /* 先恢复滑轨UI的值 */
        voiceControl->applyVolume(voiceControl->lastVolume);            /* 再实际恢复音量 */
        emit onMainWindowsVolumeChanged(voiceControl->lastVolume);
    }
}

/* 点击音量条，先保存上一个音量 */
void MainWindow::on_voiceControlSlider_sliderPressed()
{
    int lastVolume = ui->voiceControlSlider->value();
    voiceControl->lastVolume = lastVolume;
}

/* 滑轨松手后播放提示音 */
void MainWindow::on_voiceControlSlider_sliderReleased()
{
    int voiceControlSliderValue = ui->voiceControlSlider->value();

    if (voiceControlSliderValue == 0) {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
        voiceControl->applyVolume(0);     /* 修改实际音量 */
        qDebug() << "当前音量：" << voiceControl->currentVolume << "%";    /* 显示当前音量 */
        emit onMainWindowsVolumeChanged(0); /* 发射来自导航栏的音量设置 */
    } else if (voiceControlSliderValue != 0) {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
        voiceControl->applyVolume(voiceControlSliderValue);     /* 修改实际音量 */
        qDebug() << "当前音量：" << voiceControl->currentVolume << "%";    /* 显示当前音量 */
        emit onMainWindowsVolumeChanged(voiceControlSliderValue); /* 发射来自导航栏的音量设置 */
    }

    /**
     * 播放提示音，启动一个独立于Qt程序之外的进程，不阻塞UI
     * QString program = "aplay"; QString audioPath
     * QStringList args; args << audioPath;
     * QProcess::startDetached(program, args);
     */
    /* 读取配置文件，获取当前选中的音频路径 */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/audioSelection.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    QString currentAudioPath = settings.value("audioSelection/currentAudioSelection", "").toString();
    QProcess::startDetached("aplay", QStringList() << currentAudioPath);
}

/* 当在设置中的音量发生改变，同步至导航栏 */
void MainWindow::onSystemSettingVolumeChanged(int value)
{
    if (value == 0) {   /* 设置页点了静音图标 */
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
    } else {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
    }
    ui->voiceControlSlider->setValue(value);
}

/* 当在音乐播放器中的音量发生改变，同步至导航栏 */
void MainWindow::onMusicPlayerVolumeChanged(int value)
{
    if (value == 0) {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
    } else {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
    }
    ui->voiceControlSlider->setValue(value);
}

/* 当在视频播放器中的音量发生改变，同步至导航栏 */
void MainWindow::onVideoPlayerVolumeChanged(int value)
{
    if (value == 0) {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
    } else {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
    }
    ui->voiceControlSlider->setValue(value);
}

/* 返回桌面按钮 */
void MainWindow::on_backDesktopButton_clicked()
{
    int currentIndex = ui->desktopStackedWidget->currentIndex();

    if (currentIndex == 0) {
        //qDebug() << "已经在桌面第一页(desktopPage0)，无需切换";
        return;
    }
    //qDebug() << "从页面" << currentIndex << "返回桌面第一页(desktopPage0)";

    if (currentIndex == 1) {
        gesture->slidePageLeftToRight(0);  // 桌面1 → 桌面0，用滑动动画
        previousPage = 1;
        return;
    }

    /* 全部应用页回桌面 */
    if (currentIndex == 2) {
        gesture->slidePageUpToDown(0);
        previousPage = 2;
        return;
    }

    /* 设置APP回桌面 */
    if (currentIndex == 3) {
        gesture->closeApp(0);
        systemSettingPage->resetPage();
        previousPage = 3;
        return;
    }

    /* 音乐播放器APP回桌面 */
    if (currentIndex == 4) {
        gesture->closeApp(0);
        previousPage = 4;
        return;
    }

    /* 天气APP回桌面 */
    if (currentIndex == 5) {
        gesture->closeApp(0);
        previousPage = 5;
        return;
    }

    /* 时钟回桌面 */
    if (currentIndex == 6) {
        gesture->closeApp(0);
        previousPage = 6;
        return;
    }

    /* 视频播放器APP回桌面 */
    if (currentIndex == 7) {
        videoPlayerPage->mediaPlayer->pause();  /* 先暂停，避免界面卡顿 */
        gesture->closeApp(0);
        previousPage = 7;
        return;
    }

    /* 相机APP回桌面 */
    if (currentIndex == 8) {
        gesture->closeApp(0);
        previousPage = 8;
        cameraPage->closeCamera();
        return;
    }

    /* 倒车监控APP回桌面 */
    if (currentIndex == 9) {
        gesture->closeApp(0);
        previousPage = 9;
        monitorPage->closeMonitor();
        return;
    }

    /* 计算器APP回桌面 */
    if (currentIndex == 10) {
        gesture->closeApp(0);
        previousPage = 10;
        return;
    }

    /* 相册APP回桌面 */
    if (currentIndex == 11) {
        gesture->closeApp(0);
        previousPage = 11;
        return;
    }

    /* 日历APP回桌面 */
    if (currentIndex == 12) {
        gesture->closeApp(0);
        previousPage = 12;
        return;
    }

    /* 录音APP回桌面 */
    if (currentIndex == 13) {
        gesture->closeApp(0);
        recorderPage->closeRecorder();
        previousPage = 13;
        return;
    }

    /* 性能监控回桌面 */
    if (currentIndex == 14) {
        gesture->closeApp(0);
        previousPage = 14;
        return;
    }
}

/* 全部应用按钮 */
void MainWindow::on_allAppsPushButton_clicked()
{
    /* 切页逻辑 */
    int currentIndex = ui->desktopStackedWidget->currentIndex();
    if (currentIndex == 2 && previousPage == 2) {   /* 当已处在全部应用页，再次点击返回桌面 */
        gesture->slidePageUpToDown(0);
        previousPage = 0;   /* 手动更新上一页为桌面页 */
    } else if (previousPage !=2) {  /* 上一页不是桌面页 */
        if (currentIndex == 2) {    /* 当前是全部应用页面，上一页是应用界面 */
            gesture->slidePageUpToDown(previousPage);

            if (previousPage == 8) {    /* 先回到当前页，再打开相机，防止卡顿 */
                openCameraTimer->start();
            }

            if (previousPage == 9) {    /* 先回到当前页，再打开倒车监控，防止卡顿 */
                openMonitorTimer->start();
            }

        } else if (currentIndex == 0) {
            /**
              * 该处逻辑：
              * 当其他程序页面调用全部应用页面，先展示全部应用页面后，
              * 如果点击返回桌面，此时，再点击全部应用界面，再关闭全部应用界面，
              * 应当返回桌面，再一次手动标记上一个页面是桌面，
              * 避免页面记录错误。
              */
            gesture->slidePageUpToDown(0);
            previousPage = 0;
        } else if (currentIndex == 1) {
            /**
              * 该处逻辑：
              * 当其他程序页面调用全部应用页面，先展示全部应用页面后，
              * 如果点击返回桌面，此时，再点击全部应用界面，再关闭全部应用界面，
              * 应当返回桌面，再一次手动标记上一个页面是桌面，
              * 避免页面记录错误。
              */
            gesture->slidePageUpToDown(1);
            previousPage = 1;
        }
    }

    /* 打开全部应用页面 */
    if (currentIndex == 8) {    /* 处在相机界面时，因先将低负载，后打开应用抽屉 */
        cameraPage->closeCamera();
    }

    if (currentIndex == 9) {    /* 处在倒车监控界面时，因先将低负载，后打开应用抽屉 */
        monitorPage->closeMonitor();
    }

    gesture->slidePageDownToUp(2);
}


/* 打开音乐播放器（桌面） */
void MainWindow::on_musicPlayerPushButton_clicked()
{
    if (previousPage == 8) {
        cameraPage->closeCamera();
    }

    if (previousPage == 9) {
        monitorPage->closeMonitor();
    }

    gesture->openApp(4);
    previousPage = 4;
    /* 同步修改后的音量参数 */
    emit onMainWindowsVolumeChanged(voiceControl->currentVolume);
}

/* 打开音乐播放器（导航栏） */
void MainWindow::on_musicPlayerPushButton2_clicked()
{
    if (previousPage == 8) {
        cameraPage->closeCamera();
    }

    if (previousPage == 9) {
        monitorPage->closeMonitor();
    }

    gesture->openApp(4);
    previousPage = 4;
    /* 同步修改后的音量参数 */
    emit onMainWindowsVolumeChanged(voiceControl->currentVolume);
}

/* 降低空调温度 */
void MainWindow::on_downTemperaturePushButton_clicked()
{
    /* 获取当前温度 */
    QString currentTemperatureText = ui->temperatureLineEdit->text().toUtf8();
    /* 去掉℃符号 */
    currentTemperatureText.remove("℃");
    /* 转成整数 */
    int currentTemperatureValue = currentTemperatureText.toInt();

    if (currentTemperatureValue <= 16) {
        //qDebug() << "温度不得低于16℃";
        ui->downTemperaturePushButton->setEnabled(false);
        ui->riseTemperaturePushButton->setEnabled(true);
        ui->temperatureLineEdit->setEnabled(false);
        return;
    } else {
        /* 温度自减 */
        currentTemperatureValue -= 1;
        QString newTemperatureText = QString::number(currentTemperatureValue) + "℃";
        ui->temperatureLineEdit->setEnabled(true);
        ui->temperatureLineEdit->setText(newTemperatureText);
        ui->downTemperaturePushButton->setEnabled(true);
        ui->riseTemperaturePushButton->setEnabled(true);
        ui->temperatureLineEdit->setEnabled(false);
        //qDebug() << "当前温度： " << newTemperatureText;
        return;
    }
}

/* 提高空调温度 */
void MainWindow::on_riseTemperaturePushButton_clicked()
{
    QString currentTemperatureText = ui->temperatureLineEdit->text().toUtf8();
    currentTemperatureText.remove("℃");
    int currentTemperatureValue = currentTemperatureText.toInt();

    if (currentTemperatureValue >= 35) {
        //qDebug() << "温度不得高于35℃";
        ui->riseTemperaturePushButton->setEnabled(false);
        ui->downTemperaturePushButton->setEnabled(true);
        ui->temperatureLineEdit->setEnabled(false);
        return;
    } else {
        currentTemperatureValue += 1;
        QString newTemperatureText = QString::number(currentTemperatureValue) + "℃";
        ui->temperatureLineEdit->setEnabled(true);
        ui->temperatureLineEdit->setText(newTemperatureText);
        ui->riseTemperaturePushButton->setEnabled(true);
        ui->downTemperaturePushButton->setEnabled(true);
        ui->temperatureLineEdit->setEnabled(false);
        //qDebug() << "当前温度： " << newTemperatureText;
        return;
    }
}

/* 系统设置APP */
void MainWindow::on_settingButton_clicked()
{
    if (previousPage == 8) {
        cameraPage->closeCamera();
    }

    if (previousPage == 9) {
        monitorPage->closeMonitor();
    }

    int currentIndex = ui->desktopStackedWidget->currentIndex();

    if (currentIndex == 3) {
        //qDebug() << "当前已在设置页面";
    } else {
        gesture->openApp(3); // 打开系统设置页面
       //qDebug() << "跳转到系统设置页面，索引：" << ui->desktopStackedWidget->currentIndex();
    }

    /* 同步修改后的音量参数 */
    emit onMainWindowsVolumeChanged(voiceControl->currentVolume);

    previousPage = 3;
}

/* 天气APP */
void MainWindow::on_weatherPushButton_clicked()
{
    gesture->openApp(5);
    previousPage = 5;
}

/* 时钟APP */
void MainWindow::on_clockPushButton_clicked()
{
    gesture->openApp(6);
    previousPage = 6;
}

/* 视频播放器APP */
void MainWindow::on_videoPlayerPushButton_clicked()
{
    if (previousPage == 8) {
        cameraPage->closeCamera();
    }

    if (previousPage == 9) {
        monitorPage->closeMonitor();
    }

    gesture->openApp(7);
    previousPage = 7;
    /* 同步修改后的音量参数 */
    emit onMainWindowsVolumeChanged(voiceControl->currentVolume);
}

/* 视频播放器APP(桌面) */
void MainWindow::on_videoPlayerPushButton2_clicked()
{
    on_videoPlayerPushButton_clicked();
}

/* 相机APP */
void MainWindow::on_cameraPushButton_clicked()
{
    gesture->openApp(8);
    previousPage = 8;
    musicPlayerPage->pauseMusic();
    videoPlayerPage->pauseVideo();
    monitorPage->closeMonitor();
    openCameraTimer->start();
}

/* 相机打开CPU满载，最好延时进入 */
void MainWindow::onCameraAPPOpen()
{
    cameraPage->openCamera();
    openCameraTimer->stop();
}

/* 倒车监控APP */
void MainWindow::on_monitorPushButton_clicked()
{
    gesture->openApp(9);
    previousPage = 9;
    musicPlayerPage->pauseMusic();
    videoPlayerPage->pauseVideo();
    cameraPage->closeCamera();
    openMonitorTimer->start();
}

/* 倒车监控打开CPU满载，最好延时进入 */
void MainWindow::onMonitorAPPOpen()
{
    monitorPage->openMonitor();
    openMonitorTimer->stop();
}

/* 计算器APP */
void MainWindow::on_calculatorPushButton_clicked()
{
    gesture->openApp(10);
    previousPage = 10;
}

/* 相册APP */
void MainWindow::on_galleryPushButton_clicked()
{
    gesture->openApp(11);
    previousPage = 11;
}

/* 日历APP */
void MainWindow::on_calendarPushButton_clicked()
{
    gesture->openApp(12);
    previousPage = 12;
}

/* 录音APP */
void MainWindow::on_recorderPushButton_clicked()
{
    gesture->openApp(13);
    previousPage = 13;
}

/* 性能工具APP */
void MainWindow::on_performanceToolPushButton_clicked()
{
    gesture->openApp(14);
    previousPage = 14;
}

/* UDP通信APP(暂未集成) */
void MainWindow::on_updPushButton_clicked()
{
    /* 全部应用界面功能尚未集成，可自行添加 */
    exitMessageBox->setString("提示", "该界面的应用尚未集成，请自行添加", "取消", "确定");
    exitMessageBox->executeModal();
}
