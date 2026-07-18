#include "Driver/v4l2camera/cameracontroller.h"
#include <QThread>
#include <QDebug>

CameraController::CameraController(QObject *parent) : QObject(parent), m_camAvail(false)
{
}

bool CameraController::cameraAvailable() const
{
    return m_camAvail && camera.isCameraReady();
}

bool CameraController::startPreview()
{
    return startPreview(PREVIEW_W, PREVIEW_H);
}

bool CameraController::startPreview(int width, int height)
{
    m_camAvail = false;
    camera.stopCapture();

    // 第一步：先检测设备是否存在，不存在直接返回
    if (!camera.isDeviceExist("/dev/video1"))
    {
        qDebug() << "摄像头启动失败！/dev/video1 设备不存在";
        return false;
    }

    if (!camera.openDevice("/dev/video1"))
    {
        qDebug() << "摄像头启动失败！打开设备失败";
        return false;
    }

    if (width > PHOTO_W) width = PHOTO_W;
    if (height > PHOTO_H) height = PHOTO_H;

    if (!camera.initDevice(width, height, V4L2_PIX_FMT_RGB565))
    {
        qDebug() << "init camera failed";
        camera.stopCapture();
        return false;
    }

    if (!camera.startCapture())
    {
        qDebug() << "start capture stream failed";
        camera.stopCapture();
        return false;
    }
    m_camAvail = true;
    return true;
}

void CameraController::stopPreview()
{
    m_camAvail = false;
    camera.stopCapture();
}

QImage CameraController::getFrame()
{
    // 摄像头未就绪直接返回空图，不调用底层
    if (!cameraAvailable())
        return QImage();

    void* data = nullptr;
    int index = 0;
    if (!camera.getFrame(&data, index) || data == nullptr)
    {
        return QImage();
    }
    QImage img((uchar*)data, camera.getWidth(), camera.getHeight(), QImage::Format_RGB16);
    camera.releaseFrame(index);
    return img.copy();
}

QImage CameraController::capturePhoto()
{
    // 无摄像头直接返回空图
    if (!cameraAvailable())
    {
        qDebug() << "拍照失败：摄像头不可用";
        return QImage();
    }

    stopPreview();
    if (!startPreview(PHOTO_W, PHOTO_H))
    {
        startPreview();
        return QImage();
    }
    QImage frame;
    for (int i = 0; i < 3; i++)
    {
        QThread::msleep(60);
        frame = getFrame();
        if (!frame.isNull() && frame.width() > 0) break;
    }
    stopPreview();
    startPreview(PREVIEW_W, PREVIEW_H);
    return frame;
}
