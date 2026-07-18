#include "performancetool.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QTextCursor>
#include <QFont>
#include <QMessageBox>
#include <QStyleFactory>
#include <QDateTime>

/* 对话框白色样式（仅作用于 QFileDialog / QMessageBox） */
static void setDialogStyle(QWidget *widget)
{
    widget->setStyle(QStyleFactory::create("Fusion"));
    widget->setStyleSheet(R"(
        QMessageBox {
            background-color: white;
        }

        QFileDialog {
            background-color: white;
        }

        QWidget {
            background-color: white;
            color: black;
        }

        QLabel {
            background-color: white;
            color: black;
            font-size: 14px;
        }

        QLineEdit,
        QListView,
        QTreeView,
        QTextEdit {
            background-color: white;
            color: black;
            border: 1px solid gray;
        }

        QPushButton {
            background-color: rgb(240,240,240);
            color: black;
            border: 1px solid gray;
            padding: 5px 15px;
            min-width: 80px;
        }

        QPushButton:hover {
            background-color: rgb(220,220,220);
        }
    )");
}

PerformanceTool::PerformanceTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PerformanceTool)
    , logTimer(nullptr)
{
    ui->setupUi(this);

    gesture = new Gesture(nullptr);
    gesture->addVerticalScrollWidget(ui->logTextBrowser);

    floatMonitor = new getPerformanceData(nullptr);
    floatMonitor->move(30, 30);
    floatMonitor->hide();

    connect(ui->controlPushButton,
            &QPushButton::clicked,
            this,
            &PerformanceTool::onSwitchMonitor);

    connect(ui->loadPushButton,
            &QPushButton::clicked,
            this,
            &PerformanceTool::loadLog);
}

PerformanceTool::~PerformanceTool()
{
    stopWriteLog();
    floatMonitor->deleteLater();
}

void PerformanceTool::setMainWindow(MainWindow *window)
{
    floatMonitor->setMainWindow(window);
}

void PerformanceTool::closePerformanceTool()
{
    if (floatMonitor) {
        floatMonitor->hide();
    }

    stopWriteLog();
    isRunning = false;
    qDebug() << "PerformanceTool已安全停止";

    ui->logTextBrowser->clear();
}

void PerformanceTool::onSwitchMonitor()
{
    isRunning = !isRunning;

    if (isRunning) {
        ui->controlPushButton->setText("关闭性能监控");
        floatMonitor->show();
        floatMonitor->raise();
        writeLog();
    } else {
        ui->controlPushButton->setText("打开性能监控");
        floatMonitor->hide();
        stopWriteLog();
    }
}

void PerformanceTool::loadLog()
{
    if (isRunning) {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("错误");
        msgBox->setText("请先关闭性能监控");
        setDialogStyle(msgBox);
        msgBox->exec();
        delete msgBox;
        return;
    }

    QFileDialog *dialog = new QFileDialog(this);
    dialog->setWindowTitle("选择日志");
    dialog->setDirectory("/home/root/KaydonOS/logs");
    dialog->setNameFilter("*.txt");
    dialog->setFileMode(QFileDialog::ExistingFile);
    setDialogStyle(dialog);

    QString fileName;
    if (dialog->exec() == QDialog::Accepted) {
        fileName = dialog->selectedFiles().first();
    }
    delete dialog;

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream ts(&file);
    QFont font("MiSans", 12);
    ui->logTextBrowser->setFont(font);
    ui->logTextBrowser->clear();
    ui->logTextBrowser->setPlainText(ts.readAll());
    file.close();
}

void PerformanceTool::writeLog()
{
    if (isLogging) {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("错误");
        msgBox->setText("当前日志正在写入");
        setDialogStyle(msgBox);
        msgBox->exec();
        delete msgBox;
        return;
    }

    QDir dir("/home/root/KaydonOS/logs");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString time = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    QString path = dir.filePath("PerformanceTool_" + time + ".txt");

    logFile.setFileName(path);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    logStream.setDevice(&logFile);
    logStream.setCodec("UTF-8");

    QString header;
    header  = "################################################\n";
    header += "    PerformanceTool                             \n";
    header += "                                                \n";
    header += "    Author: Kaydon                              \n";
    header += "    Version: 2026.05.14-kaydonos-pfmtl          \n";
    header += "    Build_Day: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") + "\n";
    header += "################################################\n\n";

    header += "-----------------------------------------------------------------------------\n";
    header += "TIME                                  APPLICATION               CPU            MEM       \n";
    header += "-----------------------------------------------------------------------------\n";

    logStream << header;
    logStream.flush();

    QFont font("MiSans", 12);
    ui->logTextBrowser->setFont(font);
    ui->logTextBrowser->setPlainText(header);

    logTimer = new QTimer(this);

    connect(logTimer, &QTimer::timeout, this, [=]() {
        auto perf = floatMonitor->getCurrentPerformance();
        QString t =
            QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss");

        if (perf.cpu < 0) {
            perf.cpu = 0;
        }

        if (perf.mem < 0) {
            perf.mem = 0;
        }

        QString line =
            QString("%1     %2              %3%         %4%\n")
                .arg(t, -25)
                .arg(perf.appName.left(12), -12)
                .arg(perf.cpu, 4, 'f', 1)
                .arg(perf.mem, 4, 'f', 1);

        logStream << line;
        logStream.flush();

        ui->logTextBrowser->moveCursor(QTextCursor::End);
        ui->logTextBrowser->insertPlainText(line);
    });

    logTimer->start(1000);

    isLogging = true;
}

void PerformanceTool::stopWriteLog()
{
    if (!isLogging) {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("错误");
        msgBox->setText("当前未开启性能监控");
        setDialogStyle(msgBox);
        msgBox->exec();
        delete msgBox;
        return;
    }

    if (logTimer) {
        logTimer->stop();
        logTimer->deleteLater();
    }

    logStream << "-----------------------------------------------------------------------------\n";
    logFile.close();

    isLogging = false;
}

void PerformanceTool::on_clearPushButton_clicked()
{
    if (isLogging) {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("错误");
        msgBox->setText("请先暂停性能监测");
        setDialogStyle(msgBox);
        msgBox->exec();
        delete msgBox;
        return;
    }

    ui->logTextBrowser->clear();
}
