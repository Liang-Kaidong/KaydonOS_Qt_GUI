#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QResource>
#include <QCryptographicHash>
#include <QSettings>
#include <QDebug>

/* 计算文件/资源的MD5哈希值 */
QString calculateHash(const QString &source, bool isResource = false) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    QByteArray data;

    if (isResource) {
        QResource res(source);
        if (!res.isValid()) {
            qWarning() << "无效的资源路径：" << source;
            return "";
        }
        data = QByteArray::fromRawData((const char*)res.data(), res.size());
    } else {
        QFile file(source);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开文件计算哈希：" << source;
            return "";
        }
        data = file.readAll();
        file.close();
    }

    hash.addData(data);
    return hash.result().toHex();
}

int main(int argc, char *argv[])
{
    /* 基础路径 */
    QString baseDir = "/home/root/KaydonOS";
    QString audioDir = baseDir + "/audio";
    QString configDir = baseDir + "/config";
    QString systemConfigDir = configDir + "/systemConfig";
    QString logsDir  = baseDir + "/logs";
    QString cacheDir = baseDir + "/cache";
    QString screenshotsDir = baseDir + "/screenshots";
    QString musicDir = baseDir + "/music";
    QString videoDir = baseDir + "/video";
    QString picturesDir = baseDir + "/pictures";
    QString cameraPictureDir = baseDir + "/pictures/camera";
    QString soundRecorderDir = baseDir + "/soundRecorder";
    QString hashRecordFile = configDir + "/config_hash.ini";

    /* 创建目录 */
    QDir dir;
    QStringList dirsList = {
        audioDir,
        configDir,
        systemConfigDir,
        logsDir,
        cacheDir,
        screenshotsDir,
        musicDir,
        videoDir,
        picturesDir,
        cameraPictureDir,
        soundRecorderDir
    };
    for (const QString &d : dirsList) {
        if (!dir.exists(d)) {
            dir.mkpath(d);
        }
    }

    /* 音频文件：保持原有逻辑 */
    QStringList audioResources = {
        ":/audio/Audio/test.wav",
        ":/audio/Audio/Alarm.wav",
        ":/audio/Audio/Fresh.wav",
        ":/audio/Audio/Moment.wav",
        ":/audio/Audio/NotificationXylophone.wav",
        ":/audio/Audio/StoneSkimmingDay.wav",
        ":/audio/Audio/WaterDropNotificationDay.wav",
    };

    for (const QString &res : audioResources) {
        QString filename = QFileInfo(res).fileName();
        QString destFile = audioDir + "/" + filename;

        if (!QFile::exists(destFile)) {
            QFile::copy(res, destFile);
            //qDebug() << "复制音频：" << filename;
        }
    }

    /* 歌曲文件：保持原有逻辑 */
    QStringList musicResources = {
        ":/music/Music/lianren.mp3",
        ":/music/Music/mote.mp3",
    };

    for (const QString &res : musicResources) {
        QString filename = QFileInfo(res).fileName();
        QString destFile = musicDir + "/" + filename;

        if (!QFile::exists(destFile)) {
            QFile::copy(res, destFile);
            //qDebug() << "复制歌曲：" << filename;
        }
    }

    /* 视频文件：保持原有逻辑 */
    QStringList videoResources = {
        ":/video/Video/video1.avi",
        ":/video/Video/video2.avi",
    };

    for (const QString &res : videoResources) {
        QString filename = QFileInfo(res).fileName();
        QString destFile = videoDir + "/" + filename;

        if (!QFile::exists(destFile)) {
            QFile::copy(res, destFile);
            //qDebug() << "复制歌曲：" << filename;
        }
    }

    /* 配置文件：仅当qrc真的更新时覆盖，用户改本地则保留 */
    QStringList configResources = {
        ":/systemConfig/Config/systemConfig/volume.ini",
        ":/systemConfig/Config/systemConfig/systemUpdateVersion.ini",
        ":/systemConfig/Config/systemConfig/systemUpdateLog.txt",
        ":/systemConfig/Config/systemConfig/audioSelection.ini",
        ":/systemConfig/Config/systemConfig/brightness.ini",
    };

    /* 初始化哈希记录器（INI格式，key=文件名，value=基准哈希） */
    QSettings hashSettings(hashRecordFile, QSettings::IniFormat);

    for (const QString &res : configResources) {
        QString filename = QFileInfo(res).fileName();
        QString destFile = systemConfigDir + "/" + filename;
        /* 1. 计算当前qrc资源的哈希 */
        QString currentResHash = calculateHash(res, true);
        if (currentResHash.isEmpty()) {
            qWarning() << "跳过无效资源：" << res;
            continue;
        }

        /* 2. 读取本地记录的基准哈希（首次运行时为空） */
        QString savedResHash = hashSettings.value(filename).toString();

        if (!QFile::exists(destFile)) {
            /* 场景1：本地文件不存在 → 复制文件 + 记录基准哈希 */
            QResource resource(res);
            if (resource.isValid()) {
                QFile dest(destFile);
                if (dest.open(QIODevice::WriteOnly)) {
                    dest.write((const char*)resource.data(), resource.size());
                    dest.close();
                    /* 记录当前qrc的哈希作为基准 */
                    hashSettings.setValue(filename, currentResHash);
                    hashSettings.sync(); // 立即保存
                    qDebug() << "首次复制配置：" << filename;
                }
            }
        } else {
            /* 场景2：本地文件已存在 → 先判断qrc是否真的更新 */
            if (savedResHash.isEmpty()) {
                /* 兼容旧版本：首次记录基准哈希（用户已存在文件，但无哈希记录） */
                hashSettings.setValue(filename, currentResHash);
                hashSettings.sync();
                qDebug() << "初始化哈希记录：" << filename;
            } else if (savedResHash != currentResHash) {
                /* 子场景2.1：qrc真的更新了 → 覆盖本地文件 + 更新基准哈希 */
                QResource resource(res);
                if (resource.isValid()) {
                    QFile dest(destFile);
                    if (dest.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        dest.write((const char*)resource.data(), resource.size());
                        dest.close();
                        /* 更新基准哈希为新的qrc哈希 */
                        hashSettings.setValue(filename, currentResHash);
                        hashSettings.sync();
                        qDebug() << "QRC配置更新，覆盖本地文件：" << filename;
                    }
                }
            } else {
                /* 子场景2.2：qrc没更新（savedResHash == currentResHash）→ 不管本地文件是否修改，都保留 */
                QString localFileHash = calculateHash(destFile, false);
                if (localFileHash != currentResHash) {
                    qDebug() << "用户修改了本地配置，保留：" << filename;
                } else {
                    qDebug() << "配置无更新，保留本地文件：" << filename;
                }
            }
        }
    }

    qputenv("QT_IM_MODULE", QByteArray("Qt5Input"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
