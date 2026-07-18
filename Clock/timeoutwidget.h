#ifndef TIMEOUTWIDGET_H
#define TIMEOUTWIDGET_H

#include <QWidget>

namespace Ui {
class TimeoutWidget;
}

class TimeoutWidget : public QWidget
{
    Q_OBJECT
signals:
    void hideTimeoutWidget();

public:
    explicit TimeoutWidget(QWidget *parent = nullptr);
    ~TimeoutWidget();

private slots:
    void on_okPushButton_clicked();

private:
    Ui::TimeoutWidget *ui;
};

#endif // TIMEOUTWIDGET_H
