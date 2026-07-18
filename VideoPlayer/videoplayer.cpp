#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QUrl>
#include <QListWidget>
#include <QIcon>
#include "videoplayer.h"
#include "ui_videoplayer.h"

VideoPlayer::VideoPlayer(QWidget *parent, VoiceControl *voiceControl)
    : QWidget(parent)
    , ui(new Ui::VideoPlayer)
    ,voiceControl(voiceControl)
{
    ui->setupUi(this);

    /* 手势 */
    gesture = new Gesture(ui->videoPlayerStackedWidget, this);
    gesture->addVerticalScrollWidget(ui->playListWidget);

    /* 初始化播放器 */
    mediaPlayer = new QMediaPlayer(this);
    mediaPlayerList = new QMediaPlaylist(this);
    videoWidget = new QVideoWidget(ui->videoWidget);
    videoWidget->setEnabled(false);

    /* 初始化播放器 */
    mediaPlayer->setPlaylist(mediaPlayerList);      /* 设置播放器的播放队列 */
    mediaPlayer->setVideoOutput(videoWidget);       /* 设置播放器的显示窗口 */
    videoWidget->resize(1024, 390);                 /* 设置视频播放尺寸，如果是PC，最好是自己控制videoWidget */
    videoWidget->installEventFilter(this);          /* PC的话，请自己写事件过滤器，应对全屏退出的问题 */
    ui->videoWidget->hide();                        /* 先让ui->videoWidget看不见，避免首次打开，点击其他控件有残留 */

    /* 引导页 */
    ui->videoPlayerStackedWidget->setCurrentIndex(0);
    firstEntryTimer = new QTimer(this);
    firstEntryTimer->setInterval(3000);
    connect(firstEntryTimer, SIGNAL(timeout()), this, SLOT(gotoCurrentPlayPage()));

    /* 默认让playListBackgroundWidget不可见 */
    ui->playListBackgroundWidget->setVisible(false);

    /* 全屏时事件 */
    setcontrolWidgetVisibleTimer = new QTimer(this);
    setcontrolWidgetVisibleTimer->setInterval(3000);
    connect(setcontrolWidgetVisibleTimer, SIGNAL(timeout()), this, SLOT(setcontrolWidgetVisible()));

    /* 播放状态 */
    connect(mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(updateControlPushButtonUI()));

    /* 进度相关 */
    ui->playHorizontalSlider->setPageStep(0);
    connect(mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
    connect(mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));

    /* 播放项改变 */
    connect(mediaPlayerList, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    /* 保存原始按钮样式 */
    originalPlayListPushButtonStyleSheet = ui->playListPushButton->styleSheet();
    originalPlaySpeedPushButtonStyleSheet = ui->playSpeedPushButton->styleSheet();
    originalVolumePushButtonStyleSheet = ui->volumePushButton->styleSheet();

    /* 按钮按下样式 */
    pushButtonPressedStyleSheet = R"(
    QPushButton#playListPushButton,
    QPushButton#volumePushButton,
    QPushButton#playSpeedPushButton {
        background-color: rgb(52, 130, 255);
        border: none;
        border-radius: 8px;
        color: white;
    }

    QPushButton#playListPushButton:pressed,
    QPushButton#volumePushButton:pressed,
    QPushButton#playSpeedPushButton:pressed {
        background-color: rgb(21, 101, 192);
        border: none;
        border-radius: 8px;
        color: white;
    }
    )";

    /* 设置播放顺序为顺序播放 */
    mediaPlayerList->setPlaybackMode(QMediaPlaylist::Sequential);

    /* 倍速相关（默认是1.0倍速） */
    ui->playSpeedWidget->setVisible(false);

    ui->speedPushButton1->setCheckable(true);   /* 设置按钮可选中 */
    ui->speedPushButton2->setCheckable(true);
    ui->speedPushButton3->setCheckable(true);
    ui->speedPushButton4->setCheckable(true);

    speedButtonGroup = new QButtonGroup(this);  /* 按钮组，保证只能选中一个 */
    speedButtonGroup->addButton(ui->speedPushButton1, 1);
    speedButtonGroup->addButton(ui->speedPushButton2, 2);
    speedButtonGroup->addButton(ui->speedPushButton3, 3);
    speedButtonGroup->addButton(ui->speedPushButton4, 4);
    speedButtonGroup->setExclusive(true);       /* 只能选中一个 */

    ui->speedPushButton2->setChecked(true); /* 默认选中1.0倍速按钮 */

    /* 倍速按钮样式 */
    speedButtonStyleSheet = R"(
        /* 默认状态：不选中 */
        QPushButton#speedPushButton1,
        QPushButton#speedPushButton2,
        QPushButton#speedPushButton3,
        QPushButton#speedPushButton4 {
            background-color: transparent;
            color: white;
            border-radius: 6px;
        }

        /* 按下状态 */
        QPushButton#speedPushButton1:pressed,
        QPushButton#speedPushButton2:pressed,
        QPushButton#speedPushButton3:pressed,
        QPushButton#speedPushButton4:pressed {
            background-color: rgb(47, 47, 47);
            color: white;
        }

        /* 选中状态 */
        QPushButton#speedPushButton1:checked,
        QPushButton#speedPushButton2:checked,
        QPushButton#speedPushButton3:checked,
        QPushButton#speedPushButton4:checked {
            background-color: rgba(170, 170, 170, 100);
            color: white;
        }
    )";

    /* 应用样式表 */
    ui->speedPushButton1->setStyleSheet(speedButtonStyleSheet);
    ui->speedPushButton2->setStyleSheet(speedButtonStyleSheet);
    ui->speedPushButton3->setStyleSheet(speedButtonStyleSheet);
    ui->speedPushButton4->setStyleSheet(speedButtonStyleSheet);

    connect(speedButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onSpeedButtonClicked(int)));

    /* 音量条默认不可见 */
    ui->volumeWidget->setVisible(false);

    /* 声音控制 */
    ui->volumeVerticalSlider->setPageStep(0);

}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

