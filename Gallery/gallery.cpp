#include "gallery.h"
#include <QDir>
#include <QFileInfoList>
#include <QListWidget>
#include <QPixmap>
#include <QDebug>
#include <QFileDialog>
#include "ui_gallery.h"

Gallery::Gallery(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Gallery)
{
    ui->setupUi(this);

    ui->picsListWidget->setViewMode(QListView::IconMode);
    ui->picsListWidget->setResizeMode(QListView::Adjust);
    ui->picsListWidget->setMovement(QListView::Static);
    ui->picsListWidget->setSpacing(10);

    /* 缩略图大小 */
    int thumbSize = 120;
    ui->picsListWidget->setIconSize(QSize(thumbSize, thumbSize));

    /* 统一格子大小 */
    ui->picsListWidget->setGridSize(QSize(thumbSize, thumbSize));

    ui->picsListWidget->setWordWrap(false);

    connect(ui->picsListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));

    loadPictures();

    deleteConfirmDialog = new DeleteConfirmDialog(this);

    /* 按钮组 */
    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->allPicsPushButton, 1);
    buttonGroup->addButton(ui->selectPathPushButton, 2);
    buttonGroup->addButton(ui->autoShowPushButton, 3);
    buttonGroup->addButton(ui->updatePushButton, 4);
    buttonGroup->setExclusive(true);

    ui->allPicsPushButton->setCheckable(true);
    ui->selectPathPushButton->setCheckable(true);
    ui->autoShowPushButton->setCheckable(true);
    ui->updatePushButton->setCheckable(true);

    ui->allPicsPushButton->setChecked(true);

    /* 按钮组样式表 */
    buttonStyleSheet = R"(
                           QPushButton#allPicsPushButton,
                           QPushButton#selectPathPushButton,
                           QPushButton#autoShowPushButton,
                           QPushButton#updatePushButton
                           {
                               border: none;
                               border-radius: 15px;
                               text-align: left;
                               padding: 15px;
                           }

                           QPushButton#allPicsPushButton:pressed,
                           QPushButton#selectPathPushButton:pressed,
                           QPushButton#autoShowPushButton:pressed,
                           QPushButton#updatePushButton:pressed
                           {
                               background-color: rgb(200, 200, 200);
                               border: none;
                               border-radius: 15px;
                           }

                           QPushButton#allPicsPushButton:checked,
                           QPushButton#selectPathPushButton:checked,
                           QPushButton#autoShowPushButton:checked,
                           QPushButton#updatePushButton:checked
                           {
                               background-color: rgb(200, 200, 200);
                               border: none;
                               border-radius: 15px;
                           }
                       )";
    /* 应用样式表 */
    ui->allPicsPushButton->setStyleSheet(buttonStyleSheet);
    ui->selectPathPushButton->setStyleSheet(buttonStyleSheet);
    ui->autoShowPushButton->setStyleSheet(buttonStyleSheet);
    ui->updatePushButton->setStyleSheet(buttonStyleSheet);

    /* 自动播放定时器 */
    autoTimer = new QTimer(this);
    autoTimer->setInterval(2000);
    connect(autoTimer, SIGNAL(timeout()), this, SLOT(on_nextPushButton_clicked()));
}

Gallery::~Gallery()
{
    delete ui;
}

void Gallery::resetAPP()
{
    /* 1. 停止自动播放 */
    if (isAutoPlaying) {
        autoTimer->stop();
        isAutoPlaying = false;
    }

    /* 2. 清空路径（恢复默认目录） */
    currentPath.clear();

    /* 3. 重置索引 */
    currentIndex = -1;

    /* 4. 清空大图显示 */
    ui->showPicsLabel->clear();

    /* 5. 切回首页 */
    ui->picsShowStackedWidget->setCurrentWidget(ui->allPicsPage);

    /* 6. 重新加载图片 */
    loadPictures();

    /*  7. 按钮状态恢复*/
    ui->allPicsPushButton->setChecked(true);

    /* 8. 如果模态窗口存在且可见，就模拟点击取消 */
    if (deleteConfirmDialog && deleteConfirmDialog->isVisible()) {
        deleteConfirmDialog->on_cancelPushButton_clicked();
    }

    qDebug() << "Gallery reset done.";
}

/* 加载缩略图 */
void Gallery::loadPictures()
{
    QString path;

    if (currentPath.isEmpty()) {
        path = "/home/root/KaydonOS/pictures/camera";
    } else {
        path = currentPath;
    }

    QDir dir(path);

    if (!dir.exists())
    {
        qDebug() << "目录不存在:" << path;
        return;
    }

    QStringList filters;
    filters << "*.jpg" << "*.png" << "*.jpeg" << "*.bmp";

    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

    ui->picsListWidget->clear();

    int thumbSize = ui->picsListWidget->iconSize().width();

    for (int i = 0; i < fileList.size(); i++)
    {
        QFileInfo fileInfo = fileList.at(i);
        QString filePath = fileInfo.absoluteFilePath();

        QPixmap pix(filePath);

        if (pix.isNull())
        {
            qDebug() << "加载失败:" << filePath;
            continue;
        }

        /* 正方形缩略图 */
        QPixmap thumb;

        if (pix.width() > pix.height()) {
            /* 横板 → 宽度裁剪 */
            thumb = pix.scaledToHeight(thumbSize, Qt::SmoothTransformation);
            int x = (thumb.width() - thumbSize) / 2;
            thumb = thumb.copy(x, 0, thumbSize, thumbSize);
        } else {
            /* 竖板 → 高度裁剪 */
            thumb = pix.scaledToWidth(thumbSize, Qt::SmoothTransformation);
            int y = (thumb.height() - thumbSize) / 2;
            thumb = thumb.copy(0, y, thumbSize, thumbSize);
        }

        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(QIcon(thumb));

        /* 不显示文字 */
        item->setText("");

        /* 保存路径 */
        item->setData(Qt::UserRole, filePath);

        ui->picsListWidget->addItem(item);
    }
}

