#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#include <QObject>
#include <QImage>
#include "Driver/v4l2camera/v4l2camera.h"
class CameraController : public QObject
{
    Q_OBJECT
public:
    CameraController(QObject *parent = nullptr);
    bool startPreview();
    bool startPreview(int width, int height);
    void stopPreview();
    QImage getFrame();
    QImage capturePhoto();

    bool cameraAvailable() const;   // 判断摄像头是否可用
private:
    V4L2Camera camera;
    static const int PREVIEW_W = 720;
    static const int PREVIEW_H = 480;
    static const int PHOTO_W = 1280;
    static const int PHOTO_H = 720;
    bool m_camAvail;
};
#endif
