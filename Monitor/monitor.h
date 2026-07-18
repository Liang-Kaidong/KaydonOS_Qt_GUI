#ifndef MONITOR_H
#define MONITOR_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>
#include "Driver/als/alsdistant.h"
#include "Driver/beep/beepcontrol.h"
#include "Driver/v4l2camera/v4l2camera.h"
#include "Driver/v4l2camera/cameracontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Monitor; }
QT_END_NAMESPACE

class Monitor : public QWidget
{
    Q_OBJECT
public:
    Monitor(QWidget *parent = nullptr);
    void closeMonitor();
    void openMonitor();
    ~Monitor();

private slots:
    void updateDate();
    void warnUser();
    void beepTimerSlot();
    void updateFrame();

private:
    Ui::Monitor *ui;

    QTimer *dateTimer = nullptr;

    AlsDistant alsDistant;
    QTimer *warningTimer = nullptr;
    QTimer *beepTimer = nullptr;
    BeepControl *beepControl = nullptr;

    CameraController *controller = nullptr;
    QTimer *cameraTimer = nullptr;
};
#endif // MONITOR_H
