#include "camera.h"
#include "ui_camera.h"

#include <QPixmap>
#include <QDateTime>
#include <QDir>
#include <QPainter>
#include <QDebug>

Camera::Camera(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Camera)
    , timer(nullptr)
{
    ui->setupUi(this);

    ui->mainStackedWidget->setCurrentIndex(0);
    ui->ViewStackedWidget->setCurrentIndex(0);

    ui->viewLabel->setScaledContents(true);
    ui->photoMessageLabel->hide();

    controller = new CameraController(this);
    bool camOk = controller->startPreview();
    if(camOk){
        // 仅摄像头正常才创建并启动刷新定时器
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Camera::updateFrame);
        timer->start(33);
    }else{
        qDebug() << "摄像头启动失败！不开启预览定时器";
        // 无摄像头显示黑色占位图（修复fill报错）
        QPixmap blankPix(ui->viewLabel->size());
        blankPix.fill(Qt::black);
        ui->viewLabel->setPixmap(blankPix);
    }

    lastPhotoPath.clear();
}

Camera::~Camera()
{
    closeCamera();
    delete ui;
}

/* 实时预览 */
void Camera::updateFrame()
{
    // 双重保护：定时器存在+摄像头可用才取帧
    if(!timer || !controller->cameraAvailable())
        return;

    QImage img = controller->getFrame();
    if(img.isNull()){
        return;
    }
    ui->viewLabel->setPixmap(QPixmap::fromImage(img));
}

/* 点击拍照 */
void Camera::on_takePhotoPushButton_clicked()
{
    // 无摄像头直接拦截拍照
    if(!controller->cameraAvailable()){
        qDebug() << "摄像头不可用，无法拍照";
        return;
    }

    ui->photoMessageLabel->show();
    if(timer) timer->stop();

    QTimer::singleShot(50, this, &Camera::doCapture);
}

/* 真正拍照 */
void Camera::doCapture()
{
    QImage img = controller->capturePhoto();
    onPhotoFinished(img);
}

/* 拍照完成 */
void Camera::onPhotoFinished(QImage img)
{
    if(img.isNull()){
        qDebug() << "拍照失败！";
    } else {
        QString savePath = "/home/root/KaydonOS/pictures/camera";
        QDir dir;
        if(!dir.exists(savePath)){
            dir.mkpath(savePath);
        }
        QString fileName = QDateTime::currentDateTime()
                .toString("yyyyMMdd_hhmmss_zzz") + ".jpg";
        lastPhotoPath = savePath + "/" + fileName;
        bool ok = img.save(lastPhotoPath, "JPG", 100);

        if(ui->viewPushButton){
            QPixmap pix = QPixmap::fromImage(img);
            if(!pix.isNull()){
                pix = pix.scaled(ui->viewPushButton->size(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
                ui->viewPushButton->setIcon(QIcon(pix));
                ui->viewPushButton->setIconSize(ui->viewPushButton->size());
            }
        }
        if(ok) qDebug() << "拍照保存成功！";
        else qDebug() << "保存失败！";
    }

    ui->photoMessageLabel->hide();
    // 仅设备正常才恢复预览
    if(controller->cameraAvailable() && timer){
        controller->startPreview();
        timer->start(33);
    }
}

/* 查看大图 */
void Camera::on_viewPushButton_clicked()
{
    if(lastPhotoPath.isEmpty()){
        qDebug() << "还没有照片！";
        return;
    }

    if(timer) timer->stop();
    controller->stopPreview();

    QPixmap pix(lastPhotoPath);
    if(pix.isNull()){
        qDebug() << "加载图片失败！";
        return;
    }

    ui->mainStackedWidget->setCurrentIndex(1);
    ui->viewNowLabel->setFixedSize(1024, 470);
    QPixmap scaledPix = pix.scaled(ui->viewNowLabel->size(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    QPixmap finalPix(ui->viewNowLabel->size());
    finalPix.fill(Qt::black);
    QPainter painter(&finalPix);
    int x = (finalPix.width() - scaledPix.width()) / 2;
    int y = (finalPix.height() - scaledPix.height()) / 2;
    painter.drawPixmap(x, y, scaledPix);
    painter.end();
    ui->viewNowLabel->setPixmap(finalPix);
}

/* 返回 */
void Camera::on_backPushButton_clicked()
{
    ui->mainStackedWidget->setCurrentIndex(0);
    // 仅摄像头正常才重启预览
    if(controller->cameraAvailable() && timer){
        if(controller->startPreview()){
            timer->start(33);
        } else {
            qDebug() << "相机启动失败！";
        }
    }
}

void Camera::closeCamera()
{
    if(timer){
        timer->stop();
    }
    if(controller){
        controller->stopPreview();
    }
}

void Camera::openCamera()
{
    // 先判断摄像头是否可用
    if(!controller->cameraAvailable()){
        qDebug() << "无摄像头，无法打开相机预览";
        return;
    }
    if(timer){
        timer->start();
    }
    controller->startPreview();
}