/* 开屏显示事件 */
void VideoPlayer::showEvent(QShowEvent *event)
{
    if (ui->videoPlayerStackedWidget->currentIndex() == 0 && isFirstEntry == true) {
        firstEntryTimer->start();
    }

    return QWidget::showEvent(event);
}

/* 跳转至播放主界面 */
void VideoPlayer::gotoCurrentPlayPage()
{
    ui->videoPlayerStackedWidget->setCurrentIndex(1);
    isFirstEntry = false;
    firstEntryTimer->stop();
    loadVideoFile(nullptr);
}

/* 加载视频文件夹路径 */
void VideoPlayer::loadVideoFile(QString dirPath)
{
    /* 1. 指定视频文件夹路径 */
    if (dirPath.isEmpty()) {
        dirPath = "/home/root/KaydonOS/video";
    }

    /* 2. 将目录的视频名字添加到一个临时的列表 */
    QDir dir(dirPath);

    QStringList filters;
    filters << "*.mp4" << "*.avi" << "*.mkv";
    QStringList fileNames = dir.entryList(filters, QDir::Files);

    /* 3. 先清空播放器的播放队列和UI的播放队列（防止上次残留） */
    mediaPlayerList->clear();
    ui->playListWidget->clear();

    /* 4.1 将视频名和视频路径遍历 */
    foreach (const QString &fileNamesStr, fileNames) {  /* foreach (const QString &元素, 容器) */
        QString filePath = dir.absoluteFilePath(fileNamesStr);
        //qDebug() << fileNamesStr;    /* 视频名 */
        //qDebug() << filePath;        /* 对应视频的路径 */

        /* 4.2 将对应的视频路径存入到url中 */
        QUrl fileUrl = QUrl::fromLocalFile(filePath);

        /* 4.3 将url载入到播放器的播放列表 */
        mediaPlayerList->addMedia(fileUrl);

        /* 4.4 将播放器的播放列表的项目展示到ui->playListView */
        ui->playListWidget->addItem(fileNamesStr);
    }

    /* 5. 默认选中第一项 */
    if (fileNames.size() > 0) {
        mediaPlayerList->setCurrentIndex(0);
        ui->playListWidget->setCurrentRow(0);
    }
}

