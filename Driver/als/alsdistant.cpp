#include <QDebug>
#include <QByteArray>
#include "alsdistant.h"

AlsDistant::AlsDistant()
{

}

int AlsDistant::readAlsDistantValue()
{
    QFile file("/sys/class/misc/ap3216c/ps");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "open ps failed.";
        return -1;
    }

    QByteArray data = file.readAll();
    file.close();

    bool ok;
    int psValue = data.trimmed().toInt(&ok);

    if (!ok) {
        qDebug() << "convert failed:" << data;
        return -2;
    }

    return psValue;
}

int AlsDistant::realDistantValue()
{
    int psValue = readAlsDistantValue();

    int realDistantValue = -(psValue -= 1024);

    return realDistantValue;
}