/* 点击显示大图 */
void Gallery::onItemClicked(QListWidgetItem *item)
{
    QString path = item->data(Qt::UserRole).toString();
    QPixmap pix(path);

    if (pix.isNull())
    {
        qDebug() << "加载大图失败:" << path;
        return;
    }

    /* 缩放到Label大小 */
    QPixmap scaledPix = pix.scaled(
                ui->showPicsLabel->width(),
                ui->showPicsLabel->height(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

    ui->showPicsLabel->setPixmap(scaledPix);
    ui->showPicsLabel->setAlignment(Qt::AlignCenter);

    /* 切换页面 */
    ui->picsShowStackedWidget->setCurrentWidget(ui->showPicspage);

    /* 记录当前索引 */
    currentIndex = ui->picsListWidget->row(item);
}

/* 返回 */
void Gallery::on_backPushButton_clicked()
{
    if (isAutoPlaying) {
        autoTimer->stop();
        isAutoPlaying = false;
        //qDebug() << "Exit autoPlaying.";
    }

    ui->picsShowStackedWidget->setCurrentWidget(ui->allPicsPage);
}

/* 上一张 */
void Gallery::on_previousPushButton_clicked()
{
    if (currentIndex <= 0)
        return; // 已经是第一张

    currentIndex--;
    QListWidgetItem *item = ui->picsListWidget->item(currentIndex);
    if (item)
        onItemClicked(item); // 复用已有显示逻辑
}

/* 下一张 */
void Gallery::on_nextPushButton_clicked()
{
    int count = ui->picsListWidget->count();
    if (count == 0) {
        return;
    }
    currentIndex++;
    if (currentIndex >= count) {
        currentIndex = 0;
    }

    QListWidgetItem *item = ui->picsListWidget->item(currentIndex);
    if (item) {
        onItemClicked(item); // 复用已有显示逻辑
    }
}

void Gallery::on_deletePushButton_clicked()
{
    int ret = deleteConfirmDialog->executeModal();

    if (ret != 1) // 用户取消
        return;

    /* 获取当前Item */
    QListWidgetItem *currentItem = ui->picsListWidget->item(currentIndex);
    if (!currentItem)
        return;

    /* 获取图片路径 */
    QString path = currentItem->data(Qt::UserRole).toString();

    /* 删除硬盘上的文件 */
    if (QFile::exists(path)) {
        if (!QFile::remove(path)) {
            qDebug() << "删除文件失败:" << path;
        }
    }

    /* 从列表中删除 */
    delete ui->picsListWidget->takeItem(currentIndex);

    int count = ui->picsListWidget->count();
    if (count == 0) {
        /* 没有图片了，返回allPicsPage */
        ui->picsShowStackedWidget->setCurrentWidget(ui->allPicsPage);
        currentIndex = -1;
        ui->showPicsLabel->clear();
        return;
    }

    /* 自动显示下一张或上一张 */
    if (currentIndex >= count)
        currentIndex = count - 1; // 已经删除了最后一张，则显示上一张

    QListWidgetItem *nextItem = ui->picsListWidget->item(currentIndex);
    if (nextItem)
        onItemClicked(nextItem);
}

/* 更新相册 */
void Gallery::on_updatePushButton_clicked()
{
    loadPictures();
    ui->allPicsPushButton->setChecked(true);
}

/* 选择相册 */
void Gallery::on_selectPathPushButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setWindowTitle("选择相册目录");

    /* dialog样式 */
    dialog.setStyleSheet(R"(
                             QFileDialog {
                                 background-color: white;
                             }
                             QWidget {
                                 background-color: white;
                             }
                             QListView, QTreeView {
                                 background-color: white;
                             }
                             QPushButton {
                                 background-color: white;
                                 border: 1px solid lightgray;
                                 padding: 5px;
                             }
                             QLineEdit {
                                 background-color: white;
                                 border: 1px solid lightgray;
                             }
                         )"
                        );

    if (dialog.exec() == QDialog::Accepted) {
        QString dir = dialog.selectedFiles().first();

        if (!dir.isEmpty()) {
            currentPath = dir;
            loadPictures();
        }
    }

    ui->allPicsPushButton->setChecked(true);
}

/* 相册照片循环播放 */
void Gallery::on_autoShowPushButton_clicked()
{
    if (ui->picsListWidget->count() == 0) {
        qDebug() << "There are no pictures.";
        return;
    }

    if (ui->picsShowStackedWidget->currentWidget() != ui->showPicspage) {
        if (currentIndex < 0) {
            currentIndex = 0;
        }

        QListWidgetItem *item = ui->picsListWidget->item(currentIndex);
        if (item) {
            onItemClicked(item);
        }

        if (!isAutoPlaying) {
            autoTimer->start();
            isAutoPlaying = true;
            qDebug() << "Is autoPlaying now.";
        }
    }

    ui->allPicsPushButton->setChecked(true);
}