/* 播放列表按钮点击事件 */
void VideoPlayer::on_playListPushButton_clicked()
{
    if (ui->playListBackgroundWidget->isVisible()) {
        ui->playListBackgroundWidget->setVisible(false);
        ui->playListPushButton->setStyleSheet(originalPlayListPushButtonStyleSheet);
    } else {
        ui->playListBackgroundWidget->setVisible(true);
        ui->playListPushButton->setStyleSheet(pushButtonPressedStyleSheet);
    }
}

/* 播放列表返回按钮点击事件 */
void VideoPlayer::on_backPushButton_clicked()
{
    ui->playListBackgroundWidget->setVisible(false);
    ui->playListPushButton->setStyleSheet(originalPlayListPushButtonStyleSheet);
}

/* 选择视频文件夹按钮点击事件 */
void VideoPlayer::on_chooseFilePathPushButton_clicked()
{
    /* 1. 创建 QFileDialog */
    QFileDialog *dialog = new QFileDialog(this, "选择文件夹", "/home/root/KaydonOS/video");

    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(QFileDialog::ShowDirsOnly, true);
    dialog->setOption(QFileDialog::DontUseNativeDialog, true);

    /* 2. 单独设置白色样式（避免在开发板上看不见） */
    dialog->setStyleSheet(R"(
        QFileDialog {
            background-color: white;
        }

        QWidget {
            background-color: white;
            color: black;
        }

        QLineEdit, QListView, QTreeView {
            background-color: white;
            color: black;
        }

        QPushButton {
            background-color: rgb(240, 240, 240);
            color: black;
            border: 1px solid gray;
            padding: 5px;
        }

        QPushButton:hover {
            background-color: rgb(220, 220, 220);
        }
    )");

    /* 3. 执行对话框 */
    if (dialog->exec() == QDialog::Accepted) {
        QString dirPath = dialog->selectedFiles().first();
        loadVideoFile(dirPath);
    }

    delete dialog;
}

/* 播放按钮点击事件 */
void VideoPlayer::on_controlPlayPushButton_clicked()
{
    /* 1. 判断是否有选中视频 */
    if (mediaPlayerList->isEmpty()) {
        return;
    }

    /* 2. 播放/暂停切换 */
    if ((mediaPlayer->state() == QMediaPlayer::PausedState) || (mediaPlayer->state() == QMediaPlayer::StoppedState)) {
        mediaPlayer->play();
    } else {
        mediaPlayer->pause();
    }

    /* 3. 让视频容器显示 */
    videoWidget->setEnabled(true);
    ui->videoWidget->show();
}

/* 播放下一个按钮点击事件 */
void VideoPlayer::on_nextPushButton_clicked()
{
    /* 1. 判断是否有选中视频 */
    if (mediaPlayerList->isEmpty()) {
        return;
    }

    /* 2. 暂停播放，判断：如果当前已经是最后一个，直接返回，不切换 */
    if (mediaPlayerList->currentIndex() == mediaPlayerList->mediaCount() - 1) { /* 索引从0开始，项目数等于索引数-1 */
        return;
    } else {
        mediaPlayer->pause();
    }

    /* 3. 再播放下一个 */
    mediaPlayerList->next();
    mediaPlayer->play();

    videoWidget->setEnabled(true);
}

/* 播放上一个按钮点击事件 */
void VideoPlayer::on_previousPushButton_clicked()
{
    /* 1. 判断是否有选中视频 */
    if (mediaPlayerList->isEmpty()) {
        return;
    }

    /* 2. 暂停播放，判断：如果当前已经是第一个，直接返回，不切换 */
    if (mediaPlayerList->currentIndex() == 0) {
        return;
    } else {
        mediaPlayer->pause();
    }

    /* 3. 再播放上一个 */
    mediaPlayerList->previous();
    mediaPlayer->play();

    videoWidget->setEnabled(true);
}

