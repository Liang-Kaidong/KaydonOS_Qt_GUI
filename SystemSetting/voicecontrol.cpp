#include "voicecontrol.h"
#include <QProcess>
#include <QRegExp>
#include <QDebug>
#include <QSettings>

VoiceControl::VoiceControl(QObject *parent)
    : QObject(parent)     /* QObject构造 */
{
    //qDebug() << "VoiceControl";

    /* 读取配置文件volume.ini */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/volume.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);

    /* 从配置文件读取音量和静音状态，如果没有配置项，使用默认值 */
    currentVolume = settings.value("Volume/currentVolume", 80).toInt();
    lastVolume = settings.value("Volume/lastVolume", 80).toInt();
}

/* 初始化音频通路 */
void VoiceControl::initAudioHardware()
{
    /**
     * 打开PCM（输出混音器）通路，否则DAC信号到不了喇叭
     * 等价于在终端输入amixer set "Left Output Mixer PCM" on
     * \是为了告诉C++：这是字符串的一部分，不是字符串结束符
     */
    QProcess::execute("amixer set \"Right Output Mixer PCM\" on");
    QProcess::execute("amixer set \"Left Output Mixer PCM\" on");
}

/* 设置音量接口（设置音量） */
void VoiceControl::applyVolume(int value)
{
    currentVolume = value;

    /**
     * 字符串拼接："amixer set Speaker 80%"
     * QString::arg() 命令是固定的，音量值是变化的
     * 音量是80，在终端会敲amixer set Speaker 80%
     * 假设是50，在终端会敲amixer set Speaker 50%
     * 唯一变化的是数字
     *
     * QString("... %1 ...").arg(value) 表示：用value替换%1
     * QString("amixer set Speaker %1%").arg(80);
     * 最终生成的字符串就是：amixer set Speaker 80%
     */
    QString cmd = QString("amixer set Speaker %1%").arg((value));

    /* 同步执行一条系统命令，并且等它执行完再继续往下走 */
    QProcess::execute(cmd);

    /* 发射音量同步信号 */
    emit volumeChanged(value);

    /* 将新的音量值写入配置文件 */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/volume.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.setValue("Volume/currentVolume", currentVolume);
    settings.setValue("Volume/lastVolume", lastVolume);        /* 保存上次音量 */

    qDebug() << "Updated volume settings:";
    qDebug() << "currentVolume:" << currentVolume;
    qDebug() << "lastVolume:" << lastVolume;
}


