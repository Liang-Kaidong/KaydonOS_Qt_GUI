#ifndef CALENDAR_H
#define CALENDAR_H

#include <QWidget>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui { class Calendar; }
QT_END_NAMESPACE

class Calendar : public QWidget
{
    Q_OBJECT
public:
    Calendar(QWidget *parent = nullptr);
    ~Calendar();

private slots:
    void onCalendarButtonClicked(int id);
    void on_lastMonthPushButton_clicked();
    void on_nextMonthPushButton_clicked();
    void on_backTodayPushButton_clicked();

private:
    Ui::Calendar *ui;
    QButtonGroup *buttonGroup = nullptr;

    QDateTime dateInfo(int day = -1);
    QDateTime todayInfo();

    void updateCalendar();

    int getButtonIdByDay(int day);  // 根据日期数字，找到对应的按钮ID
};
#endif // CALENDAR_H
