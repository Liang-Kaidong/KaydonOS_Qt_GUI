#include "musicplayer.h"
#include "ui_musicplayer.h"
#include <QTimer>        /* 定时器 */
#include <QMouseEvent>   /* 鼠标事件 */
#include <QDir>          /* 目录操作 */
#include <QFileDialog>   /* 选择文件夹操作 */
#include <QFileInfo>     /* 文件信息 */
#include <QTextStream>   /* 文本流 */
#include <QPainterPath>  /* 圆角路径 */
#include <QMediaContent> /* 媒体内容 */

MusicPlayer::MusicPlayer(QWidget *parent, VoiceControl *voiceControl) :
    QWidget(parent),
    ui(new Ui::MusicPlayer),
    voiceControl(voiceControl)
{
    ui->setupUi(this);
    gesture = new Gesture(ui->musicPlayerStackedWidget, this);      /* 创建页面切换控制器 */
    ui->musicPlayerStackedWidget->setCurrentIndex(0);               /* 默认显示欢迎页 */
    gesture->addVerticalScrollWidget(ui->musicListWidget);          /* 添加滚动支持 */

    /* 欢迎页引导 */
    enterMainPageTimer = new QTimer(this);
    enterMainPageTimer->setSingleShot(true);    /* 单次触发，避免重复执行。 */
    connect(enterMainPageTimer, SIGNAL(timeout()), this, SLOT(gotoMainPage()));

    /* 创建播放器 */
    mediaPlayer = new QMediaPlayer(this);
    mediaPlayList = new QMediaPlaylist(this);
    mediaPlayer->setPlaylist(mediaPlayList);    /* 绑定播放列表 */

    /* 默认循环播放 */
    mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);

    /* 初始化目录路径加载音乐 */
    QString directory = "/home/root/KaydonOS/music";
    loadMusicFiles(directory);

    ui->endTimeLabel->setText("00:00");             /* 初始化结束时间 */
    ui->currentTimeLabel->setText("00:00");         /* 初始化当前时间 */
    ui->processBarHorizontalSlider->setValue(0);    /* 进度条归零 */
    ui->processBarHorizontalSlider->setPageStep(0); /* 禁止点击跳跃 */

    /* 进度条信号处理 */
    connect(mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));   /* 时长变化信号 */
    connect(mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));   /* 播放位置变化信号 */

    /* 歌曲发生变化 */
    connect(mediaPlayList, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentSongChanged(int)));    /* 歌曲变化信号 */
    connect(mediaPlayer, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));                   /* 元数据变化信号 */

    /* 播放状态 */
    connect(mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onPlayerStateChanged(QMediaPlayer::State)));

    /* 音量条 */
    ui->volumeWidget->hide();                           /* 默认隐藏音量面板 */
    ui->mainPage->installEventFilter(this);             /* 安装事件过滤器 */
    ui->voiceControlHorizontalSlider->setValue(voiceControl->currentVolume);    /* 设置初始音量 */
    ui->voiceControlHorizontalSlider->setPageStep(0);   /* 禁止点击跳跃 */
}

MusicPlayer::~MusicPlayer()
{
    delete ui;
}

/* 显示事件 */
void MusicPlayer::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    /* 首次进入播放器 */
    if (!hasEnteredMainPage)
    {
        ui->musicPlayerStackedWidget->setCurrentIndex(0);

        enterMainPageTimer->start(2000);

        hasEnteredMainPage = true;
    }
}

/* 进入主界面 */
void MusicPlayer::gotoMainPage()
{
    ui->musicPlayerStackedWidget->setCurrentIndex(1);
    //qDebug() << "MusicPlayer has been successfully loaded";
}

/* 点击到选择音乐文件夹 */
void MusicPlayer::on_selectMusicPathPushButton_clicked()
{
    /* 1. 创建 QFileDialog */
    QFileDialog *dialog = new QFileDialog(this, "选择文件夹", "/home/root/KaydonOS/music");

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
        /* 加载该路径下的音乐文件 */
        loadMusicFiles(dirPath);
    }

    delete dialog;
}

