#include "weather.h"
#include "ui_weather.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>

#define INCREMENT 3            /* 温度每升高/降低1°的Y轴偏移量 */
#define POINT_RADIUS 3         /* 曲线描点的大小 */
#define POINT_OFFSET_X 25      /* 曲线描点X轴偏移量 */
#define POINT_OFFSET_Y 15      /* 曲线描点Y轴偏移量 */
#define TEXT_OFFSET_X 20       /* 曲线文本X轴偏移量 */
#define TEXT_OFFSET_Y 5        /* 曲线文本Y轴偏移量 */

Weather::Weather(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Weather)
{
    ui->setupUi(this);

    netAccessManager = new QNetworkAccessManager(this);
    connect(netAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplied(QNetworkReply*)));

    /* 将控件添加到控件数组 */
    weekList << ui->day2TitleLabel << ui->day3TitleLabel << ui->day4TitleLabel << ui->day5TitleLabel;
    weatherIconList << ui->day2WeatherIconLabel << ui->day3WeatherIconLabel << ui->day4WeatherIconLabel << ui->day5WeatherIconLabel;
    weatherStateList << ui->day2WeatherStateLabel << ui->day3WeatherStateLabel << ui->day4WeatherStateLabel << ui->day5WeatherStateLabel;
    windyStateList << ui->day2WindyStateLabel << ui->day3WindyStateLabel << ui->day4WindyStateLabel << ui->day5WindyStateLabel;
    highestTemperatureList << ui->day2HighestTemperatureLabel << ui->day3HighestTemperatureLabel << ui->day4HighestTemperatureLabel << ui->day5HighestTemperatureLabel;
    lowestTemperatureList << ui->day2LowestTemperatureLabel << ui->day3LowestTemperatureLabel << ui->day4LowestTemperatureLabel << ui->day5LowestTemperatureLabel;
    temperatureProcessBarList << ui->day2TemperatureProgressBar << ui->day3TemperatureProgressBar << ui->day4TemperatureProgressBar << ui->day5TemperatureProgressBar;

    /* 天气类型：以天气状态为KEY，资源路径为value */
    weatherTypeMap.insert("晴", ":/Icons/weatherIcons/Icons/sunny.png");
    weatherTypeMap.insert("多云", ":/Icons/weatherIcons/Icons/cloudy.png");
    weatherTypeMap.insert("阴", ":/Icons/weatherIcons/Icons/overcast.png");
    weatherTypeMap.insert("雨", ":/Icons/weatherIcons/Icons/rain.png");
    weatherTypeMap.insert("小雨", ":/Icons/weatherIcons/Icons/rain.png");
    weatherTypeMap.insert("中雨", ":/Icons/weatherIcons/Icons/rain.png");
    weatherTypeMap.insert("大雨", ":/Icons/weatherIcons/Icons/rain.png");
    weatherTypeMap.insert("暴雨", ":/Icons/weatherIcons/Icons/rain.png");
    weatherTypeMap.insert("阵雨", ":/Icons/weatherIcons/Icons/rain.png");
    weatherTypeMap.insert("雷阵雨", ":/Icons/weatherIcons/Icons/thunderRain.png");
    weatherTypeMap.insert("小雪", ":/Icons/weatherIcons/Icons/snow.png");
    weatherTypeMap.insert("中雪", ":/Icons/weatherIcons/Icons/snow.png");
    weatherTypeMap.insert("大雪", ":/Icons/weatherIcons/Icons/snow.png");
    weatherTypeMap.insert("暴雪", ":/Icons/weatherIcons/Icons/snow.png");
    weatherTypeMap.insert("细雪", ":/Icons/weatherIcons/Icons/snow.png");
    weatherTypeMap.insert("雨夹雪", ":/Icons/weatherIcons/Icons/snow.png");
    weatherTypeMap.insert("雾", ":/Icons/weatherIcons/Icons/foggy.png");
    weatherTypeMap.insert("雾霾", ":/Icons/weatherIcons/Icons/foggy.png");
    weatherTypeMap.insert("霾", ":/Icons/weatherIcons/Icons/foggy.png");
    weatherTypeMap.insert("沙尘", ":/Icons/weatherIcons/Icons/sandDust.png");

    /* 天气背景类型：以天气状态为KEY，资源路径为value */
    weatherTypeBackgroundMap.insert("晴", ":/Pics/weatherPics/Pics/sunny.png");
    weatherTypeBackgroundMap.insert("多云", ":/Pics/weatherPics/Pics/cloudy.png");
    weatherTypeBackgroundMap.insert("阴", ":/Pics/weatherPics/Pics/overcast.png");
    weatherTypeBackgroundMap.insert("小雨", ":/Pics/weatherPics/Pics/rain.png");
    weatherTypeBackgroundMap.insert("中雨", ":/Pics/weatherPics/Pics/rain.png");
    weatherTypeBackgroundMap.insert("大雨", ":/Pics/weatherPics/Pics/rain.png");
    weatherTypeBackgroundMap.insert("暴雨", ":/Pics/weatherPics/Pics/rain.png");
    weatherTypeBackgroundMap.insert("阵雨", ":/Pics/weatherPics/Pics/rain.png");
    weatherTypeBackgroundMap.insert("雷阵雨", ":/Pics/weatherPics/Pics/thunderRain.png");
    weatherTypeBackgroundMap.insert("小雪", ":/Pics/weatherPics/Pics/snow.png");
    weatherTypeBackgroundMap.insert("中雪", ":/Pics/weatherPics/Pics/snow.png");
    weatherTypeBackgroundMap.insert("大雪", ":/Pics/weatherPics/Pics/snow.png");
    weatherTypeBackgroundMap.insert("暴雪", ":/Pics/weatherPics/Pics/snow.png");
    weatherTypeBackgroundMap.insert("细雪", ":/Pics/weatherPics/Pics/snow.png");
    weatherTypeBackgroundMap.insert("雨夹雪", ":/Pics/weatherPics/Pics/snow.png");
    weatherTypeBackgroundMap.insert("雾", ":/Pics/weatherPics/Pics/foggy.png");
    weatherTypeBackgroundMap.insert("雾霾", ":/Pics/weatherPics/Pics/foggy.png");
    weatherTypeBackgroundMap.insert("霾", ":/Pics/weatherPics/Pics/foggy.png");
    weatherTypeBackgroundMap.insert("沙尘", ":/Pics/weatherPics/Pics/sandstorm.png");

    /* 请求天气数据 */
    getWeatherInfo("顺德");    /* 城市名 */

    /* 给温度曲线加事件过滤器 */
    /* this指定当前窗口对象 */
    ui->highestTemperatureCurveLabel->installEventFilter(this);
    ui->lowestTemperatureCurveLabel->installEventFilter(this);

    /* 显示更新天气数据提示框 */
    messageTimer.setSingleShot(true);
    connect(&messageTimer, SIGNAL(timeout()), this, SLOT(hideMessageBox()));
    hideMessageBox();
}

