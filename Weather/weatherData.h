#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QString>

class Today {

public:
    Today()
    {
        province = "";
        city = "";
        district = "";
        weather = "";
        temperature = 0;
        wind_direction = "";
        wind_power = "";
        humidity = 0;
        report_time = "";
        temp_max = 0;
        temp_min = 0;
        aqi = 0;
        aqi_category = "";
    }

    QString province;
    QString city;
    QString district;
    QString weather;
    double temperature;
    int temperatureDoubleToInt;
    QString wind_direction;
    QString wind_power;
    int humidity;
    QString report_time;
    int temp_max;
    int temp_min;
    int aqi;
    QString aqi_category;
};

class Forecast {

public:
    Forecast()
    {
        date = "";
        week = "";
        temp_max = 0;
        temp_min = 0;
        weather_day = "";
        wind_dir_day = "";
        wind_scale_day = "";
        windyState = "";
    }

    QString date;
    QString week;
    int temp_max;
    int temp_min;
    QString weather_day;
    QString wind_dir_day;
    QString wind_scale_day;
    QString windyState;
};

#endif // WEATHERDATA_H