/* 加载音乐文件 */
void MusicPlayer::loadMusicFiles(const QString &musicDirectoryPath)
{
    QDir musicDirectory(musicDirectoryPath);

    if (!musicDirectory.exists()) {
        qDebug() << "MusicDirectory is not exists";
        return;
    }

    /* 清空播放列表和列表控件 */
    mediaPlayList->clear();   /* 清空播放列表 */
    ui->musicListWidget->clear();  /* 清空列表控件 */
    ui->currentPlayLabel->setText("音乐播放器"); /* 清空歌曲标题 */

    /* 清空歌词 */
    ui->musicLyricTextBrowser->setText("暂无歌词");
    lyricTimeList.clear();
    lyricTextList.clear();
    currentLyricIndex = -1;

    /* 进度条归零 */
    ui->processBarHorizontalSlider->setValue(0);
    ui->currentTimeLabel->setText("00:00");
    ui->endTimeLabel->setText("00:00");
    mediaPlayer->setPosition(0);

    /* 清除封面 */
    ui->albumCoverLabel->clear();

    /* 设置过滤条件 */
    QStringList fileFilters;
    fileFilters << "*.mp3" << "*.wav" << "*.ogg";

    /* 获取文件列表 */
    QFileInfoList fileInfoList = musicDirectory.entryInfoList(fileFilters, QDir::Files);

    /* 如果文件夹没有音乐文件 */
    if (fileInfoList.isEmpty()) {
        ui->musicListWidget->addItem("该文件夹下没有音乐文件，请重新选择");  // 提示没有音乐文件
        qDebug() << "No music files found in this folder";
        return;
    }

    /* 为每个文件添加项 */
    for (int i = 0; i < fileInfoList.size(); i++) {
        QFileInfo fileInfo = fileInfoList.at(i);

        /* 创建一个新的QListWidgetItem */
        QListWidgetItem *item = new QListWidgetItem(fileInfo.fileName(), ui->musicListWidget);
        item->setData(Qt::UserRole, fileInfo.absoluteFilePath());

        /* 将音乐文件添加到播放列表 */
        mediaPlayList->addMedia(QMediaContent(QUrl::fromLocalFile(fileInfo.absoluteFilePath())));
    }

    /* 设置为第一首歌曲选中 */
    if (ui->musicListWidget->count() > 0) {
        ui->musicListWidget->setCurrentRow(0);
        ui->musicListWidget->scrollToItem(ui->musicListWidget->item(0), QAbstractItemView::PositionAtCenter);
    }

    /* 更新UI或者进行任何其他更新 */
    qDebug() << "Music files loaded successfully";
}

/* 播放器状态改变 */
void MusicPlayer::onPlayerStateChanged(QMediaPlayer::State playerState)
{
    int playState = 0;  /* 播放按钮状态变量 */
    if (playerState  == QMediaPlayer::PlayingState) {   /* 判断当前是否正在播放 */
        playState = 1;  /* 播放状态，显示暂停图标 */
    } else {
        playState = 2;  /* 暂停状态，显示播放图标 */
    }
    ui->playControlPushButton->setProperty("playState", playState);             /* 设置QSS动态属性 */
    ui->playControlPushButton->setStyle(ui->playControlPushButton->style());    /* 强制刷新样式 */
    //qDebug() << "playState: " << playState;
}

/* 点击到播放按钮 */
void MusicPlayer::on_playControlPushButton_clicked()
{
    int currentSongIndex = mediaPlayList->currentIndex();   /* 获取当前播放歌曲索引 */

    /* 如果还没有选中歌曲，但列表里有歌 */
    if (currentSongIndex < 0 && mediaPlayList->mediaCount() > 0) {
        mediaPlayList->setCurrentIndex(0);
        mediaPlayer->play();
        return;
    }

    if (mediaPlayer->state() == QMediaPlayer::PlayingState) /* 如果当前正在播放 */
    {
        mediaPlayer->pause();   /* 切换为暂停状态 */
    } else {
        mediaPlayer->play();    /* 切换为播放状态 */
    }
}

/* 点击到播放列表按钮 */
void MusicPlayer::on_musicListPushButton_clicked()
{
    ui->musicPlayerStackedWidget->setCurrentIndex(2);       /* 切换播放列表页面 */

    int currentPlayIndex  = mediaPlayList->currentIndex();  /* 获取当前正在播放的歌曲索引 */

    /* 当前播放索引合法且列表控件中存在该索引项 */
    if (currentPlayIndex  >= 0 && ui->musicListWidget->count() > currentPlayIndex)
    {
        ui->musicListWidget->setCurrentRow(currentPlayIndex);  /* 在列表中高亮当前播放歌曲 */
        ui->musicListWidget->scrollToItem(ui->musicListWidget->item(currentPlayIndex), QAbstractItemView::PositionAtCenter);   /* 滚动列表，使当前歌曲居中显示 */
    }
}

