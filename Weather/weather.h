#ifndef WEATHER_H
#define WEATHER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QIcon>
#include "weatherData.h"

namespace Ui {
class Weather;
}

class Weather : public QWidget
{
    Q_OBJECT

public:
    explicit Weather(QWidget *parent = nullptr);
    ~Weather();

private slots:
    void on_refreshPushButton_clicked();

    void onReplied(QNetworkReply *netWorkReply);

    void on_searchCityPushButton_clicked();

    void showMessageBox(const QString &text, const QIcon &icon = QIcon());
    void hideMessageBox();

private:
    Ui::Weather *ui;

    QNetworkAccessManager *netAccessManager = nullptr;

    QTimer messageTimer;

    void getWeatherInfo(QString cityName);
    void parseJson(QByteArray &byteArray);

    Today today;
    Forecast forecast[6];

    QList<QLabel*> weekList;                    /* 存储UI控件的星期几，方便更新 */
    QList<QLabel*> weatherIconList;             /* 天气图标 */
    QList<QLabel*> weatherStateList;            /* 天气状态 */
    QList<QLabel*> windyStateList;              /* 风值 */
    QList<QLabel*> highestTemperatureList;      /* 最高温度 */
    QList<QLabel*> lowestTemperatureList;       /* 最低温度 */
    QList<QProgressBar*> temperatureProcessBarList; /* 温度条 */

    QMap<QString, QString>weatherTypeMap;           /* 天气类型：以天气状态为KEY，资源路径为value */
    QMap<QString, QString>weatherTypeBackgroundMap; /* 天气背景类型：以天气状态为KEY，资源路径为value */

    void updateUI();

    bool eventFilter(QObject *watched, QEvent *event);
    void paintHighestTemperatureCurve();        /* 高温曲线 */
    void paintLowestTemperatureCurve();         /* 低温曲线 */

};

#endif // WEATHER_H
