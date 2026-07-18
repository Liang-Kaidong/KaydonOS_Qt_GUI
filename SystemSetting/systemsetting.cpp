#include "systemsetting.h"
#include "ui_systemsetting.h"
#include <QScrollBar>
#include <QDebug>
#include <QPainter>
#include <QSettings>  /* 引入QSettings，管理配置文件读取 */
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStorageInfo> /* 读取存储空间 */
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QList>

SystemSetting::SystemSetting(QWidget *parent, VoiceControl *voiceControl) :
    QWidget(parent),
    ui(new Ui::SystemSetting),
    voiceControl(voiceControl)
{
    ui->setupUi(this); /* 设置 UI 组件 */
    gesture = new Gesture(ui->systemStackedWidget, this);           /* 创建PageSwitch对象，用于处理页面切换 */
    ui->systemStackedWidget->setCurrentIndex(0);                    /* 初始化stackedWidget，默认显示第0页 */
    gesture->addHorizontalScrollWidget({0});                        /* 允许拖动翻页，传递页索引 */
    gesture->addVerticalScrollWidget(ui->selectionScrollArea);      /* 注册允许纵向滑动的控件 */
    gesture->addVerticalScrollWidget(ui->updateLogTextBrowser);
    gesture->addVerticalScrollWidget(ui->voiceSettingScrollArea);

    /* 保存所有需要切换的按钮的原始图标（关键！） */
    originalIcons[ui->accountPushButton] = ui->accountPushButton->icon();                           /* 账户设置 */
    originalIcons[ui->aboutCarPushButton] = ui->aboutCarPushButton->icon();                         /* 关于本机设置 */
    originalIcons[ui->wlanPushButton] = ui->wlanPushButton->icon();                                 /* WLAN设置 */
    originalIcons[ui->cellularNetworkPushButton] = ui->cellularNetworkPushButton->icon();           /* 移动网络设置 */
    originalIcons[ui->bluetoothPushButton] = ui->bluetoothPushButton->icon();                       /* 蓝牙设置 */
    originalIcons[ui->voiceSettingPushButton] = ui->voiceSettingPushButton->icon();                 /* 声音设置 */
    originalIcons[ui->brightnessSettingPushButton] = ui->brightnessSettingPushButton->icon();       /* 亮度设置 */
    originalIcons[ui->moreInfoPushButton] = ui->moreInfoPushButton->icon();                         /* 更多信息 */

    /* 音量控制模块初始化 */
    ui->voiceControlSlider->setValue(voiceControl->currentVolume); /* 同步当前音量 */
    ui->voiceControlSlider->setPageStep(0);

    /* 亮度控制模块初始化 */
    brightnessControl = new BrightnessControl(this);
    ui->brightnessControlSlider->setPageStep(0);

    /* 亮度变化信号 */
    connect(brightnessControl, SIGNAL(brightnessChanged(int)),  this, SLOT(onBrightnessChanged(int)));


    gifShow = new QMovie(":/gif/systemSetting/GIF/Welcome.gif");
    ui->showGIFLabel->setMovie(gifShow);
    gifShow->stop();
    ui->showGIFLabel->setAlignment(Qt::AlignCenter);
    ui->showGIFLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    /* 放最后，作为默认第一项 */
    on_aboutCarPushButton_clicked();

}

SystemSetting::~SystemSetting()
{
    delete ui;
}

/* 结束设置 */
void SystemSetting::resetPage()
{
    ui->systemStackedWidget->setCurrentIndex(0); /* 复位 stackedWidget 页，回到第一页 */
    if (ui->selectionScrollArea) {
        ui->selectionScrollArea->verticalScrollBar()->setValue(0); /* 复位滚动条（回到顶部） */
        ui->selectionScrollArea->horizontalScrollBar()->setValue(0); /* 复位水平滚动条 */
    }
    resetLastButtonStyle();
    //qDebug() << "SystemSetting 页面已复位到第一页，ScrollArea 回到顶部 (关闭触发)";

    lastClickedButton = ui->aboutCarPushButton;
    setCurrentButtonStyle(lastClickedButton);

    gifShow->stop();
}

