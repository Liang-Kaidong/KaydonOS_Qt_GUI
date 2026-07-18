#ifndef GETPERFORMANCEDATA_H
#define GETPERFORMANCEDATA_H

#include <QWidget>
#include <QTimer>
#include "ui_getperformancedata.h"

class MainWindow;

struct systemPerformance {
    float cpu = 0.0f;
    float mem = 0.0f;
    QString appName;
};

class getPerformanceData : public QWidget
{
    Q_OBJECT
public:
    explicit getPerformanceData(QWidget *parent = nullptr);

    systemPerformance getCurrentPerformance() const;
    void setMainWindow(MainWindow *window);

signals:
    void performanceUpdate(const systemPerformance &perf);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateSystemInfo();

private:
    Ui::getPerformanceData *ui;
    MainWindow *mainWindow;

    float getCpuUsage();
    float getMemoryUsage();

    QTimer *timer;
    QPoint dragPos;
    systemPerformance currentPerf;
};

#endif