Weather::~Weather()
{
    delete ui;
}

void Weather::getWeatherInfo(QString cityName)
{
    /* cityName留空，则自动获取当地位置 */
    QUrl url("https://uapis.cn/api/v1/misc/weather?city=" + cityName + "&forecast=true&extended=true");
    netAccessManager->get(QNetworkRequest(url));
}

void Weather::onReplied(QNetworkReply *netWorkReply)
{
    int statusCode = netWorkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    //qDebug() << "operation is " << netWorkReply->operation();
    //qDebug() << "getStatusCode is " << statusCode;
    //qDebug() << "url is " << netWorkReply->url();
    //qDebug() << "raw header is " << netWorkReply->rawHeaderList();

    if (netWorkReply->error() != QNetworkReply::NoError || statusCode !=200) {
        //qDebug() << "reply error " << netWorkReply->errorString().toLatin1().data();
        showMessageBox("请求数据失败，请检查网络");
    } else {
        QByteArray fromNetworkReply = netWorkReply->readAll();
        //qDebug() << "read networkreply data is " << fromNetworkReply.data();
        parseJson(fromNetworkReply);
    }

    /* 在堆上，防内存泄漏 */
    netWorkReply->deleteLater();
}

void Weather::parseJson(QByteArray &byteArray)
{
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(byteArray, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject rootObj = jsonDocument.object();

    /* 1.解析日期与城市(有些地方无区/县，则用city；否则默认用district) */
    today.report_time = rootObj.value("report_time").toString();
    today.city = rootObj.value("city").toString();
    today.district = rootObj.value("district").toString();

    //qDebug() << rootObj.value("report_time").toString();
    //qDebug() << rootObj.value("city").toString();
    //qDebug() << rootObj.value("district").toString();
    //qDebug() << rootObj.value("weather").toString();
    //qDebug() << rootObj.value("temp_max").toInt();
    //qDebug() << rootObj.value("temp_min").toInt();
    //qDebug() << rootObj.value("humidity").toInt();
    //qDebug() << rootObj.value("wind_direction").toString();
    //qDebug() << rootObj.value("wind_power").toString();
    //qDebug() << rootObj.value("aqi").toInt();
    //qDebug() << rootObj.value("aqi_category").toString();

    /* 2.解析今天的数据 */
    today.weather = rootObj.value("weather").toString();

    double temperature = rootObj.value("temperature").toDouble();
    int temperatureDoubleToInt = (int)temperature;
    today.temperatureDoubleToInt = temperatureDoubleToInt;
    today.temp_max = rootObj.value("temp_max").toInt();
    today.temp_min = rootObj.value("temp_min").toInt();
    QString windDir = rootObj.value("wind_direction").toString();
    if (windDir == "无持续风向") {       /* 防UI排版乱 */
        windDir = "无风向";
    }
    today.wind_direction = windDir;
    today.wind_power = rootObj.value("wind_power").toString();

    today.humidity = rootObj.value("humidity").toInt();
    today.aqi = rootObj.value("aqi").toInt();
    today.aqi_category = rootObj.value("aqi_category").toString();

    today.report_time = rootObj.value("report_time").toString();

    /* 2.解析多天数据，不含今天 */
    QJsonArray forecastArray = rootObj.value("forecast").toArray();

    //qDebug() << "\nforecast";

    for (int i = 1; i < 5; i++) {
        QJsonObject objForecast = forecastArray[i].toObject();
        forecast[i].week = objForecast.value("week").toString();
        forecast[i].temp_max = objForecast.value("temp_max").toInt();
        forecast[i].temp_min = objForecast.value("temp_min").toInt();
        forecast[i].weather_day = objForecast.value("weather_day").toString();
        QString windDir = objForecast.value("wind_dir_day").toString();
        if (windDir == "无持续风向") {           /* 防UI排版乱 */
            windDir = "无风向";
        }
        forecast[i].wind_dir_day = windDir;
        forecast[i].wind_scale_day = objForecast.value("wind_scale_day").toString();
        forecast[i].windyState = windDir + " " + objForecast.value("wind_scale_day").toString();

        //qDebug() << "\n" << objForecast.value("week").toString()
        //         << "\n" << objForecast.value("temp_max").toInt()
        //         << "\n" << objForecast.value("temp_min").toInt()
        //         << "\n" << objForecast.value("weather_day").toString()
        //         << "\n" << objForecast.value("wind_dir_day").toString()
        //         << "\n" << objForecast.value("wind_scale_day").toString();
    }
    /* 更新UI */
    updateUI();

    if (ui->messageBoxPushButton->isVisible()) {
        ui->messageBoxPushButton->hide();
    }
}

void Weather::updateUI()
{
    /* 更新今天数据 */
    if (today.district.isEmpty()) {
        ui->cityLabel->setText(today.city);
    } else {
        ui->cityLabel->setText(today.district);
    }

    ui->currentTemperatureLabel->setText(QString::number(today.temperatureDoubleToInt) + "°");
    ui->currentWeatherLabel->setText(today.weather +
                                     " 最高" + QString::number(today.temp_max) + "° " +
                                     "最低" + QString::number(today.temp_min) + "°");
    ui->currentHumidityPushButton->setText(QString::number(today.humidity) + "%");
    ui->currentHumidityPushButton->setFixedHeight(31);
    ui->currentHumidityPushButton->setFixedWidth(ui->currentHumidityPushButton->sizeHint().width() + 10);
    ui->airQualityPushButton->setText("空气" + today.aqi_category + " " + QString::number(today.aqi));
    ui->airQualityPushButton->setFixedHeight(31);
    ui->airQualityPushButton->setFixedWidth(ui->airQualityPushButton->sizeHint().width() + 10);

    ui->todayWeatherStateLabel->setText(today.weather);
    ui->todayWindyStateLabel->setText(today.wind_direction + " " + today.wind_power);
    ui->todayWeatherIconLabel->setPixmap(weatherTypeMap[today.weather]);
    /* designer界面不要使用border-image ,同时打开ScaledContents使图片与控件大小一致 */
    ui->todayHighestTemperatureLabel->setText(QString::number(today.temp_max) + "°");
    ui->todayLowestTemperatureLabel->setText(QString::number(today.temp_min) + "°");
    ui->todayTemperatureProgressBar->setValue(today.temp_max);

    ui->latestdataSourceLabel->setText("数据最后更新时间  " + today.report_time);
    ui->backgroundWidget->setStyleSheet("QWidget#backgroundWidget { "
                                        "background-image: url(" + weatherTypeBackgroundMap[today.weather] + "); "
                                        "}"
                                        );

    /* 更新后几天数据 */
    for (int i = 0; i < 4; i++) {
        /* 更新日期 */
        weekList[i]->setText("周" + forecast[i+1].week.right(1));

        /* 更新天气类型 */
        weatherStateList[i]->setText(forecast[i+1].weather_day);

        /* 更新天气图标 */
        weatherIconList[i]->setPixmap(weatherTypeMap[forecast[i+1].weather_day]);

        /* 更新风值 */
        windyStateList[i]->setText(forecast[i+1].windyState);

        /* 更新最高温度 */
        highestTemperatureList[i]->setText(QString::number(forecast[i+1].temp_max) + "°");

        /* 更新最低温度 */
        lowestTemperatureList[i]->setText(QString::number(forecast[i+1].temp_min) + "°");

        /* 更新温度条 */
        temperatureProcessBarList[i]->setValue(forecast[i+1].temp_max);
    }

}

bool Weather::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->highestTemperatureCurveLabel && event->type() == QEvent::Paint) {
        paintHighestTemperatureCurve();
    }

    if (watched == ui->lowestTemperatureCurveLabel && event->type() == QEvent::Paint) {
        paintLowestTemperatureCurve();
    }

    /* 非上述事件，则沿用QWidget自身的 */
    return QWidget::eventFilter(watched, event);
}


