#include "brightnesscontrol.h"
#include <QFile>
#include <QTextStream>  /* 用于文本检索 */
#include <QDebug>
#include <QSettings>

BrightnessControl::BrightnessControl(QWidget *parent)
    : QWidget(parent)
{
    //qDebug() << "brightnessControl init";

    /* 1. 读取配置文件（亮度值 + 自动亮度状态） */
    QSettings settings(configFilePath, QSettings::IniFormat);

    /* 1.1 读取手动亮度值 */
    brightnessValue = settings.value("brightness/brightnessValue", 100).toInt();
    currentBrightness = brightnessValue;
    setBrightness(brightnessValue);
    //qDebug() << "已加载配置亮度值： " << brightnessValue;

    /* 1.2 读取自动亮度状态（核心优化：从配置文件初始化） */
    autoBrightnessEnabled = settings.value("autoBrightness/isAutoBrightness", false).toBool();
    //qDebug() << "已加载自动亮度状态： " << autoBrightnessEnabled;

    /* 2. 初始化定时器 */
    alsTimer = new QTimer(this);
    connect(alsTimer, SIGNAL(timeout()), this, SLOT(autoBrightnessUpdate()));
    alsTimer->setInterval(1500);

    smoothBrightnessTimer = new QTimer(this);
    connect(smoothBrightnessTimer, SIGNAL(timeout()), this, SLOT(smoothBrightnessStep()));
    smoothBrightnessTimer->setInterval(30);

    /* 3.启动自动亮度（依照配置文件） */
    if (autoBrightnessEnabled) {
        if (readAlsValue() >= 0) {
            alsTimer->start();
            manualBrightnessOverride = false;
            qDebug() << "自动亮度已从配置启动";
            emit autoBrightnessStateChanged(true);
        } else {
            autoBrightnessEnabled = false;
            saveAutoBrightnessState();
            emit autoBrightnessStateChanged(false);
            qWarning() << "ALS硬件异常，自动亮度启动失败";
        }
    }
}

BrightnessControl::~BrightnessControl()
{
    delete alsTimer;
    delete smoothBrightnessTimer;
    saveAutoBrightnessState();
}

/* 获取当前的亮度值（实际板子返回值0-7） */
int BrightnessControl::getCurrentBrightness() const
{
    QFile file(brightnessFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        return in.readLine().toUInt();
    }
    qWarning() << "无法读取当前亮度值";
    return -1;
}

/* 设置亮度值 */
bool BrightnessControl::setBrightness(int brightnessValue)
{
    int brightness = 7;

    if (brightnessValue == 0) {
        brightness = 0;
    } else if (brightnessValue <= 14) {
        brightness = 1;
    } else if (brightnessValue <= 28) {
        brightness = 2;
    } else if (brightnessValue <= 43) {
        brightness = 3;
    } else if (brightnessValue <= 57) {
        brightness = 4;
    } else if (brightnessValue <= 71) {
        brightness = 5;
    } else if (brightnessValue <= 85) {
        brightness = 6;
    } else {
        brightness = 7;
    }

    QFile file(brightnessFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << brightness;  /* 设置亮度值 */
        //qDebug() << "亮度值： " << brightnessValue;
        //qDebug() << "映射亮度值： " << brightness;

        emit brightnessChanged(brightnessValue);
        return true;
    }

    qWarning() << "无法设置亮度";
    return false;
}

void BrightnessControl::saveAutoBrightnessState()
{
    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.setValue("autoBrightness/isAutoBrightness", autoBrightnessEnabled);
    qDebug() << "已保存自动亮度状态到配置：" << autoBrightnessEnabled;
}

void BrightnessControl::setAutoBrightnessEnabled(bool enabled)
{
    if (autoBrightnessEnabled == enabled) return;

    autoBrightnessEnabled = enabled;

    if (enabled) {
        if (readAlsValue() < 0) {
            qWarning() << "自动亮度硬件异常，无法开启";
            autoBrightnessEnabled = false;
            saveAutoBrightnessState();
            emit autoBrightnessStateChanged(false);
            return;
        }
        alsTimer->start();
        manualBrightnessOverride = false;
        //qDebug() << "自动亮度开启";
    } else {
        alsTimer->stop();
        smoothBrightnessTimer->stop();
        //qDebug() << "自动亮度关闭";
    }

    saveAutoBrightnessState();
    emit autoBrightnessStateChanged(autoBrightnessEnabled);
}

void BrightnessControl::setManualOverride(bool override)
{
    manualBrightnessOverride = override;
    if (override) {
        smoothBrightnessTimer->stop();
        currentBrightness = brightnessValue;
    }
    //qDebug() << "手动优先标记：" << override;
}

int BrightnessControl::readAlsValue() const
{
    QFile file(alsFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开ALS设备节点";
        return -1;
    }

    int value = file.readLine().trimmed().toInt();
    file.close();
    return value;
}

int BrightnessControl::mapAlsToBrightness(int als) const
{
    if (als <= 10) return 1;
    if (als <= 80) {
        double ratio = (double)(als - 10) / 70;
        return 1 + ratio * 74;
    }
    if (als <= 100) {
        double ratio = (double)(als - 80) / 20;
        return 75 + ratio * 25;
    }
    return 100;
}

void BrightnessControl::autoBrightnessUpdate()
{
    if (!autoBrightnessEnabled || manualBrightnessOverride) return;

    int alsValue = readAlsValue();
    if (alsValue < 0) return;

    targetBrightness = mapAlsToBrightness(alsValue);
    if (currentBrightness != targetBrightness) {
        smoothBrightnessTimer->start();
    }

    //qDebug() << "ALS:" << alsValue << " 目标亮度:" << targetBrightness << " 当前亮度:" << currentBrightness;
}

void BrightnessControl::smoothBrightnessStep()
{
    int step = 0;
    if (currentBrightness < targetBrightness) {
        step = qMax(1, (targetBrightness - currentBrightness) / 5);
        currentBrightness = qMin(currentBrightness + step, targetBrightness);
    } else if (currentBrightness > targetBrightness) {
        step = qMax(1, (currentBrightness - targetBrightness) / 5);
        currentBrightness = qMax(currentBrightness - step, targetBrightness);
    }

    setBrightness(currentBrightness);

    if (currentBrightness == targetBrightness) {
        smoothBrightnessTimer->stop();
    }
}