/* 更新播放按钮UI事件 */
void VideoPlayer::updateControlPushButtonUI()
{
    if ((mediaPlayer->state() == QMediaPlayer::PausedState) || (mediaPlayer->state() == QMediaPlayer::StoppedState)) {
        ui->controlPlayPushButton->setIcon(QIcon(":/Icons/videoPlayer/Icons/playVideo.png"));
    } else {
        ui->controlPlayPushButton->setIcon(QIcon(":/Icons/videoPlayer/Icons/pauseVideo.png"));
    }
}

/* 点击全屏显示按钮事件 */
void VideoPlayer::on_setFullScreenPushButton_clicked()
{
    if (!isFullScreen) {
        videoWidget->resize(1024, 470); /* 开发板 */
        ui->controlWidget->setVisible(false);
        isFullScreen = true;
        ui->setFullScreenPushButton->setIcon(QIcon(":/Icons/videoPlayer/Icons/fullscreenExit.png"));
    } else {
        videoWidget->resize(1024, 390);
        ui->controlWidget->setVisible(true);
        isFullScreen = false;
        ui->setFullScreenPushButton->setIcon(QIcon(":/Icons/videoPlayer/Icons/fullscreen.png"));
    }
}

/* 全屏状态下，鼠标点击事件 */
bool VideoPlayer::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == videoWidget) {   /* 非UI的videoWidget，而是插件 */
        if (event->type() == QEvent::MouseButtonPress) {
            if (isFullScreen == true) {
                ui->controlWidget->setVisible(true);
                setcontrolWidgetVisibleTimer->start();
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

/* 全屏状态下，控制栏显示事件 */
void VideoPlayer::setcontrolWidgetVisible()
{
    if (isFullScreen) {
        ui->controlWidget->setVisible(false);
    }

    setcontrolWidgetVisibleTimer->stop();
}

/* 进度变化事件 */
void VideoPlayer::onDurationChanged(qint64 duration)
{
    //qDebug() << "duration is" << duration;

    /* duration is 52416 秒数分钟数要求余，避免出现00:70:70 */
    qint64 totalSeconds = duration / 1000;

    qint64 seconds = totalSeconds % 60;
    qint64 minutes = (totalSeconds / 60) % 60;
    qint64 hours = totalSeconds / 60 / 60;

    QString secondsString;
    QString minutesString;
    QString hoursString;

    if (seconds < 10) {
        secondsString = "0" + QString::number(seconds);
    } else {
        secondsString = QString::number(seconds);
    }

    if (minutes < 10) {
        minutesString = "0" + QString::number(minutes);
    } else {
        minutesString = QString::number(minutes);
    }

    if (hours < 10) {
        hoursString = "0" + QString::number(hours);
    } else {
        hoursString = QString::number(hours);
    }

    /* 更新时间标签 */
    ui->totalTimeLabel->setText(hoursString + ":" + minutesString + ":" + secondsString);

    /* 设置进度条最大值 */
    ui->playHorizontalSlider->setMaximum(duration);
}

/* 播放进度改变事件 */
void VideoPlayer::onPositionChanged(qint64 position)
{
    //qDebug() << "position is" << position;

    /* duration is 52416 秒数分钟数要求余，避免出现00:70:70*/
    qint64 totalSeconds = position / 1000;

    qint64 seconds = totalSeconds % 60;
    qint64 minutes = (totalSeconds / 60) % 60;
    qint64 hours = totalSeconds / 60 / 60;

    QString secondsString;
    QString minutesString;
    QString hoursString;

    if (seconds < 10) {
        secondsString = "0" + QString::number(seconds);
    } else {
        secondsString = QString::number(seconds);
    }

    if (minutes < 10) {
        minutesString = "0" + QString::number(minutes);
    } else {
        minutesString = QString::number(minutes);
    }

    if (hours < 10) {
        hoursString = "0" + QString::number(hours);
    } else {
        hoursString = QString::number(hours);
    }

    /* 更新进度条 */
    ui->playHorizontalSlider->setValue(position);

    /* 更新时间标签 */
    ui->currentTimeLabel->setText(hoursString + ":" + minutesString + ":" + secondsString);

    /* 让视频容器显示（防止第一次没点击播放按钮，导致画面不显示） */
    ui->videoWidget->show();
}

/* 点击进度条事件 */
void VideoPlayer::on_playHorizontalSlider_sliderPressed()
{
    /* 暂停播放，避免卡顿 */
    if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
    }
}

/* 进度条移动事件 */
void VideoPlayer::on_playHorizontalSlider_sliderMoved(int position)
{
    /* 设置要移动的位置 */
    ui->playHorizontalSlider->setValue(position);

    /* 先不告诉MediaPlayer要移到这个位置，画面更新太频繁，松手才更新 */
    //mediaPlayer->setPosition((qint64)position);

    /* 更新进度 */
    onPositionChanged(position);
}

/* 进度条施放事件 */
void VideoPlayer::on_playHorizontalSlider_sliderReleased()
{
    /* 移动后松手，可以通知mediaPlayer，以便更新画面 */
    int position = ui->playHorizontalSlider->value();
    mediaPlayer->setPosition(position);

    /* 继续播放 */
    mediaPlayer->play();
}

/* 当前播放视频项改变事件 */
void VideoPlayer::onCurrentIndexChanged(int position)
{
    if (mediaPlayerList->isEmpty()) {
        return;
    }

    /* 当播放完毕后currentIndex会变成-1，故还需要防止越界，执行下面的操作 */
    if (position < 0) {
        return;
    }

    /* 更新UI列表的选中状态 */
    ui->playListWidget->setCurrentRow(position);
    ui->playListWidget->scrollToItem(ui->playListWidget->item(position), QAbstractItemView::PositionAtCenter);

    /* 获取当前的视频名 */
    QString videoName = ui->playListWidget->currentItem()->text();

    /* 更新currentPlayNameLabel */
    ui->currentPlayNameLabel->setText("当前正在播放：" + videoName);
}

/* 倍速按钮点击事件 */
void VideoPlayer::on_playSpeedPushButton_clicked()
{
    if (ui->playSpeedWidget->isVisible()) {
        ui->playSpeedWidget->setVisible(false);
        ui->playSpeedPushButton->setStyleSheet(originalPlaySpeedPushButtonStyleSheet);
    } else {
        ui->playSpeedWidget->setVisible(true);
        ui->playSpeedPushButton->setStyleSheet(pushButtonPressedStyleSheet);
    }
}

/* 倍速按钮选择事件 */
void VideoPlayer::onSpeedButtonClicked(int buttonID)
{
    double playSpeed = 1.0;

    switch (buttonID) {
        case 1: playSpeed = 0.5; break;
        case 2: playSpeed = 1.0; break;
        case 3: playSpeed = 1.5; break;
        case 4: playSpeed = 2.0; break;
    }

    mediaPlayer->setPlaybackRate(playSpeed);

    ui->playSpeedWidget->setVisible(false);
    ui->playSpeedPushButton->setStyleSheet(originalPlaySpeedPushButtonStyleSheet);  /* 恢复默认样式，避免残留 */

    /* QButtonGroup + setExclusive已经帮我们自动切换高亮，所以不需要手动去取消其他按钮的checked */
}

/* 音量调节按钮事件 */
void VideoPlayer::on_volumePushButton_clicked()
{
    if (ui->volumeWidget->isVisible()) {
        ui->volumeWidget->setVisible(false);
        ui->volumePushButton->setStyleSheet(originalVolumePushButtonStyleSheet);
    } else {
        ui->volumeWidget->setVisible(true);
        ui->volumePushButton->setStyleSheet(pushButtonPressedStyleSheet);
    }
}

/* 播放列表项目双击事件 */
void VideoPlayer::on_playListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    /* 1. 获取传入的项目序号 */
    int clickedItemIndex = ui->playListWidget->row(item);

    /* 2. 先暂停播放 */
    mediaPlayer->stop();

    /* 3. 设置播放队列 */
    mediaPlayerList->setCurrentIndex(clickedItemIndex);

    /* 4. 更新UI播放列表 */
    ui->playListWidget->setCurrentRow(clickedItemIndex);
    ui->playListWidget->scrollToItem(ui->playListWidget->item(clickedItemIndex), QAbstractItemView::PositionAtCenter);

    /* 5. 更新当前播放标签 */
    QString videoName = ui->playListWidget->currentItem()->text();
    ui->currentPlayNameLabel->setText(videoName);

    /* 6. 开始播放 */
    mediaPlayer->play();

    /* 7. 让视频容器显示（程序首次运行，为点击播放按钮时） */
    ui->videoWidget->show();
    videoWidget->setEnabled(true);
}

/* 音量条点击事件 */
void VideoPlayer::on_volumeVerticalSlider_sliderPressed()
{
    int position = ui->volumeVerticalSlider->value();    /* 获取当前音量 */
    voiceControl->lastVolume = position;                 /* 保存上次音量 */
}

/* 音量条释放事件 */
void VideoPlayer::on_volumeVerticalSlider_sliderReleased()
{
    int position = ui->volumeVerticalSlider->value();
    voiceControl->applyVolume(position);
    emit onVideoPlayerVolumeChanged(position);  /* 通知主窗口音量发生变化 */
}

/* 主窗口音量变化同步 */
void VideoPlayer::onMainWindowsVolumeChanged(int value)
{
    ui->volumeVerticalSlider->setValue(value);  /* 同步滑动条数值 */
}

/* 给外部提供暂停播放入口 */
void VideoPlayer::pauseVideo()
{
    if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
    }
}

