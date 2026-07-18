#ifndef PERFORMANCETOOL_H
#define PERFORMANCETOOL_H

#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include "getperformancedata.h"
#include "ui_performancetool.h"
#include "gesture.h"

class PerformanceTool : public QWidget
{
    Q_OBJECT
public:
    explicit PerformanceTool(QWidget *parent = nullptr);
    ~PerformanceTool();

    void setMainWindow(MainWindow *window);
    void closePerformanceTool();

private slots:
    void onSwitchMonitor();
    void loadLog();
    void writeLog();
    void stopWriteLog();

    void on_clearPushButton_clicked();

private:
    Ui::PerformanceTool *ui;
    getPerformanceData *floatMonitor;

    bool isRunning = false;
    bool isLogging = false;

    QFile logFile;
    QTextStream logStream;
    QTimer *logTimer;

    Gesture *gesture;
};

#endif
