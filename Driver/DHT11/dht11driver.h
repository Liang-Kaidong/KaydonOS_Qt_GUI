#ifndef DHT11DRIVER_H
#define DHT11DRIVER_H

#include <QString>
#include <QObject>

class DHT11Driver : public QObject
{
    Q_OBJECT
public:
    explicit DHT11Driver(QObject *parent = nullptr);
    DHT11Driver();
    void initDHT11Driver();
    void parseDHT11Data();
    QString getDHT11Data();

private:
    QString currentData;
};

#endif // DHT11DRIVER_H