/* 重置APP */
void VideoPlayer::resetAPP()
{
    /* 1. 停止播放 */
    mediaPlayer->stop();

    /* 2. 清空播放列表 */
    mediaPlayerList->clear();
    ui->playListWidget->clear();

    /* 3. 隐藏视频窗口 */
    ui->videoWidget->hide();
    videoWidget->setEnabled(false);

    /* 4. 回到引导页 */
    ui->videoPlayerStackedWidget->setCurrentIndex(0);
    isFirstEntry = true;

    /* 5. 重置播放按钮UI */
    ui->controlPlayPushButton->setIcon(QIcon(":/Icons/videoPlayer/Icons/playVideo.png"));

    /* 6. 重置进度条 */
    ui->playHorizontalSlider->setValue(0);
    ui->playHorizontalSlider->setMaximum(0);

    /* 7. 重置时间显示 */
    ui->currentTimeLabel->setText("00:00:00");
    ui->totalTimeLabel->setText("00:00:00");

    /* 8. 隐藏各种弹出控件 */
    ui->playListBackgroundWidget->setVisible(false);
    ui->playSpeedWidget->setVisible(false);
    ui->volumeWidget->setVisible(false);

    /* 9. 恢复按钮样式 */
    ui->playListPushButton->setStyleSheet(originalPlayListPushButtonStyleSheet);
    ui->playSpeedPushButton->setStyleSheet(originalPlaySpeedPushButtonStyleSheet);
    ui->volumePushButton->setStyleSheet(originalVolumePushButtonStyleSheet);

    /* 10. 重置倍速（默认1.0） */
    mediaPlayer->setPlaybackRate(1.0);
    ui->speedPushButton2->setChecked(true);

    /* 11. 退出全屏 */
    if (isFullScreen) {
        videoWidget->resize(1024, 390);
        ui->controlWidget->setVisible(true);
        isFullScreen = false;
        ui->setFullScreenPushButton->setIcon(QIcon(":/Icons/videoPlayer/Icons/fullscreen.png"));
    }

    /* 12. 当前播放名称清空 */
    ui->currentPlayNameLabel->setText("当前正在播放：");

    /* 13. 停止定时器 */
    firstEntryTimer->stop();
    setcontrolWidgetVisibleTimer->stop();
}
