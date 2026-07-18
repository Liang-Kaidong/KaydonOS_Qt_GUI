#ifndef SYSTEMSETTING_H
#define SYSTEMSETTING_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QMovie>
#include "gesture.h"
#include "ui_systemsetting.h"
#include "voicecontrol.h"
#include "brightnesscontrol.h"

namespace Ui {
class SystemSetting;
}

class SystemSetting : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSetting(QWidget *parent, VoiceControl *voiceControl);
    ~SystemSetting();

public slots:
    void resetPage();  /* 仅关闭时复位页面函数 */
    void onMainWindowsVolumeChanged(int value);

signals:
    void onSystemSettingVolumeChanged(int value);   /* 来自设置的音量大小变化信号 */

private slots:
    void on_aboutCarPushButton_clicked();           /* 处理 "关于汽车" 按钮点击事件 */
    void on_wlanPushButton_clicked();               /* 处理 "WLAN" 按钮点击事件 */

    void on_updatePushButton_clicked();
    void on_updatePageBackPushButton_clicked();

    void on_voiceControlSlider_sliderPressed();
    void on_voiceControlSlider_sliderReleased();    /* 松手播放提示音 */
    void on_voiceControlPushButton_clicked();
    void on_voiceSettingPushButton_clicked();

    void on_audioListWidget_itemSelectionChanged();

    void on_brightnessSettingPushButton_clicked();
    void on_autoBrightnessPushButton_clicked();
    void on_brightnessControlSlider_sliderReleased();
    void onBrightnessChanged(int value);

    void on_accountPushButton_clicked();
    void on_moreInfoPushButton_clicked();
    void on_cellularNetworkPushButton_clicked();
    void on_bluetoothPushButton_clicked();

private:
    Ui::SystemSetting *ui;
    Gesture *gesture;

    QPushButton *lastClickedButton = nullptr; /* 记录上一个点击的按钮（指针类型） */
    QMap<QPushButton*, QIcon> originalIcons;  /* 保存每个按钮的原始图标 */

    void resetLastButtonStyle();                            /* 恢复上一个按钮的样式 */
    void setCurrentButtonStyle(QPushButton *currentButton); /* 设置当前按钮样式并切换页面 */

    VoiceControl *voiceControl;

    QString currentAudioPath;   // 存储当前选中的音频文件路径
    bool isInitializingAudioList = false;

    BrightnessControl *brightnessControl;

    QMovie *gifShow;
};

#endif // SYSTEMSETTING_H
