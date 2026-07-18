#include "getperformancedata.h"
#include "mainwindow.h"
#include <QMouseEvent>
#include <QFile>
#include <QTextStream>

getPerformanceData::getPerformanceData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::getPerformanceData)
    , mainWindow(nullptr)
{
    ui->setupUi(this);

    setWindowFlags(
        Qt::FramelessWindowHint
        | Qt::WindowStaysOnTopHint
        | Qt::X11BypassWindowManagerHint
        | Qt::Tool
        | Qt::WindowDoesNotAcceptFocus
    );

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, &getPerformanceData::updateSystemInfo);
    timer->start();
}

void getPerformanceData::updateSystemInfo()
{
    currentPerf.cpu = getCpuUsage();
    currentPerf.mem = getMemoryUsage();

    if (mainWindow) {
        currentPerf.appName = mainWindow->currentRunningAppName();
    } else {
        currentPerf.appName = "Unknown";
    }

    ui->currentAppLabel->setText(currentPerf.appName);
    ui->cpuLabel->setText(QString("CPU: %1%").arg(currentPerf.cpu, 0, 'f', 1));
    ui->memLabel->setText(QString("MEM: %1%").arg(currentPerf.mem, 0, 'f', 1));

    raise();
    emit performanceUpdate(currentPerf);
}

systemPerformance getPerformanceData::getCurrentPerformance() const
{
    return currentPerf;
}

void getPerformanceData::setMainWindow(MainWindow *window)
{
    mainWindow = window;
}

float getPerformanceData::getCpuUsage()
{
    static long long preTotal = 0, preIdle = 0;
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) return 0;

    QTextStream stream(&file);
    QString line = stream.readLine();
    file.close();

    QStringList list = line.split(" ", QString::SkipEmptyParts);
    if (list.size() < 5) return 0;

    long long user = list[1].toLongLong();
    long long nice = list[2].toLongLong();
    long long sys  = list[3].toLongLong();
    long long idle = list[4].toLongLong();

    long long total = user + nice + sys + idle;
    float usage = 0;

    if (preTotal > 0) {
        long long dTotal = total - preTotal;
        long long dIdle = idle - preIdle;
        usage = 100.0f * (dTotal - dIdle) / dTotal;
    }

    preTotal = total;
    preIdle = idle;
    return (usage < 0) ? 0 : usage;
}

float getPerformanceData::getMemoryUsage()
{
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) return 0;

    QTextStream in(&file);
    QStringList lines = in.readAll().split("\n");
    file.close();

    qlonglong total = 0, available = 0;

    for (QString line : lines) {
        line = line.simplified();
        if (line.startsWith("MemTotal:")) total = line.split(" ").at(1).toLongLong();
        if (line.startsWith("MemAvailable:")) available = line.split(" ").at(1).toLongLong();
    }

    if (total <= 0) return 0;
    qlonglong used = total - available;
    return 100.0f * used / total;
}

void getPerformanceData::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragPos = event->globalPos() - frameGeometry().topLeft();
}

void getPerformanceData::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        move(event->globalPos() - dragPos);
}
