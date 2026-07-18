#-------------------------------------------------
#
# Project created by Kaydon 2026-07-18 17:44:30
#
#-------------------------------------------------

QT       += core gui widgets network multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KaydonOS
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    Caculator/caculator.cpp \
    Calendar/calendar.cpp \
    Camera/camera.cpp \
    Clock/circleprogresswidget.cpp \
    Clock/clock.cpp \
    Clock/timeoutwidget.cpp \
    Driver/DHT11/dht11driver.cpp \
    Driver/als/alsdistant.cpp \
    Driver/beep/beepcontrol.cpp \
    Driver/v4l2camera/cameracontroller.cpp \
    Driver/v4l2camera/v4l2camera.cpp \
    Gallery/deleteconfirmdialog.cpp \
    Gallery/gallery.cpp \
    Monitor/monitor.cpp \
    MusicPlayer/musicplayer.cpp \
    PerformanceTool/getperformancedata.cpp \
    PerformanceTool/performancetool.cpp \
    Recorder/recorder.cpp \
    VideoPlayer/videoplayer.cpp \
    Weather/weather.cpp \
    exitmessagebox.cpp \
    gesture.cpp \
    main.cpp \
    mainwindow.cpp \
    SystemSetting/systemsetting.cpp \
    SystemSetting/voicecontrol.cpp \
    SystemSetting/brightnesscontrol.cpp \

HEADERS += \
    Caculator/caculator.h \
    Calendar/calendar.h \
    Camera/camera.h \
    Clock/circleprogresswidget.h \
    Clock/clock.h \
    Clock/timeoutwidget.h \
    Driver/DHT11/dht11driver.h \
    Driver/als/alsdistant.h \
    Driver/beep/beepcontrol.h \
    Driver/v4l2camera/cameracontroller.h \
    Driver/v4l2camera/v4l2camera.h \
    Gallery/deleteconfirmdialog.h \
    Gallery/gallery.h \
    Monitor/monitor.h \
    MusicPlayer/musicplayer.h \
    PerformanceTool/getperformancedata.h \
    PerformanceTool/performancetool.h \
    Recorder/recorder.h \
    VideoPlayer/videoplayer.h \
    Weather/weather.h \
    Weather/weatherData.h \
    exitmessagebox.h \
    gesture.h \
    mainwindow.h \
    SystemSetting/systemsetting.h \
    SystemSetting/voicecontrol.h \
    SystemSetting/brightnesscontrol.h \

FORMS += \
    Caculator/caculator.ui \
    Calendar/calendar.ui \
    Camera/camera.ui \
    Clock/clock.ui \
    Clock/timeoutwidget.ui \
    Gallery/deleteconfirmdialog.ui \
    Gallery/gallery.ui \
    Monitor/monitor.ui \
    MusicPlayer/musicplayer.ui \
    PerformanceTool/getperformancedata.ui \
    PerformanceTool/performancetool.ui \
    Recorder/recorder.ui \
    VideoPlayer/videoplayer.ui \
    Weather/weather.ui \
    exitmessagebox.ui \
    mainwindow.ui \
    SystemSetting/systemsetting.ui

RESOURCES += \
    audio.qrc \
    config.qrc \
    gif.qrc \
    icons.qrc \
    music.qrc \
    video.qrc

SUBDIRS += \
    Camera/Camera.pro

DISTFILES +=