/* 恢复上一个按钮的样式 */
void SystemSetting::resetLastButtonStyle()
{
    if (!lastClickedButton || !originalIcons.contains(lastClickedButton)) {
        return; /* 如果没有上次点击的按钮，或者该按钮不在图标列表中，直接返回 */
    }

    /* 恢复上一个按钮的样式 */
    lastClickedButton->setStyleSheet(R"(
                                     QPushButton {
                                        background-color: rgb(50, 55, 67);
                                        color: white;
                                        padding: 15px;
                                        text-align: left;
                                        border-radius: 10px;
                                     }

                                     QPushButton:pressed {
                                        background-color: rgb(40, 45, 55);
                                     }
                                     )");

    /* 恢复上一个按钮的原始图标 */
    lastClickedButton->setIcon(originalIcons[lastClickedButton]);
}

/* 设置当前按钮的样式 */
void SystemSetting::setCurrentButtonStyle(QPushButton *currentButton)
{
    if (!currentButton) return; /* 如果按钮为空，直接返回 */

    /* 设置当前按钮样式 */
    currentButton->setStyleSheet("background-color: rgb(255, 255, 255);"
                                 "color: rgb(50, 55, 67);"
                                 "padding: 15px;"
                                 "text-align: left;"
                                 "border-radius: 10px;");

    /* 从原始图标表中取出原图标 */
    QIcon originalIcon = originalIcons[currentButton];

    /* 如果图标为空，直接返回，防止异常 */
    if (originalIcons.isEmpty()) {
        return;
    }

    /* 获取按钮当前的图标尺寸（安全写法） */
    QSize iconSize = currentButton->iconSize();

    /* 根据按钮尺寸生成对应大小的 Pixmap */
    QPixmap pixmap = originalIcon.pixmap(iconSize);

    /* 如果 pixmap 生成失败，直接返回 */
        if (pixmap.isNull()) {
            return;
        }

    /* 使用 QPainter 对图标重新上色 */
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn); /* 使用源图形模式 */
    painter.fillRect(pixmap.rect(), QColor(50, 55, 67)); /* 设置你想要的颜色 */

    /* 结束绘制（好习惯） */
    painter.end();

    /* 将修改后的图标重新设置给按钮 */
    currentButton->setIcon(QIcon(pixmap));
    lastClickedButton = currentButton; /* 记录当前按钮 */

    //qDebug() << "iconSize:" << iconSize;

    if (currentButton != ui->accountPushButton && gifShow->state() == QMovie::Running) {
        gifShow->stop();
        //qDebug() << "gif stop";
    }
}

/* 关于汽车按钮点击事件 */
void SystemSetting::on_aboutCarPushButton_clicked()
{
    resetLastButtonStyle();                                                         /* 恢复上一个按钮样式 */
    setCurrentButtonStyle(ui->aboutCarPushButton);                                  /* 设置当前按钮样式并切换到页面1 */
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->aboutPage)); /* 执行页面切换 */

    /* 读取配置文件 */
    QString versionFilePath = "/home/root/KaydonOS/config/systemConfig/systemUpdateVersion.ini";
    //qDebug() << "versionFilePath: " << versionFilePath;
    QSettings settings(versionFilePath, QSettings::IniFormat);

    /* 读取系统版本号 */
    QString version = settings.value("systemUpdateVersion/updateVersion", "当前无权限查看版本号").toString();
    //qDebug() << "systemUpdateVersion: " << version;
    ui->osVersionLabel->setText(version);

    /* 读取存储空间信息 */
    QStorageInfo storageInfo("/");  /* 根分区（IMX6 一般系统就在这里） */

    if (!storageInfo.isValid()) {
        ui->storageValueLabel->setText("无法读取");
        return;
    }

    qint64 totalBytes = storageInfo.bytesTotal();
    qint64 freeBytes = storageInfo.bytesFree();
    qint64 usedBytes = totalBytes - freeBytes;

    double totalGB = totalBytes / 1024.0 / 1024.0 / 1024.0;
    double usedGB = usedBytes / 1024.0 / 1024.0 / 1024.0;

    QString storageText = QString("%1 GB / %2 GB").arg(QString::number(usedGB, 'f', 1)).arg(QString::number(totalGB, 'f', 1));
    ui->storageValueLabel->setText(storageText);
}

