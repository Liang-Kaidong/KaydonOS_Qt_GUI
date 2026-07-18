#include "timeoutwidget.h"
#include "ui_timeoutwidget.h"

TimeoutWidget::TimeoutWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeoutWidget)
{
    ui->setupUi(this);
}

TimeoutWidget::~TimeoutWidget()
{
    delete ui;
}

void TimeoutWidget::on_okPushButton_clicked()
{
    hideTimeoutWidget();
}
