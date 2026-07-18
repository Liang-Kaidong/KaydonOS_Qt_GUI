#ifndef RECORDER_H
#define RECORDER_H

#include "gesture.h"
#include <QWidget>
#include <QMovie>
#include <QAudioRecorder>
#include <QMediaPlayer>
#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class Recorder; }
QT_END_NAMESPACE

class Recorder : public QWidget
{
    Q_OBJECT
public:
    Recorder(QWidget *parent = nullptr);
    ~Recorder();

    void closeRecorder();

private slots:
    void on_controlPushButton_clicked();
    void playRecording();
    void deleteRecording();
    void updateRecordTime();

private:
    Ui::Recorder *ui;

    void initAudioInput();          // 初始化音频通道
    QAudioRecorder *audioRecorder;
    QMediaPlayer *player;
    QString saveFilePath;           // 录音文件保存路径
    void loadRecordingList();       // 加载录音文件列表
    bool isRecording = false;       // 录音开始标志
    QTimer *recordTimer;
    int recordSeconds = 0;

    QMovie *movie;                  // GIF播放需要

    Gesture *gesture;               // 自定义手势类
};
#endif // RECORDER_H