/* 点击到播放状态按钮 */
void MusicPlayer::on_playSortPushButton_clicked()
{
    /* 1:列表循环播放 2:单曲循环播放 3:列表随机播放 */
    if (playSort == 1) {        /* 将列表循环播放 转变成 单曲循环播放 */
        mediaPlayList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        playSort = 2;
    } else if (playSort == 2) { /* 将单曲循环播放 转变成 列表随机播放 */
        mediaPlayList->setPlaybackMode(QMediaPlaylist::Random);
        playSort = 3;
    } else {                    /* 将列表随机播放 转变成 列表循环播放 */
        mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);
        playSort = 1;
    }

    ui->playSortPushButton->setProperty("playSort", playSort);
    ui->playSortPushButton->setStyle(ui->playSortPushButton->style());
    //qDebug() << "The new playsort is " << mediaPlayList->playbackMode();
}

/* 处于播放列表界面再次点击播放列表按钮->返回主界面 */
void MusicPlayer::on_musicListPushButton_2_clicked()
{
    ui->musicPlayerStackedWidget->setCurrentIndex(1);
}

/* 播放列表项目被点击 */
void MusicPlayer::on_musicListWidget_itemClicked(QListWidgetItem *item)
{
    int clickedItemIndex = ui->musicListWidget->row(item);   /* 获取被点击歌曲的索引 */
    mediaPlayList->setCurrentIndex(clickedItemIndex);        /* 设置当前播放的歌曲 */
    mediaPlayer->play();

    ui->musicPlayerStackedWidget->setCurrentIndex(1);

    /* 获取当前歌曲名称 */
    QString selectedSongName = item->text();
    ui->currentPlayLabel->setText(selectedSongName);
}

/* 歌曲时长发生变化 */
void MusicPlayer::onDurationChanged(qint64 duration)
{
    ui->processBarHorizontalSlider->setMaximum((int)duration);  /* 设置进度条最大值 */
    ui->endTimeLabel->setText(formatTime(duration));            /* 更新结束时间显示 */
}

/* 播放进度发生变化 */
void MusicPlayer::onPositionChanged(qint64 position)
{
    ui->processBarHorizontalSlider->setValue((int)position);    /* 更新进度条当前位置 */
    ui->currentTimeLabel->setText(formatTime(position));        /* 更新时间显示 */

    if (isSliderPressed == false) {     /* 如果当前未拖动滑块，防止拖动更新造成卡顿 */
        updateLyric(position);          /* 更新歌词显示 */
    }
}

/* 当前歌曲发生改变 */
void MusicPlayer::onCurrentSongChanged(int newSongIndex)
{
    if (newSongIndex < 0) {
        return;
    }

    /* 同步列表选中 */
    ui->musicListWidget->setCurrentRow(newSongIndex);
    ui->musicListWidget->scrollToItem(ui->musicListWidget->item(newSongIndex), QAbstractItemView::PositionAtCenter);

    /* 获取歌曲名称 */
    QString currentSongName = ui->musicListWidget->item(newSongIndex)->text();
    ui->currentPlayLabel->setText(currentSongName);

    /* 获取当前歌词路径 */
    QString musicLyricFilePath = mediaPlayList->currentMedia().canonicalUrl().toLocalFile();

    /* 加载歌词 */
    loadLyric(musicLyricFilePath);

    //qDebug() << currentSongName << "is playing now";
}

/* 歌曲时长显示 */
QString MusicPlayer::formatTime(qint64 timeMs)
{
    int totalSeconds = timeMs / 1000;   /* 转换为秒 */
    int minutes = totalSeconds / 60;    /* 计算分钟 */
    int seconds = totalSeconds % 60;    /* 计算秒数 */

    QString minutesString;
    QString secondsString;

    if (minutes < 10) { /* 不足两位补零 */
        minutesString = "0" + QString::number(minutes);
    } else {
        minutesString = QString::number(minutes);
    }

    if (seconds < 10) { /* 不足两位补零 */
        secondsString = "0" + QString::number(seconds);
    } else {
        secondsString = QString::number(seconds);
    }
    return minutesString + ":" + secondsString;
}