/* 处理 "WLAN" 按钮点击事件 */
void SystemSetting::on_wlanPushButton_clicked()
{
    resetLastButtonStyle(); /* 恢复上一个按钮样式 */
    setCurrentButtonStyle(ui->wlanPushButton); /* 设置当前按钮样式并切换到页面0 */
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->wlanPage)); /* 执行页面切换 */
}

/* 系统更新 */
void SystemSetting::on_updatePushButton_clicked()
{
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->updatePage));

    /* 读取配置文件 */
    QString versionFilePath = "/home/root/KaydonOS/config/systemConfig/systemUpdateVersion.ini";
    QString logFilePath = "/home/root/KaydonOS/config/systemConfig/systemUpdateLog.txt";
    //qDebug() << "versionFilePath: " << versionFilePath;
    //qDebug() << "logFilePath: " << logFilePath;
    QSettings settings(versionFilePath, QSettings::IniFormat);
    QFile file(logFilePath);

    /* 读取系统版本号 */
    QString version = settings.value("systemUpdateVersion/updateVersion", "当前无权限查看版本号").toString();
    //qDebug() << "systemUpdateVersion: " << version;
    ui->versionLabel->setText(version);

    /* 读取更新日志 */
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString updateLog = in.readAll();

        ui->updateLogTextBrowser->setPlainText(updateLog);
        //qDebug() << "Update Log: " << updateLog;
        file.close();
    }
    ui->updateLogTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->updateLogTextBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/* 返回关于本机 */
void SystemSetting::on_updatePageBackPushButton_clicked()
{
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->aboutPage));
}

/* 音量与声音 */
void SystemSetting::on_voiceSettingPushButton_clicked()
{
    resetLastButtonStyle();
    setCurrentButtonStyle(ui->voiceSettingPushButton);
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->voiceSettingPage));

    /* 读取配置文件，获取当前选中的音频路径 */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/audioSelection.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    QString currentAudioPath = settings.value("audioSelection/currentAudioSelection", "").toString();

    isInitializingAudioList = true; /* 标记进入初始化状态 */

    /* 加载音频文件列表 */
    QString directoryPath = "/home/root/KaydonOS/audio";
    QDir dir(directoryPath);
    QStringList filters;
    filters << "*.wav";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

    ui->audioListWidget->clear();
    if (fileList.isEmpty()) {
        qDebug() << "没有找到 .wav 文件";
        isInitializingAudioList = false; // 取消初始化标记
        return;
    }

    /* 填充音频列表 */
    int itemHeight = 60;
    int selectedIndex = -1;
    for (int i = 0; i < fileList.size(); i++) {
        QFileInfo fileInfo = fileList.at(i);

        QListWidgetItem *item = new QListWidgetItem(fileInfo.fileName(), ui->audioListWidget);
        item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
        item->setSizeHint(QSize(item->sizeHint().width(), itemHeight));

        /* 记录当前已选中的索引 */
        if (fileInfo.absoluteFilePath() == currentAudioPath) {
            selectedIndex = i;
        }
    }

    /* 设置列表高度 */
    ui->audioListWidget->setFixedHeight(fileList.size() * itemHeight);

    /* 同步选中 */
        if (selectedIndex >= 0) {
            ui->audioListWidget->setCurrentRow(selectedIndex);
        }

    /* 初始化完成，取消标记 */
    isInitializingAudioList = false;
}

/* 音量条图标被点击 */
void SystemSetting::on_voiceControlPushButton_clicked()
{
    if (voiceControl->currentVolume != 0) {
        voiceControl->lastVolume = ui->voiceControlSlider->value();     /* 先保存静音前的音量 */
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");   /* 设置静音图标 */
        //qDebug() << "已静音";
        //qDebug() << "当前音量：0%";
        ui->voiceControlSlider->setValue(0);    /* 滑轨UI置0 */
        voiceControl->applyVolume(0);           /* 实际将音量置0 */
        emit onSystemSettingVolumeChanged(0);   /* 发送来自设置的信号 */
    } else {
        if (voiceControl->currentVolume == voiceControl->lastVolume) {
            /* currentVolume = lastVolume = 0时，给出图标更新提醒 */
            static int icon = 0;
            if (icon == 0) {
                ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
                icon = 1;
            } else {
                ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
                icon = 0;
            }
            return;
        }
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
        //qDebug() << "取消静音";
        //qDebug() << "当前音量：" << voiceControl->currentVolume << "%";
        ui->voiceControlSlider->setValue(voiceControl->lastVolume);     /* 先恢复滑轨UI的值 */
        voiceControl->applyVolume(voiceControl->lastVolume);            /* 再实际恢复音量 */
        emit onSystemSettingVolumeChanged(voiceControl->lastVolume);
    }
}

