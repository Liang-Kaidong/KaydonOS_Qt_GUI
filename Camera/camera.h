#ifndef CAMERA_H
#define CAMERA_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include "Driver/v4l2camera/v4l2camera.h"
#include "Driver/v4l2camera/cameracontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Camera; }
QT_END_NAMESPACE

class CameraController;

class Camera : public QWidget
{
    Q_OBJECT
public:
    explicit Camera(QWidget *parent = nullptr);
    void closeCamera();
    void openCamera();
    ~Camera();

private slots:
    void updateFrame();
    void on_takePhotoPushButton_clicked();
    void doCapture();                     // 延迟执行拍照
    void onPhotoFinished(QImage img);

    void on_viewPushButton_clicked();
    void on_backPushButton_clicked();

private:
    Ui::Camera *ui;
    QTimer *timer;
    CameraController *controller;

    QString lastPhotoPath;
};

#endif