void Weather::paintHighestTemperatureCurve()
{
    QPainter painter(ui->highestTemperatureCurveLabel);

    /* 抗锯齿 */
    painter.setRenderHint(QPainter::Antialiasing, true);

    /* 获取x坐标 */
    int pointX[5] = {0};    /* 五天的坐标 */
    int step = ui->highestTemperatureCurveLabel->width() / 4.5;   /* 5个点，4.5个间隔 */
    for (int i = 0; i < 5; i++) {
        pointX[i] = step * i;   /* 均匀分布X坐标 */
    }

    /* 获取y坐标 */
    int tempSum = 0;
    int tempAverage = 0;

    tempSum += today.temp_max;              /* 今日最高温度 */
    for (int i = 1; i < 5; i++) {
        tempSum += forecast[i].temp_max;    /* 未来四天的最高温度 */
    }
    tempAverage = tempSum / 5;  /* 5天内最高温平均值 */

    /* 计算y轴坐标 */
    int pointY[5] = {0};
    int yCenter = ui->highestTemperatureCurveLabel->height() / 2;   /* 标签1/2值 */

    pointY[0] = yCenter - ((today.temp_max - tempAverage) * INCREMENT);
    for (int i = 1; i < 5; i++) {
        /* QT坐标：下增右增，上减左减 */
        pointY[i] = yCenter - ((forecast[i].temp_max - tempAverage) * INCREMENT);
    }

    /* 绘制温度点 */
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255, 170, 0));
    painter.setPen(pen);
    painter.setBrush(QColor(255, 170, 0));  /* 画刷：设置内部填充颜色 */
    for (int i = 0; i < 5; i++) {
        painter.drawEllipse(QPoint(pointX[i] + POINT_OFFSET_X, pointY[i] + POINT_OFFSET_Y), POINT_RADIUS, POINT_RADIUS);
    }

    /* 绘制温度文本 */
    pen.setWidth(1);
    pen.setColor(QColor(255, 255, 255));
    painter.setFont(QFont("MiSans", 12));
    painter.setPen(pen);
    painter.drawText(pointX[0] + TEXT_OFFSET_X, pointY[0] + TEXT_OFFSET_Y, QString::number(today.temp_max) + "°");
    for (int i = 0; i < 4; i++) {
        painter.drawText(pointX[i+1] + TEXT_OFFSET_X, pointY[i+1] + TEXT_OFFSET_Y, QString::number(forecast[i+1].temp_max) + "°");
    }

    /* 绘制温度曲线 */
    pen.setWidth(2);
    pen.setColor(QColor(255, 170, 0));
    painter.setPen(pen);
    for (int i = 0; i < 4; i++) {
        painter.drawLine(pointX[i] + POINT_OFFSET_X, pointY[i] + POINT_OFFSET_Y, pointX[i+1] + POINT_OFFSET_X, pointY[i+1] + POINT_OFFSET_Y);
    }
}

