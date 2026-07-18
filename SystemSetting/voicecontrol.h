#ifndef VOICECONTROL_H
#define VOICECONTROL_H

#include <QObject>
#include <QSettings>

class VoiceControl : public QObject
{
    Q_OBJECT
public:
    explicit VoiceControl(QObject *parent = nullptr);

    void initAudioHardware();       /* 初始化音频通路 */
    void applyVolume(int value);    /* 真正应用音量（调用 amixer） */
    int  currentVolume;             /* 记录当前的音量 */
    int  lastVolume;                /* 记录静音前音量 */

signals:
    void volumeChanged(int value);  /* 音量变化信号 */ 
};

#endif // VOICECONTROL_H
