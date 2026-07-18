#include "dht11driver.h"
#include <QProcess>
#include <QDebug>
#include <QFile>

DHT11Driver::DHT11Driver(QObject *parent) :
    QObject(parent)
{
    initDHT11Driver();
}

void DHT11Driver::initDHT11Driver()
{
    QProcess loadDHT11Driver;
    loadDHT11Driver.start("insmod", QStringList() << "/home/root/driver/dht11/dht11.ko");

    if (!loadDHT11Driver.waitForFinished()) {
        qDebug() << "Failed to load DHT11 driver";
    }
}

void DHT11Driver::parseDHT11Data()
{
    QFile DHT11File("/sys/class/misc/dht11/value");

    if (!DHT11File.exists()) {
        qDebug() << "DHT11 file does not exist.";
        return;
    }

    if (DHT11File.open(QIODevice::ReadOnly)) {
        QTextStream information(&DHT11File);
        QString data = information.readLine();


        /* 59.0,24.2 */
        QStringList values = data.split(',');

        if (values.size() == 2) {
            float humidityFloat = values.at(0).trimmed().toFloat();
            float temperatureFloat = values.at(1).trimmed().toFloat();

            int humidity = (int)humidityFloat;
            int temperature = (int)temperatureFloat;
            currentData = "车内温度：" + QString::number(temperature) + "° " + "车内湿度：" + QString::number(humidity) + "%";
        } else {
            qDebug() << "Failed to parse DHT11 data.";
            return;
        }
    }

    DHT11File.close();
}

QString DHT11Driver::getDHT11Data()
{
    parseDHT11Data();
    return currentData;
}
