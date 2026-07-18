#ifndef GALLERY_H
#define GALLERY_H

#include <QWidget>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QTimer>
#include "deleteconfirmdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Gallery; }
QT_END_NAMESPACE

class Gallery : public QWidget
{
    Q_OBJECT
public:
    Gallery(QWidget *parent = nullptr);
    ~Gallery();

    void resetAPP();

private slots:
    void onItemClicked(QListWidgetItem *item);
    void on_backPushButton_clicked();
    void on_previousPushButton_clicked();
    void on_nextPushButton_clicked();
    void on_deletePushButton_clicked();
    void on_updatePushButton_clicked();
    void on_selectPathPushButton_clicked();
    void on_autoShowPushButton_clicked();

private:
    Ui::Gallery *ui;
    DeleteConfirmDialog *deleteConfirmDialog;

    void loadPictures();

    int currentIndex = -1;  // 当前大图索引

    QButtonGroup *buttonGroup;
    QString buttonStyleSheet;

    QString currentPath;

    QTimer *autoTimer;
    bool isAutoPlaying = false;
};
#endif // GALLERY_H
