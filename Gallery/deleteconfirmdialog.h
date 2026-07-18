#ifndef DELETECONFIRMDIALOG_H
#define DELETECONFIRMDIALOG_H

#include <QWidget>
#include <QEventLoop>

namespace Ui {
class DeleteConfirmDialog;
}

class DeleteConfirmDialog : public QWidget
{
    Q_OBJECT
public:
    explicit DeleteConfirmDialog(QWidget *parent = nullptr);
    ~DeleteConfirmDialog();

    int executeModal();                     /* 模态执行，返回 1=OK, 0=Cancel */

public slots:
    void on_okPushButton_clicked();         /* OK按钮点击槽 */
    void on_cancelPushButton_clicked();     /* Cancel按钮点击槽 */

private:
    Ui::DeleteConfirmDialog *ui;
    int resultValue = 0;                    /* 用户选择结果 */
    QEventLoop *eventLoopPointer = nullptr; /* 指向事件循环 */
};

#endif // DELETECONFIRMDIALOG_H
