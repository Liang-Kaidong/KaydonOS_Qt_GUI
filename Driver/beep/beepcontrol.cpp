#include "beepcontrol.h"
#include <QDebug>

BeepControl::BeepControl(QObject *parent)
    : QObject(parent)
    , isOn(false)
{
}

void BeepControl::setBeep(bool on)
{
    if (isOn == on) {
        /* 状态没变化，直接返回 */
        return;
    }

    isOn = on;

    QProcess process;
    QString command = "tee";
    QStringList args;
    args << "/sys/class/leds/beep/brightness";

    process.start(command, args);
    if (!process.waitForStarted(100)) {
        qDebug() << "Beep process start failed";
        return;
    }

    if (on) {
        process.write("1");
        qDebug() << "Beep ON";
    } else {
        process.write("0");
        qDebug() << "Beep OFF";
    }

    process.closeWriteChannel();
    process.waitForFinished(100);
}

bool BeepControl::getState() const
{
    return isOn;
}