/* 点击音量条，先保存上一个音量 */
void SystemSetting::on_voiceControlSlider_sliderPressed()
{
    int lastVolume = ui->voiceControlSlider->value();
    voiceControl->lastVolume = lastVolume;
}

/* 滑轨松手后播放提示音 */
void SystemSetting::on_voiceControlSlider_sliderReleased()
{
    int voiceControlSliderValue = ui->voiceControlSlider->value();

    if (voiceControlSliderValue == 0) {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
        voiceControl->applyVolume(voiceControlSliderValue);     /* 修改实际音量 */
        //qDebug() << "当前音量：" << voiceControl->currentVolume << "%";    /* 显示当前音量 */
        emit onSystemSettingVolumeChanged(voiceControlSliderValue); /* 发射来自设置的音量设置 */
    } else {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
        voiceControl->applyVolume(voiceControlSliderValue);     /* 修改实际音量 */
        //qDebug() << "当前音量：" << voiceControl->currentVolume << "%";    /* 显示当前音量 */
        emit onSystemSettingVolumeChanged(voiceControlSliderValue); /* 发射来自设置的音量设置 */
    }

    /**
     * 播放提示音，启动一个独立于Qt程序之外的进程，不阻塞UI
     * QString program = "aplay"; QString audioPath
     * QStringList args; args << audioPath;
     * QProcess::startDetached(program, args);
     */
    /* 读取配置文件，获取当前选中的音频路径 */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/audioSelection.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    QString currentAudioPath = settings.value("audioSelection/currentAudioSelection", "").toString();
    QProcess::startDetached("aplay", QStringList() << currentAudioPath);
}

/* 当在导航栏中的音量发生改变，同步至音量设置页面 */
void SystemSetting::onMainWindowsVolumeChanged(int value)
{
    if (value == 0) {   /* 设置页点了静音图标 */
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volMuted.png)");
    } else {
        ui->voiceControlPushButton->setStyleSheet("border-image: url(:/Icons/navigationBar/Icons/volUp.png)");
    }
    ui->voiceControlSlider->setValue(value);
}

/* 铃声选择 */
void SystemSetting::on_audioListWidget_itemSelectionChanged()
{
    if (isInitializingAudioList) {
        return;   /* 初始化阶段不播放 */
    }

    QListWidgetItem *selectedItem = ui->audioListWidget->currentItem();
    if (!selectedItem) {
        return;
    }

    /* 获取当前选中的音频路径 */
    QString audioFilePath = selectedItem->data(Qt::UserRole).toString();

    /* 保证选中状态持久化 */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/audioSelection.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.setValue("audioSelection/currentAudioSelection", audioFilePath);
    //qDebug() << "Current audio selection set to: " << audioFilePath;

    /* 播放预览音 */
    QProcess::startDetached("aplay", QStringList() << audioFilePath);
    //qDebug() << "用户手动更换铃声，播放预览音：" << audioFilePath;
}

/* 显示与亮度 */
void SystemSetting::on_brightnessSettingPushButton_clicked()
{
    resetLastButtonStyle();
    setCurrentButtonStyle(ui->brightnessSettingPushButton);
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->brightnessSettingPage));

    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/brightness.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    int brightnessValue = settings.value("brightness/brightnessValue", 100).toInt();
    ui->brightnessControlSlider->setValue(brightnessValue);

    /* 手动同步自动亮度按钮样式 */
    if (brightnessControl->autoBrightnessEnabled) {
        ui->autoBrightnessPushButton->setStyleSheet("border-image: url(:/Icons/systemSetting/Icons/openButton.png);");
    } else {
        ui->autoBrightnessPushButton->setStyleSheet("border-image: url(:/Icons/systemSetting/Icons/closeButton.png);");
    }
}

