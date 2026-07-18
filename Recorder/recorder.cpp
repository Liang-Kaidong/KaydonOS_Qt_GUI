#include "recorder.h"
#include "ui_recorder.h"
#include <QListWidgetItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDebug>
#include <QProcess>

Recorder::Recorder(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Recorder)
{
    ui->setupUi(this);

    /* 设置GIF播放路径 */
    movie = new QMovie(":/Icons/Recorder/Icons/startRecord.gif");
    ui->GIFLabel->setScaledContents(true);

    /* 自定义手势类 */
    gesture = new Gesture(nullptr, this);
    gesture->addVerticalScrollWidget(ui->recorderFileListWidget);

    /* 初始化录音和播放器 */
    audioRecorder = new QAudioRecorder(this);
    player = new QMediaPlayer(this);

    /* 设置录音参数 */
    audioRecorder->setContainerFormat("audio/ogg");
    QAudioEncoderSettings settings;
    settings.setCodec("audio/x-vorbis");
    settings.setSampleRate(44100);
    settings.setBitRate(128000);
    settings.setChannelCount(2);
    audioRecorder->setAudioSettings(settings);

    /* 初始化音频通道 */
    initAudioInput();

    /* UI加载录音文件 */
    loadRecordingList();

    /* 初始化录音计时器 */
    recordTimer = new QTimer(this);
    recordTimer->setInterval(1000);
    connect(recordTimer, SIGNAL(timeout()), this, SLOT(updateRecordTime()));
}

Recorder::~Recorder()
{
    delete ui;
}

/* 结束程序 */
void Recorder::closeRecorder()
{
    /* 停止录音 */
    if (isRecording) {
        audioRecorder->stop();
        recordTimer->stop();
        isRecording = false;

        /* 停止并清除 GIF */
        movie->stop();
        ui->GIFLabel->clear();

        /* 恢复录音按钮图标 */
        ui->controlPushButton->setIcon(QIcon(":/Icons/Recorder/Icons/startRecord.png"));

        /* 恢复计时标签 */
        ui->recordTimeLabel->setText("点击开始录制");
    }

    /* 停止播放 */
    if (player->state() != QMediaPlayer::StoppedState) {
        player->stop();
    }
}

/* 初始化音频通道 */
void Recorder::initAudioInput()
{
    QProcess process;
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());

    /* 1. 打开 Capture */
    process.start("amixer set Capture cap");
    process.waitForFinished();

    /* 2. 打开左声道增益 / Boost */
    process.start("amixer sset 'Left Input Boost Mixer LINPUT1' 3"); // 最大
    process.waitForFinished();

    process.start("amixer sset 'Left Input Boost Mixer LINPUT2' 7"); // 最大
    process.waitForFinished();

    process.start("amixer sset 'Left Input Boost Mixer LINPUT3' 0"); // 不用
    process.waitForFinished();

    process.start("amixer sset 'Left Input Mixer Boost' on");
    process.waitForFinished();

    /* 3. 关闭右声道（如果没接麦克风） */
    process.start("amixer sset 'Right Input Mixer Boost' off");
    process.waitForFinished();

    /* 4. Capture 音量调到最大 */
    process.start("amixer sset 'Capture Volume' 63,63");
    process.waitForFinished();

    qDebug() << "WM8960 音频输入初始化完成";
}

