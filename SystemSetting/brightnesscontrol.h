#ifndef BRIGHTNESSCONTROL_H
#define BRIGHTNESSCONTROL_H

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QFile>

class BrightnessControl : public QWidget
{
    Q_OBJECT
public:
    explicit BrightnessControl(QWidget *parent = nullptr);
    ~BrightnessControl();

    int getCurrentBrightness() const;   /* 获取当前亮度 */
    bool setBrightness(int brightness); /* 设置指定亮度，亮度值需要在0到最大亮度之间 */
    int brightnessValue;                /* 配置文件的亮度值 */

    void setAutoBrightnessEnabled(bool enabled); // 开启/关闭自动亮度
    bool autoBrightnessEnabled = false;          // 直接暴露自动亮度状态（无需冗余接口）
    void setManualOverride(bool override);       // 设置手动优先标记

signals:
    void brightnessChanged(int value);              // 亮度变化信号
    void autoBrightnessStateChanged(bool state);    // 自动亮度状态变化信号

private slots:
    void autoBrightnessUpdate();        // 自动亮度更新
    void smoothBrightnessStep();        // 平滑过渡单步

private:
    QString brightnessFile = "/sys/class/backlight/backlight/brightness";
    QString alsFile = "/sys/class/misc/ap3216c/als";
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/brightness.ini";

    QTimer *alsTimer = nullptr;
    QTimer *smoothBrightnessTimer = nullptr;

    bool manualBrightnessOverride = false;  // 手动优先标记
    int targetBrightness = 100;             // 目标亮度
    int currentBrightness = 100;            // 当前亮度

    int readAlsValue() const;
    int mapAlsToBrightness(int als) const;
    void saveAutoBrightnessState();         // 保存自动亮度状态到配置文件
};

#endif // BRIGHTNESSCONTROL_H