/* 进度条按下 */
void MusicPlayer::on_processBarHorizontalSlider_sliderPressed()
{
    isSliderPressed = true; /* 标记滑块正在拖动，通知歌词不更新 */
    if (mediaPlayer->state() == mediaPlayer->PlayingState) {
        mediaPlayer->pause();
    }
}

/* 进度条移动 */
void MusicPlayer::on_processBarHorizontalSlider_sliderMoved(int position)
{
    ui->processBarHorizontalSlider->setValue((int)position);    /* 实时更新进度条值 */
    mediaPlayer->setPosition((qint64)position);                 /* 设置播放器位置 */
    ui->currentTimeLabel->setText(formatTime(position));        /* 更新时间显示 */
}

/* 进度条释放 */
void MusicPlayer::on_processBarHorizontalSlider_sliderReleased()
{
    isSliderPressed = false;    /* 取消拖动标记，通知可以更新歌词 */
    if (mediaPlayer->state() == mediaPlayer->PausedState) {
        mediaPlayer->play();
    }

    updateLyric(mediaPlayer->position());   /* 更新歌词位置 */
}

/* 媒体元数据发生变化时更新封面 */
void MusicPlayer::onMetaDataChanged()
{
    if (!mediaPlayer->isMetaDataAvailable()) {
        return;
    }

    QImage albumCoverImage;     /* 用于存储封面图片 */

    /* 第一优先：Qt Metadata（PC+部分Linux）*/
    QVariant coverData = mediaPlayer->metaData(QMediaMetaData::CoverArtImage);  /* 获取封面元数据 */

    if (coverData.isValid()) {  /* 如果数据有效 */
        albumCoverImage = coverData.value<QImage>();    /* 转换为QImage */
    }

    /* 第二优先：如果PC metadata没读到，尝试BSP metadata */
    if (albumCoverImage.isNull()) {
        /* 尝试备用字段 */
        QVariant fallbackData = mediaPlayer->metaData("ThumbnailImage");

        if (fallbackData.isValid()) {
            albumCoverImage = fallbackData.value<QImage>();
        }
    }

    /* 如果仍然没有封面 */
    if (albumCoverImage.isNull()) {
        ui->albumCoverLabel->clear();   /* 清除封面显示 */
        //qDebug() << "Albumcover failed to load";
        return;
    }

    /* 圆角遮罩（只创建一次，避免嵌入式性能损耗） */
    static QRegion roundedRegion;

    if (roundedRegion.isEmpty()) {
        QPainterPath path;
        path.addRoundedRect(ui->albumCoverLabel->rect(), 40, 40);   /* 添加圆角矩形 */
        roundedRegion = QRegion(path.toFillPolygon().toPolygon());  /* 创建区域遮罩 */
    }
    ui->albumCoverLabel->setMask(roundedRegion);

    ui->albumCoverLabel->setPixmap(      /* 设置缩放后的封面图 */
        QPixmap::fromImage(albumCoverImage).scaled(
            ui->albumCoverLabel->size(),
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation));

    //qDebug() << "Albumcover has been successfully loaded";
}

