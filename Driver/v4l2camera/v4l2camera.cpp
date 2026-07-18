#include "v4l2camera.h"
#include <QDebug>

V4L2Camera::V4L2Camera()
{
#ifdef Q_OS_LINUX
    deviceFd = -1;
    frameBuffers = nullptr;
    bufferCount = 0;
#endif
    width = 640;
    height = 480;
    pixelFormat = 0;
    m_ready = false;
#ifndef Q_OS_LINUX
    dummyFrame = QImage(width, height, QImage::Format_RGB16);
    dummyFrame.fill(Qt::blue);
#endif
}

V4L2Camera::~V4L2Camera()
{
    stopCapture();
}

// 预检测设备文件是否存在，不存在直接不打开
bool V4L2Camera::isDeviceExist(const char* dev)
{
#ifdef Q_OS_LINUX
    struct stat st;
    if (stat(dev, &st) != 0)
    {
        qDebug() << "[V4L2] device file not exist:" << dev;
        return false;
    }
    return true;
#else
    dev = "Kaydon";
    return true;
#endif
}

bool V4L2Camera::openDevice(const char* dev)
{
#ifdef Q_OS_LINUX
    // 先判断设备文件是否存在
    if (!isDeviceExist(dev))
        return false;

    if (deviceFd >= 0)
    {
        stopCapture();
    }
    deviceFd = open(dev, O_RDWR);
    if (deviceFd < 0)
    {
        perror("open device failed");
        return false;
    }
#endif
    dev = "Kaydon";
    return true;
}

bool V4L2Camera::initDevice(int w, int h, uint32_t pixFmt)
{
    width = w;
    height = h;
    pixelFormat = pixFmt;
    m_ready = false;
#ifdef Q_OS_LINUX
    if (deviceFd < 0)
    {
        qDebug() << "[V4L2] init fail: fd invalid";
        return false;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = w;
    fmt.fmt.pix.height = h;
    fmt.fmt.pix.pixelformat = pixFmt;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(deviceFd, VIDIOC_S_FMT, &fmt) < 0)
    {
        perror("VIDIOC_S_FMT");
        // ioctl失败，立刻关闭fd，防止半开状态
        close(deviceFd);
        deviceFd = -1;
        return false;
    }

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(deviceFd, VIDIOC_REQBUFS, &req) < 0)
    {
        perror("VIDIOC_REQBUFS");
        close(deviceFd);
        deviceFd = -1;
        return false;
    }

    bufferCount = req.count;
    if (frameBuffers != nullptr)
    {
        delete[] frameBuffers;
        frameBuffers = nullptr;
    }
    frameBuffers = new Buffer[bufferCount];

    for (int i = 0; i < bufferCount; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = req.type;
        buf.memory = req.memory;
        buf.index = i;
        if (ioctl(deviceFd, VIDIOC_QUERYBUF, &buf) < 0)
        {
            perror("VIDIOC_QUERYBUF");
            close(deviceFd);
            deviceFd = -1;
            return false;
        }
        frameBuffers[i].length = buf.length;
        frameBuffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, deviceFd, buf.m.offset);
        if (frameBuffers[i].start == MAP_FAILED)
        {
            perror("mmap");
            close(deviceFd);
            deviceFd = -1;
            return false;
        }
        if (ioctl(deviceFd, VIDIOC_QBUF, &buf) < 0)
        {
            perror("VIDIOC_QBUF");
            close(deviceFd);
            deviceFd = -1;
            return false;
        }
    }
    // 全部初始化成功，标记就绪
    m_ready = true;
#endif
#ifndef Q_OS_LINUX
    dummyFrame = QImage(width, height, QImage::Format_RGB16);
    dummyFrame.fill(Qt::gray);
    m_ready = true;
#endif
    return true;
}

bool V4L2Camera::startCapture()
{
#ifdef Q_OS_LINUX
    if (deviceFd < 0 || !m_ready)
    {
        qDebug() << "[V4L2] startCapture fail: device not ready";
        return false;
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(deviceFd, VIDIOC_STREAMON, &type) < 0)
    {
        perror("VIDIOC_STREAMON");
        close(deviceFd);
        deviceFd = -1;
        m_ready = false;
        return false;
    }
#endif
    return true;
}

void V4L2Camera::stopCapture()
{
#ifdef Q_OS_LINUX
    if (deviceFd < 0)
    {
        m_ready = false;
        return;
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(deviceFd, VIDIOC_STREAMOFF, &type);

    if (frameBuffers != nullptr)
    {
        for (int i = 0; i < bufferCount; i++)
        {
            if (frameBuffers[i].start != nullptr)
            {
                munmap(frameBuffers[i].start, frameBuffers[i].length);
                frameBuffers[i].start = nullptr;
            }
        }
        delete[] frameBuffers;
        frameBuffers = nullptr;
    }
    bufferCount = 0;
    close(deviceFd);
    deviceFd = -1;
    m_ready = false;
#endif
}

bool V4L2Camera::getFrame(void** data, int& index)
{
#ifdef Q_OS_LINUX
    if (deviceFd < 0 || frameBuffers == nullptr || !m_ready)
    {
        return false;
    }
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(deviceFd, VIDIOC_DQBUF, &buf) < 0)
    {
        return false;
    }
    *data = frameBuffers[buf.index].start;
    index = buf.index;
    return true;
#else
    *data = dummyFrame.bits();
    index = 0;
    return true;
#endif
}

void V4L2Camera::releaseFrame(int index)
{
#ifdef Q_OS_LINUX
    if (deviceFd < 0 || frameBuffers == nullptr || !m_ready)
        return;
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;
    ioctl(deviceFd, VIDIOC_QBUF, &buf);
#else
    index = 0;
#endif
}

bool V4L2Camera::isCameraReady() const
{
    return m_ready;
}

int V4L2Camera::getWidth() const { return width; }
int V4L2Camera::getHeight() const { return height; }
uint32_t V4L2Camera::getPixelFormat() const { return pixelFormat; }