/* UI加载录音文件 */
void Recorder::loadRecordingList()
{
    QString dirPath = "/home/root/KaydonOS/soundRecorder/";
    QDir dir(dirPath);
    if (!dir.exists()) {
        qDebug() << "目录不存在:" << dirPath;
        return;
    }

    QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.wav" << "*.ogg" << "*.aac",
                                                QDir::Files,
                                                QDir::Time);

    ui->recorderFileListWidget->clear();
    ui->recorderFileListWidget->setSpacing(6); // 项目之间间距 6

    /* 设置录音文件列表的样式表 */
    ui->recorderFileListWidget->setStyleSheet(R"(
        QListWidget#recorderFileListWidget {
            background-color: rgb(255, 255, 255);
            border: none;
            border-radius: 30px;
            padding-top: 6px;
        }

        QListWidget::item {
            border-radius: 20px;
            background-color: #f0f0f0;
            margin: 0px 8px;
            padding: 0px 18px;
        }

        QListWidget::item:selected {
            background-color: #4980F7;
            color: white;
            border-radius: 20px;
        }
    )");

    /* 遍历文件 */
    for (int i = 0; i < fileList.size(); ++i) {
        QFileInfo fileInfo = fileList.at(i);

        QListWidgetItem *item = new QListWidgetItem(ui->recorderFileListWidget);
        item->setText(fileInfo.fileName());
        item->setData(Qt::UserRole, fileInfo.filePath());

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);

        /* 左侧弹簧推按钮靠右 */
        layout->setContentsMargins(0, 0, 0, 0);     // 去掉所有内边距
        layout->setSpacing(3);                      // 播放/删除按钮之间的间距
        QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
        layout->addItem(spacer);

        /* 设置列表项按钮参数 */
        QPushButton *playButton = new QPushButton();
        playButton->setFixedSize(50, 50);
        playButton->setIcon(QIcon(":/Icons/Recorder/Icons/playRecord.png"));
        playButton->setIconSize(QSize(50, 50));
        playButton->setStyleSheet(
            "QPushButton {"
            "border-radius: 22px;"
            "background-color: transparent;"
            "color: white;"
            "padding: 0px;"
            "}"
        );

        QPushButton *deleteButton = new QPushButton();
        deleteButton->setFixedSize(50, 50);
        deleteButton->setIcon(QIcon(":/Icons/Recorder/Icons/deleteRecord.png"));
        deleteButton->setIconSize(QSize(50, 50));
        deleteButton->setStyleSheet(
            "QPushButton {"
            "border-radius: 22px;"
            "background-color: transparent;"
            "color: white;"
            "padding: 0px;"
            "}"
        );

        /* 添加按钮 */
        layout->addWidget(playButton);
        layout->addWidget(deleteButton);

        /* 添加布局 */
        widget->setLayout(layout);
        ui->recorderFileListWidget->setItemWidget(item, widget);

        /* 重设列表项目大小 */
        item->setSizeHint(QSize(0, 72));

        /* 列表项按钮点击信号 */
        connect(playButton, &QPushButton::clicked, this, [=]() {
            ui->recorderFileListWidget->setCurrentItem(item);
            playRecording();
        });
        connect(deleteButton, &QPushButton::clicked, this, [=]() {
            ui->recorderFileListWidget->setCurrentItem(item);
            deleteRecording();
        });
    }
}

/* 录音控制按钮点击事件 */
void Recorder::on_controlPushButton_clicked()
{
    if (isRecording == false) {
        /* 1. 取消列表所有选中 */
        ui->recorderFileListWidget->clearSelection();
        ui->recorderFileListWidget->setCurrentItem(nullptr);

        /* 2. 如果正在播放，自动暂停 */
        if (player->state() == QMediaPlayer::PlayingState) {
            player->stop();
        }

        /* 3. 设置录音文件保存方式 */
        QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".ogg";
        saveFilePath = "/home/root/KaydonOS/soundRecorder/" + fileName;

        audioRecorder->setOutputLocation(QUrl::fromLocalFile(saveFilePath));
        audioRecorder->record();

        /* 4. 更改录制按钮UI */
        ui->controlPushButton->setIcon(QIcon(":/Icons/Recorder/Icons/finishRecord.png"));
        ui->GIFLabel->setMovie(movie);
        movie->start();

        /* 5. 开始录制计时 */
        isRecording = true;
        //qDebug() << "开始录音:" << saveFilePath;
        recordSeconds = 0;
        ui->recordTimeLabel->clear();
        ui->recordTimeLabel->setVisible(true);
        ui->recordTimeLabel->setText("当前已录制 00:00:00");

        recordTimer->start();

    } else {
        audioRecorder->stop();

        ui->controlPushButton->setIcon(QIcon(":/Icons/Recorder/Icons/startRecord.png"));
        movie->stop();
        ui->GIFLabel->clear();

        isRecording = false;
        //qDebug() << "录音结束:" << saveFilePath;

        recordTimer->stop();
        ui->recordTimeLabel->clear();
        ui->recordTimeLabel->setText("点击开始录制");

        loadRecordingList();    // 刷新录音文件列表
    }
}

/* 点击项目旁的播放按钮 */
void Recorder::playRecording()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) return;

    QWidget *widget = button->parentWidget();
    if (!widget) return;

    QListWidgetItem *item = ui->recorderFileListWidget->itemAt(widget->pos());
    if (!item) return;

    QString filePath = item->data(Qt::UserRole).toString();
    //qDebug() << "播放:" << filePath;

    player->setMedia(QUrl::fromLocalFile(filePath));
    player->play();
}

/* 点击项目旁的删除按钮 */
void Recorder::deleteRecording()
{
    player->stop();

    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) return;

    QWidget *widget = button->parentWidget();
    if (!widget) return;

    QListWidgetItem *item = ui->recorderFileListWidget->itemAt(widget->pos());
    if (!item) return;

    QString filePath = item->data(Qt::UserRole).toString();

    //qDebug() << "删除:" << filePath;

    QFile::remove(filePath);

    delete ui->recorderFileListWidget->takeItem(ui->recorderFileListWidget->row(item));
}

/* 刷新录制计时器 */
void Recorder::updateRecordTime()
{
    recordSeconds++;
    int hours = recordSeconds / 3600;
    int minutes = (recordSeconds % 3600) / 60;
    int seconds = recordSeconds % 60;
    QString text = QString("当前已录制 %1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    ui->recordTimeLabel->setText(text);
}