/* 歌词加载 */
void MusicPlayer::loadLyric(const QString &musicFilePath)
{
    ui->musicLyricTextBrowser->clear();     /* 清空歌词显示区域 */
    lyricTimeList.clear();                  /* 清空时间列表 */
    lyricTextList.clear();                  /* 清空歌词文本列表 */
    currentLyricIndex = -1;                 /* 重置当前歌词索引 */

    QFileInfo musicFileInfo(musicFilePath); /* 获取音乐文件信息 */

    /* 拼接歌词文件路径 */
    QString lyricFilePath =
            musicFileInfo.path() + "/" +
            musicFileInfo.completeBaseName() + ".lrc";

    QFile lyricFile(lyricFilePath);         /* 创建歌词文件对象 */

    /* 如果歌词文件不存在 */
    if (!lyricFile.exists())
    {
        ui->musicLyricTextBrowser->setText("暂无歌词");
        qDebug() << "There is no lyric";
        return;
    }

    /* 如果文件打开失败 */
    if (!lyricFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->musicLyricTextBrowser->setText("歌词文件打开失败");
        qDebug() << "LyricFile open failed";
        return;
    }

    QTextStream textStream(&lyricFile); /* 创建文本流对象 */
    textStream.setCodec("UTF-8");       /* 设置编码格式 */

    while (!textStream.atEnd())         /* 循环读取文件 */
    {
        QString lineText = textStream.readLine();   /* 读取一行文本 */

        /* [00:00]XXXXX */
        int leftBracket = lineText.indexOf("[");    /* 查找左括号 */
        int rightBracket = lineText.indexOf("]");   /* 查找右括号 */

        /* 如果时间标签存在 */
        if (leftBracket != -1 && rightBracket != -1)
        {
            /* 提取时间字符串 */
            QString timeText = lineText.mid(leftBracket + 1, rightBracket - leftBracket - 1);

            /* 提取歌词文本 */
            QString lyricText = lineText.mid(rightBracket + 1);

            /* 按冒号分割 */
            QStringList minuteSecondList = timeText.split(":");

            if (minuteSecondList.size() == 2) {
                int minutes = minuteSecondList.at(0).toInt();       /* 分钟数 */
                QStringList secondMillisecondList = minuteSecondList.at(1).split(".");  /* 按小数点分割 */

                int seconds = secondMillisecondList.at(0).toInt();  /* 秒数 */

                int millisecond = 0;    /* 毫秒值初始化 */
                if (secondMillisecondList.size() > 1) {                 /* 如果存在毫秒 */
                    millisecond = secondMillisecondList.at(1).toInt();  /* 读取毫秒 */
                }

                /* 计算总毫秒 */
                qint64 totalTime = minutes * 60 * 1000 +
                                   seconds * 1000 +
                                   millisecond *10;
                lyricTimeList.append(totalTime);    /* 保存时间 */
                lyricTextList.append(lyricText);    /* 保存歌词文本 */
            }
        }
    }

    lyricFile.close();  /*关闭文件 */
    //qDebug() << "lyricFile has been successfully loaded";
}

/* 根据播放位置更新歌词 */
void MusicPlayer::updateLyric(qint64 currentPositionMillisecond)
{
    if (lyricTimeList.isEmpty()) {
        return;
    }

    int newLyricIndex  = 0;     /* 初始化索引 */

    for (int i = lyricTimeList.size() - 1; i >= 0; i--) {   /* 倒序查找 */
        if (currentPositionMillisecond >= lyricTimeList.at(i)) {   /* 找到当前时间段 */
            newLyricIndex = i;  /* 记录索引 */
            break;
        }
    }

    /* 如果未变化 */
    if (newLyricIndex == currentLyricIndex) {
        return;
    }

    /* 更新当前索引 */
    currentLyricIndex = newLyricIndex;

    /* 固定显示5行，当前歌词在第3行 */
    ui->musicLyricTextBrowser->clear();         /* 清空显示 */

    int startIndex = currentLyricIndex - 2;     /* 第三行居中 */

    /* 防止越界 */
    if (startIndex < 0) {
        startIndex = 0;
    }

    int endIndex = startIndex + 5;   /* 一共显示5行 */

    /* 防止越界 */
    if (endIndex > lyricTextList.size()) {
        endIndex = lyricTextList.size();
    }

    /* 遍历显示 */
    for (int i = startIndex; i < endIndex; i++) {
        if (i == currentLyricIndex) {   /* 当前歌词 */
            ui->musicLyricTextBrowser->setFontPointSize(28);
            ui->musicLyricTextBrowser->setTextColor(QColor("#FF4081")); /* 设置高亮颜色 */
        } else {
            ui->musicLyricTextBrowser->setFontPointSize(18);
            ui->musicLyricTextBrowser->setTextColor(Qt::white);
        }

        ui->musicLyricTextBrowser->append(lyricTextList.at(i)); /* 添加文本 */
    }

    ui->musicLyricTextBrowser->ensureCursorVisible();   /* 保证可见 */
}

/* 播放上一首 */
void MusicPlayer::on_previousPushButton_clicked()
{
    QMediaPlaylist::PlaybackMode currentMode = mediaPlayList->playbackMode();

    /* 防止单曲循环时，上/下一首不被允许切换 */
    if (currentMode == QMediaPlaylist::CurrentItemInLoop) {
        mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);
        mediaPlayList->previous();
        mediaPlayList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    } else {
        mediaPlayList->previous();
    }
}

