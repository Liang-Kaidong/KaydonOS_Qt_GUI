#ifndef EXITMESSAGEBOX_H
#define EXITMESSAGEBOX_H

#include <QWidget>      /* QWidget 基类 */
#include <QEventLoop>   /* QEventLoop 用于模态阻塞 */

namespace Ui {
class ExitMessageBox; /* UI指针类 */
}

class ExitMessageBox : public QWidget
{
    Q_OBJECT
public:
    explicit ExitMessageBox(QWidget *parent = nullptr); /* 构造函数 */

    int executeModal(); /* 模态执行，返回 1=OK, 0=Cancel */
    void setString(const QString Title, const QString Text, const QString Cancle, const QString OK); // 提供对外的接口函数

private slots:
    void on_okPushButton_clicked();      /* OK按钮点击槽 */
    void on_cancelPushButton_clicked();  /* Cancel按钮点击槽 */

private:
    Ui::ExitMessageBox *ui;
    int resultValue = 0;                    /* 用户选择结果 */
    QEventLoop *eventLoopPointer = nullptr; /* 指向事件循环 */
};

#endif // EXITMESSAGEBOX_H