/* 亮度发生变化 */
void SystemSetting::onBrightnessChanged(int value)
{
    /* 阻塞信号防止循环触发，同步滑块值 */
    ui->brightnessControlSlider->blockSignals(true);
    ui->brightnessControlSlider->setValue(value);
    ui->brightnessControlSlider->blockSignals(false);
}

/* 自动亮度 */
void SystemSetting::on_autoBrightnessPushButton_clicked()
{
    /* 1. 调用 BrightnessControl 切换自动亮度状态 */
    brightnessControl->setAutoBrightnessEnabled(!brightnessControl->autoBrightnessEnabled);

    /* 2.切换状态 */
    if (brightnessControl->autoBrightnessEnabled) {
        ui->autoBrightnessPushButton->setStyleSheet("border-image: url(:/Icons/systemSetting/Icons/openButton.png);");
        //qDebug() << "自动亮度打开";
    } else {
        ui->autoBrightnessPushButton->setStyleSheet("border-image: url(:/Icons/systemSetting/Icons/closeButton.png);");
        //qDebug() << "自动亮度关闭";
    }
}

void SystemSetting::on_brightnessControlSlider_sliderReleased()
{
    int brightnessValue = ui->brightnessControlSlider->value(); /* 滑道亮度值 */
    //qDebug() <<"滑道亮度值： " << brightnessValue;

    /* 1. 调用BrightnessControl设置亮度（内部会发射brightnessChanged信号同步UI） */
    brightnessControl->setBrightness(brightnessValue);

    /* 2. 通知BrightnessControl标记为手动优先（停止自动亮度调整） */
    brightnessControl->setManualOverride(true);

    /* 3. 保存手动亮度值到配置文件（持久化） */
    QString configFilePath = "/home/root/KaydonOS/config/systemConfig/brightness.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);
    settings.setValue("brightness/brightnessValue", brightnessValue);
}

/* 已登录账户界面 */
void SystemSetting::on_accountPushButton_clicked()
{
    resetLastButtonStyle(); /* 恢复上一个按钮样式 */
    setCurrentButtonStyle(ui->accountPushButton); /* 设置当前按钮样式并切换页面 */
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->accountPage)); /* 执行页面切换 */

    gifShow->start();
    //qDebug() << "gif start";
}

/* 更多信息 */
void SystemSetting::on_moreInfoPushButton_clicked()
{
    resetLastButtonStyle();
    setCurrentButtonStyle(ui->moreInfoPushButton);
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->moreInfoPage));

    /* 1. 获取有线网络状态 */
    QString wiredStatus = "未连接";
    QString wiredIP = "无";

    /* 2. 遍历所有网络接口 */
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaces.size(); i++) {
        QNetworkInterface iface = interfaces.at(i);

        /* 常见以太网接口名是eth0 */
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            iface.name().startsWith("eth")) {

            wiredStatus = "已连接";

            /* 获取IPV4地址 */
            QList<QNetworkAddressEntry> entries = iface.addressEntries();
            for (int j = 0; j < entries.size(); j++) {
                QNetworkAddressEntry entry = entries.at(j);
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    wiredIP = entry.ip().toString();
                    break;
                }
            }
            break;  // 找到一个有线接口就够了
        }
    }

    /* 3. 更新UI */
    ui->wiredStateLabel->setText("有线网络连接状态：" + wiredStatus);
    ui->wiredIPLabel->setText("有线IP地址：" + wiredIP);

    //qDebug() << "wiredStatus:" << wiredStatus << "wiredIP:" << wiredIP;
}

/* 移动网络 */
void SystemSetting::on_cellularNetworkPushButton_clicked()
{
    resetLastButtonStyle();
    setCurrentButtonStyle(ui->cellularNetworkPushButton);
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->cellularNetworkPage));
}

/* 蓝牙 */
void SystemSetting::on_bluetoothPushButton_clicked()
{
    resetLastButtonStyle();
    setCurrentButtonStyle(ui->bluetoothPushButton);
    gesture->slidePageRightToLeft(ui->systemStackedWidget->indexOf(ui->bluetoothPage));
}