/* 播放下一首 */
void MusicPlayer::on_nextPushButton_clicked()
{
    QMediaPlaylist::PlaybackMode currentMode = mediaPlayList->playbackMode();

    if (currentMode == QMediaPlaylist::CurrentItemInLoop) {
        mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);
        mediaPlayList->next();
        mediaPlayList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    } else {
        mediaPlayList->next();
    }
}

/* 点击音量设置图标 */
void MusicPlayer::on_voiceControlPushButton_clicked()
{
    if (ui->volumeWidget->isVisible()) {
        ui->volumeWidget->hide();
    } else {
        ui->volumeWidget->show();
        ui->volumeWidget->raise();
    }
}

/* 事件过滤器 */
bool MusicPlayer::eventFilter(QObject *watched, QEvent *event)
{
    /* 监听主页面与鼠标点击事件 */
    if (watched == ui->mainPage && event->type() == QEvent::MouseButtonPress) {
        if (ui->volumeWidget->isVisible()) {    /*如果音量窗口可见 */
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); /* 转换事件类型 */
            QPoint mousePosition = mouseEvent->pos();   /* 获取点击位置 */

            /* 如果点击非音量条（空白处） */
            if (!ui->volumeWidget->geometry().contains(mousePosition)) {
                ui->volumeWidget->hide();   /* 隐藏音量窗口 */
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

/* 音量滑动条按下 */
void MusicPlayer::on_voiceControlHorizontalSlider_sliderPressed()
{
    int volumeValue = ui->voiceControlHorizontalSlider->value();    /* 获取当前音量 */
    voiceControl->lastVolume = volumeValue;                         /* 保存上次音量 */
}

/* 音量滑动条释放 */
void MusicPlayer::on_voiceControlHorizontalSlider_sliderReleased()
{
    int volumeValue = ui->voiceControlHorizontalSlider->value();

    if (volumeValue == 0) {
        voiceControl->applyVolume(0);       /* 应用音量 */
        emit onMusicPlayerVolumeChanged(0); /* 发出音量变化信号 */
        //qDebug() << "Current volume value is " << voiceControl->currentVolume << "%";
    } else {
        voiceControl->applyVolume(volumeValue);
        emit onMusicPlayerVolumeChanged(volumeValue);
        //qDebug() << "当前音量：" << voiceControl->currentVolume << "%";
    }
}

/* 主窗口音量变化同步 */
void MusicPlayer::onMainWindowsVolumeChanged(int value)
{
    ui->voiceControlHorizontalSlider->setValue(value);  /* 同步滑动条数值 */
}

/* 给外部提供暂停播放入口 */
void MusicPlayer::pauseMusic()
{
    if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
    }
}

/* 重置页面 */
void MusicPlayer::resetPage()
{
    /* 1.停止欢迎页定时器 */
    if (enterMainPageTimer->isActive()) {
        enterMainPageTimer->stop();
    }

    /* 2.停止播放 */
    mediaPlayer->stop();

    /* 3.清空歌词 */
    ui->musicLyricTextBrowser->setText("暂无歌词");
    lyricTimeList.clear();
    lyricTextList.clear();
    currentLyricIndex = -1;

    /* 4.进度条归零 */
    ui->processBarHorizontalSlider->setValue(0);
    ui->currentTimeLabel->setText("00:00");
    ui->endTimeLabel->setText("00:00");
    mediaPlayer->setPosition(0);

    /* 5.清除封面 */
    ui->albumCoverLabel->clear();

    /* 6.清除当前歌曲显示 */
    ui->currentPlayLabel->setText("音乐播放器");

    /* 7.播放列表索引重置 */
    mediaPlayList->setCurrentIndex(-1);
    loadMusicFiles("/home/root/KaydonOS/music");
    ui->musicListWidget->scrollToTop();

    /* 8.播放顺序重置 */
    mediaPlayList->setPlaybackMode(QMediaPlaylist::Loop);
    playSort = 1;
    ui->playSortPushButton->setProperty("playSort", playSort);
    ui->playSortPushButton->setStyle(ui->playSortPushButton->style());

    /* 9.隐藏音量条 */
    ui->volumeWidget->hide();

    /* 10.允许下次重新播放欢迎页动画 */
    hasEnteredMainPage = false;
    ui->musicPlayerStackedWidget->setCurrentIndex(0);

    qDebug() << "MusicPlayer has been reset";
}


