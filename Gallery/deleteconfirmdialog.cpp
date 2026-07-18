#include "deleteconfirmdialog.h"
#include "ui_deleteconfirmdialog.h"

DeleteConfirmDialog::DeleteConfirmDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeleteConfirmDialog)
{
    ui->setupUi(this);

    this->hide();

    connect(ui->okPushButton, SIGNAL(clicked()), this, SLOT(on_okPushButton_clicked()));
    connect(ui->cancelPushButton, SIGNAL(clicked()), this, SLOT(on_cancelPushButton_clicked()));
}

DeleteConfirmDialog::~DeleteConfirmDialog()
{
    delete ui;
}

int DeleteConfirmDialog::executeModal()
{
    this->show();   /* 显示窗口 */
    this->raise();  /* 提到最前端 */

    QEventLoop eventLoop;           /* 创建事件循环 */
    eventLoopPointer = &eventLoop;  /* 保存循环指针 */
    eventLoop.exec();               /* 阻塞直到用户操作 */
    eventLoopPointer = nullptr;     /* 循环结束清空指针 */

    return resultValue;             /* 返回用户选择结果 */
}

void DeleteConfirmDialog::on_okPushButton_clicked()
{
    resultValue = 1;                /* 用户选择OK */
    this->hide();                   /* 隐藏窗口 */
    if(eventLoopPointer) eventLoopPointer->quit(); /* 退出事件循环 */
}

void DeleteConfirmDialog::on_cancelPushButton_clicked()
{
    resultValue = 0;                /* 用户选择Cancel */
    this->hide();                   /* 隐藏窗口 */
    if(eventLoopPointer) eventLoopPointer->quit(); /* 退出事件循环 */
}