void Weather::paintLowestTemperatureCurve()
{
    QPainter painter(ui->lowestTemperatureCurveLabel);

    /* 抗锯齿 */
    painter.setRenderHint(QPainter::Antialiasing, true);

    /* 获取x坐标 */
    int pointX[5] = {0};    /* 五天的坐标 */
    int step = ui->lowestTemperatureCurveLabel->width() / 4.5;   /* 5个点，4.5个间隔 */
    for (int i = 0; i < 5; i++) {
        pointX[i] = step * i;   /* 均匀分布X坐标 */
    }

    /* 获取y坐标 */
    int tempSum = 0;
    int tempAverage = 0;

    tempSum += today.temp_min;
    for (int i = 1; i < 5; i++) {
        tempSum += forecast[i].temp_min;
    }
    tempAverage = tempSum / 5;

    /* 计算y轴坐标 */
    int pointY[5] = {0};
    int yCenter = ui->lowestTemperatureCurveLabel->height() / 2;   /* 标签1/2值 */

    pointY[0] = yCenter - ((today.temp_min - tempAverage) * INCREMENT);
    for (int i = 1; i < 5; i++) {
        /* QT坐标：下增右增，上减左减 */
        pointY[i] = yCenter - ((forecast[i].temp_min - tempAverage) * INCREMENT);
    }

    /* 绘制温度点 */
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(0, 255, 255));
    painter.setPen(pen);
    painter.setBrush(QColor(0, 255, 255));  /* 画刷：设置内部填充颜色 */
    for (int i = 0; i < 5; i++) {
        painter.drawEllipse(QPoint(pointX[i] + POINT_OFFSET_X, pointY[i] + POINT_OFFSET_Y), POINT_RADIUS, POINT_RADIUS);
    }

    /* 绘制温度文本 */
    pen.setWidth(1);
    pen.setColor(QColor(255, 255, 255));
    painter.setFont(QFont("MiSans", 12));
    painter.setPen(pen);
    painter.drawText(pointX[0] + TEXT_OFFSET_X, pointY[0] + TEXT_OFFSET_Y, QString::number(today.temp_min) + "°");
    for (int i = 0; i < 4; i++) {
        painter.drawText(pointX[i+1] + TEXT_OFFSET_X, pointY[i+1] + TEXT_OFFSET_Y, QString::number(forecast[i+1].temp_min) + "°");
    }

    /* 绘制温度曲线 */
    pen.setWidth(2);
    pen.setColor(QColor(0, 255, 255));
    painter.setPen(pen);
    for (int i = 0; i < 4; i++) {
        painter.drawLine(pointX[i] + POINT_OFFSET_X, pointY[i] + POINT_OFFSET_Y, pointX[i+1] + POINT_OFFSET_X, pointY[i+1] + POINT_OFFSET_Y);
    }
}

