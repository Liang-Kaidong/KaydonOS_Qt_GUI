#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>                          /* QWidget 基类 */
#include <QMediaPlayer>                     /* 音频播放器类 */
#include <QMediaPlaylist>                   /* 播放列表类 */
#include <QMediaMetaData>                   /* 音频元数据 */
#include <QDir>                             /* 目录操作 */
#include <QListWidgetItem>                  /* 列表项控件 */
#include "gesture.h"                        /* 页面切换类 */
#include "SystemSetting/voicecontrol.h"     /* 音量控制类 */

namespace Ui {
class MusicPlayer;
}

class MusicPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MusicPlayer(QWidget *parent, VoiceControl *voiceControl);  /* 共用音量控制 */
    void pauseMusic();      /* 给外部提供暂停播放入口 */
    ~MusicPlayer();

public slots:
    void onMainWindowsVolumeChanged(int value); /* 接收主窗口音量变化 */
    void resetPage();                           /* 重置页面 */

signals:
    void onMusicPlayerVolumeChanged(int value); /* 通知主窗口音量变化 */

private slots:
    void showEvent(QShowEvent *event);          /* 重写显示事件，引入欢迎页 */
    void gotoMainPage();                        /* 跳转主页面 */

    void on_playControlPushButton_clicked();    /* 播放按钮点击 */
    void on_musicListPushButton_clicked();      /* 打开播放列表 */
    void on_selectMusicPathPushButton_clicked();/* 打开播放文件夹 */

    void onPlayerStateChanged(QMediaPlayer::State playerState);   /* 播放状态变化 */

    void on_playSortPushButton_clicked();       /* 播放模式切换 */
    void on_musicListPushButton_2_clicked();    /* 返回主界面 */

    void onCurrentSongChanged(int newSongIndex);       /* 当前歌曲变化 */
    void onDurationChanged(qint64 duration);           /* 歌曲时长变化 */
    void onPositionChanged(qint64 position);           /* 播放进度变化 */

    void on_processBarHorizontalSlider_sliderPressed();             /* 进度条按下 */
    void on_processBarHorizontalSlider_sliderMoved(int position);   /* 拖动 */
    void on_processBarHorizontalSlider_sliderReleased();            /* 释放 */

    void onMetaDataChanged();                   /* 元数据变化 */

    void updateLyric(qint64 currentPositionMillisecond);    /* 更新歌词 */

    void on_previousPushButton_clicked();       /* 上一首 */
    void on_nextPushButton_clicked();           /* 下一首 */

    void on_musicListWidget_itemClicked(QListWidgetItem *item); /* 点击歌曲 */

    void on_voiceControlPushButton_clicked();               /* 音量图标点击 */
    void on_voiceControlHorizontalSlider_sliderReleased();  /* 音量释放 */
    void on_voiceControlHorizontalSlider_sliderPressed();   /* 音量按下 */

protected:
    bool eventFilter(QObject *watched, QEvent *event);      /* 事件过滤器，设置音量时，非音量条点击返回 */

private:
    Ui::MusicPlayer *ui;
    Gesture *gesture;               /* 页面切换控制器 */
    VoiceControl *voiceControl;     /* 音量控制对象 */
    QMediaPlayer *mediaPlayer;      /* 音频播放器 */
    QMediaPlaylist *mediaPlayList;  /* 播放列表 */
    QTimer *enterMainPageTimer;     /* 进入主页面计时器 */

    bool hasEnteredMainPage = false;    /* 是否首次进入 */
    bool isSliderPressed = false;       /* 是否拖动进度条 */

    int  playSort = 1;   /* 1:列表循环播放 2:单曲循环播放 3:列表随机播放 */

    void loadMusicFiles(const QString &musicDirectoryPath); /* 加载音乐文件 */

    QString formatTime(qint64 timeMs);  /* 时间格式化 */

    void loadLyric(const QString &musicFilePath);   /* 加载歌词 */
    QStringList lyricTextList;                      /* 歌词文本列表 */
    QList<qint64> lyricTimeList;                    /* 歌词时间列表 */
    int currentLyricIndex = -1;                     /* 当前歌词索引 */
};

#endif // MUSICPLAYER_H
