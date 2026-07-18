#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include "gesture.h"
#include "exitmessagebox.h"
#include "SystemSetting/voicecontrol.h"
#include "SystemSetting/systemsetting.h"
#include "MusicPlayer/musicplayer.h"
#include "Driver/DHT11/dht11driver.h"
#include "Weather/weather.h"
#include "Weather/weatherData.h"
#include "Clock/clock.h"
#include "VideoPlayer/videoplayer.h"
#include "Camera/camera.h"
#include "Monitor/monitor.h"
#include "Caculator/caculator.h"
#include "Gallery/gallery.h"
#include "Calendar/calendar.h"
#include "Recorder/recorder.h"
#include "PerformanceTool/performancetool.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString currentRunningAppName();               /*  提供对外的API接口查询当前应用 */

public slots:
    void onSystemSettingVolumeChanged(int value);  /*  收到来自设置的音量大小变化信号 */
    void onMusicPlayerVolumeChanged(int value);    /*  收到来自音乐播放器的音量大小变化信号 */
    void onVideoPlayerVolumeChanged(int value);    /*  收到来自视频播放器的音量大小变化信号 */

signals:
    void onMainWindowsVolumeChanged(int value);    /* 发射导航栏的音量大小变化信号 */

private slots:
    void updatePageIndicator(int currentIndex);

    void on_backDesktopButton_clicked();

    void updateCurrentTime();

    void on_voiceControlSlider_sliderReleased();    /* 松手播放提示音 */

    void on_voiceControlPushButton_clicked();
    void on_exitAppPushButton_clicked();
    void on_settingButton_clicked();
    void on_systemSettingPushButton_clicked();
    void on_allAppsPushButton_clicked();
    void on_musicPlayerPushButton_clicked();
    void on_downTemperaturePushButton_clicked();
    void on_riseTemperaturePushButton_clicked();

    void on_voiceControlSlider_sliderPressed();

    void onDHT11Timeout();

    void on_weatherPushButton_clicked();
    void on_clockPushButton_clicked();
    void on_videoPlayerPushButton_clicked();
    void on_musicPlayerPushButton2_clicked();
    void on_cameraPushButton_clicked();

    void onCameraAPPOpen();
    void onMonitorAPPOpen();

    void on_monitorPushButton_clicked();
    void on_calculatorPushButton_clicked();
    void on_videoPlayerPushButton2_clicked();
    void on_galleryPushButton_clicked();
    void on_calendarPushButton_clicked();
    void on_recorderPushButton_clicked();
    void on_performanceToolPushButton_clicked();

    void on_updPushButton_clicked();

private:
    Ui::MainWindow *ui;
    SystemSetting *systemSettingPage;
    MusicPlayer *musicPlayerPage;
    Gesture *gesture;
    QTimer *timeTimer;
    Weather *weatherPage;
    Clock *clockPage;
    VideoPlayer *videoPlayerPage;
    Camera *cameraPage;
    Monitor *monitorPage;
    Caculator *caculatorPage;
    Gallery *galleryPage;
    Calendar *calendarPage;
    Recorder *recorderPage;
    PerformanceTool *performanceToolPage;

    /* 辅助判断：是否为桌面页 */
    bool isDesktopPage();

    ExitMessageBox *exitMessageBox;
    VoiceControl *voiceControl;

    DHT11Driver *dht11Driver;
    QTimer *dht11Timer;

    QTimer *openCameraTimer;
    QTimer *openMonitorTimer;
};

#endif // MAINWINDOW_H