/* 更新天气数据 */
void Weather::on_refreshPushButton_clicked()
{
    QString cityName = ui->searchCityLineEdit->text();
    if (cityName.isEmpty()) {
        cityName = today.district;
        if (cityName.isEmpty()) {
            cityName = today.city;
        }
    }

    getWeatherInfo(cityName);
    ui->searchCityLineEdit->clear();
    showMessageBox("正在更新天气数据", QIcon(":/icons/resources/icons/refresh.png"));
}

void Weather::on_searchCityPushButton_clicked()
{
    QString cityName = ui->searchCityLineEdit->text();
    if (cityName.isEmpty()) {
        showMessageBox("请检查输入的城市名是否有误");
        return;
    }
    getWeatherInfo(cityName);
    ui->searchCityLineEdit->clear();
    showMessageBox("正在更新天气数据", QIcon(":/icons/resources/icons/refresh.png"));
}

void Weather::showMessageBox(const QString &text, const QIcon &icon)
{
    if (!icon.isNull()) {
        ui->messageBoxPushButton->setIcon(icon);
    } else {
        ui->messageBoxPushButton->setIcon(QIcon());
    }

    ui->messageBoxPushButton->setText(text);
    ui->messageBoxPushButton->setFixedWidth(ui->messageBoxPushButton->sizeHint().width() + 30);

    ui->messageBoxPushButton->show();

    if (messageTimer.isActive()) {
        messageTimer.stop();
    }

    messageTimer.start(3000);
}

void Weather::hideMessageBox()
{
    ui->messageBoxPushButton->hide();
}
