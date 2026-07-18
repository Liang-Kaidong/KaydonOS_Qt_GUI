#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H
#include <cstddef>
#include <cstdint>
#include <QImage>
#include <QColor>
#ifdef Q_OS_LINUX
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
struct Buffer {
    void* start;
    size_t length;
};
#ifndef V4L2_PIX_FMT_RGB565
#define V4L2_PIX_FMT_RGB565 v4l2_fourcc('R','G','B','P')
#endif
#else
#define V4L2_PIX_FMT_RGB565 0
#endif
class V4L2Camera
{
public:
    V4L2Camera();
    ~V4L2Camera();

    bool isDeviceExist(const char* dev);    // 检测设备文件是否存在(Linux)
    bool openDevice(const char* dev);
    bool initDevice(int w, int h, uint32_t pixFmt);
    bool startCapture();
    void stopCapture();
    bool getFrame(void** data, int& index);
    void releaseFrame(int index);
    int getWidth() const;
    int getHeight() const;
    uint32_t getPixelFormat() const;
    bool isCameraReady() const; // 设备是否完整可用(Linux)
private:
#ifdef Q_OS_LINUX
    int deviceFd;
    Buffer* frameBuffers;
    int bufferCount;
#endif
    int width;
    int height;
    uint32_t pixelFormat;
    bool m_ready; // 标记整套采集链路正常初始化完成
#ifndef Q_OS_LINUX
    QImage dummyFrame;
#endif
};
#endif
