#include "exitmessagebox.h"
#include "ui_exitmessagebox.h"

ExitMessageBox::ExitMessageBox(QWidget *parent)
    : QWidget(parent), ui(new Ui::ExitMessageBox) /* 初始化UI对象 */
{
    ui->setupUi(this);  /* 设置UI布局 */
    this->hide();       /* 初始隐藏窗口 */

    connect(ui->okPushButton, SIGNAL(clicked()), this, SLOT(on_okPushButton_clicked()));
    connect(ui->cancelPushButton, SIGNAL(clicked()), this, SLOT(on_cancelPushButton_clicked()));
}

int ExitMessageBox::executeModal()
{
    this->show();   /* 显示窗口 */
    this->raise();  /* 提到最前端 */

    QEventLoop eventLoop;           /* 创建事件循环 */
    eventLoopPointer = &eventLoop;  /* 保存循环指针 */
    eventLoop.exec();               /* 阻塞直到用户操作 */
    eventLoopPointer = nullptr;     /* 循环结束清空指针 */

    return resultValue;             /* 返回用户选择结果 */
}

/* 设置显示文本 */
void ExitMessageBox::setString(const QString Title, const QString Text, const QString Cancle, const QString OK)
{
    ui->titleLabel->clear();
    ui->contentLabel->clear();
    ui->cancelPushButton->setText("");
    ui->okPushButton->setText("");

    ui->titleLabel->setText(Title);
    ui->contentLabel->setText(Text);
    ui->cancelPushButton->setText(Cancle);
    ui->okPushButton->setText(OK);
}

void ExitMessageBox::on_okPushButton_clicked()
{
    resultValue = 1;    /* 用户选择OK */
    this->hide();       /* 隐藏窗口 */
    if(eventLoopPointer) eventLoopPointer->quit(); /* 退出事件循环 */
}

void ExitMessageBox::on_cancelPushButton_clicked()
{
    resultValue = 0;    /* 用户选择Cancel */
    this->hide();       /* 隐藏窗口 */
    if(eventLoopPointer) eventLoopPointer->quit(); /* 退出事件循环 */
}
