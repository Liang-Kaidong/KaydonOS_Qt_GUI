#include "calendar.h"
#include "ui_calendar.h"
#include <QDateTime>
#include <QDebug>

static QDateTime g_currentDateTime;

Calendar::Calendar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calendar)
{
    ui->setupUi(this);

    /* 日历按钮组 */
    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->pushButton,   1);
    buttonGroup->addButton(ui->pushButton_2, 2);
    buttonGroup->addButton(ui->pushButton_3, 3);
    buttonGroup->addButton(ui->pushButton_4, 4);
    buttonGroup->addButton(ui->pushButton_5, 5);
    buttonGroup->addButton(ui->pushButton_6, 6);
    buttonGroup->addButton(ui->pushButton_7, 7);

    buttonGroup->addButton(ui->pushButton_8, 8);
    buttonGroup->addButton(ui->pushButton_9, 9);
    buttonGroup->addButton(ui->pushButton_10, 10);
    buttonGroup->addButton(ui->pushButton_11, 11);
    buttonGroup->addButton(ui->pushButton_12, 12);
    buttonGroup->addButton(ui->pushButton_13, 13);
    buttonGroup->addButton(ui->pushButton_14, 14);

    buttonGroup->addButton(ui->pushButton_15, 15);
    buttonGroup->addButton(ui->pushButton_16, 16);
    buttonGroup->addButton(ui->pushButton_17, 17);
    buttonGroup->addButton(ui->pushButton_18, 18);
    buttonGroup->addButton(ui->pushButton_19, 19);
    buttonGroup->addButton(ui->pushButton_20, 20);
    buttonGroup->addButton(ui->pushButton_21, 21);

    buttonGroup->addButton(ui->pushButton_22, 22);
    buttonGroup->addButton(ui->pushButton_23, 23);
    buttonGroup->addButton(ui->pushButton_24, 24);
    buttonGroup->addButton(ui->pushButton_25, 25);
    buttonGroup->addButton(ui->pushButton_26, 26);
    buttonGroup->addButton(ui->pushButton_27, 27);
    buttonGroup->addButton(ui->pushButton_28, 28);

    buttonGroup->addButton(ui->pushButton_29, 29);
    buttonGroup->addButton(ui->pushButton_30, 30);
    buttonGroup->addButton(ui->pushButton_31, 31);
    buttonGroup->addButton(ui->pushButton_32, 32);
    buttonGroup->addButton(ui->pushButton_33, 33);
    buttonGroup->addButton(ui->pushButton_34, 34);
    buttonGroup->addButton(ui->pushButton_35, 35);

    buttonGroup->addButton(ui->pushButton_36, 36);
    buttonGroup->addButton(ui->pushButton_37, 37);
    buttonGroup->addButton(ui->pushButton_38, 38);
    buttonGroup->addButton(ui->pushButton_39, 39);
    buttonGroup->addButton(ui->pushButton_40, 40);
    buttonGroup->addButton(ui->pushButton_41, 41);
    buttonGroup->addButton(ui->pushButton_42, 42);

    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onCalendarButtonClicked(int)));

    /* 获取当前的日期 */
    g_currentDateTime = QDateTime::currentDateTime();
    dateInfo();

    /* 更新日期 */
    updateCalendar();

    /* 选中今日 */
    int today = todayInfo().date().day();
    int todayId = getButtonIdByDay(today);
    if (todayId != -1) onCalendarButtonClicked(todayId);

    /* 默认隐藏返回今日按钮 */
    ui->backTodayPushButton->setVisible(false);
}

Calendar::~Calendar()
{
    delete ui;
}

QDateTime Calendar::dateInfo(int day)
{
    /* 不再每次取系统时间 */
    QDateTime currentDateTime = g_currentDateTime;

    /* 当有传入day时 */
    if (day > 0) {
        QDate dayTemp = currentDateTime.date();
        dayTemp.setDate(dayTemp.year(), dayTemp.month(), day);
        currentDateTime.date().setDate(currentDateTime.date().year(), currentDateTime.date().month(), day);
        currentDateTime.setDate(dayTemp);
        g_currentDateTime = currentDateTime;   // 保存修改
    }

    QStringList monthList = {"一月", "二月", "三月", "四月", "五月", "六月",
                             "七月", "八月", "九月", "十月", "十一月", "十二月"};

    QStringList dayList = {"一号", "二号", "三号", "四号", "五号", "六号", "七号",
                           "八号", "九号", "十号", "十一号", "十二号", "十三号", "十四号",
                           "十五号", "十六号", "十七号", "十八号", "十九号", "二十号", "二十一号",
                           "二十二号", "二十三号", "二十四号", "二十五号", "二十六号", "二十七号", "二十八号",
                           "二十九号", "三十号", "三十一号"};

    QStringList weekList = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};

    ui->selectLabel->setText(currentDateTime.toString("yyyy") + "/" + currentDateTime.toString("MM"));
    ui->currentYearLabel->setText(currentDateTime.toString("yyyy"));
    ui->currentDayLabel->setText(currentDateTime.toString("dd"));
    ui->currentDateLabel->setText(monthList[currentDateTime.date().month() - 1] + dayList[currentDateTime.date().day() - 1]);    // 元素号 - 1
    ui->currentWeekLabel->setText(weekList[currentDateTime.date().dayOfWeek() - 1]);    // 元素号 - 1

    return currentDateTime;
}

/* 用于保留今日数据 */
QDateTime Calendar::todayInfo()
{
    QDateTime todayTime = QDateTime::currentDateTime();
    return todayTime;
}

void Calendar::updateCalendar()
{
    QDateTime currentDateTime = dateInfo();

    /* 清空所有按钮 */
    for (int i = 1; i <= 42; i++) {
        QPushButton *btn = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (btn) {
            btn->setText("");
            btn->setEnabled(true);
            btn->setStyleSheet("border: none");
        }
    }

    int year = currentDateTime.date().year();
    int month = currentDateTime.date().month();
    QDate firstDay(year, month, 1);
    int totalDays = firstDay.daysInMonth();
    int week = firstDay.dayOfWeek(); //1 ~ 7

    /* 绘制上月剩余日期 */
    QDate LastMonthDay = firstDay.addDays(-1);
    int remainDays = LastMonthDay.day();
    int remainStart = week - 1;

    for (int i = 0; i < remainStart; i++) {
        int remainId = remainStart - i;
        int remainday = remainDays - i;
        QPushButton *btn = qobject_cast<QPushButton*>(buttonGroup->button(remainId));
        if (btn) {
            btn->setText(QString::number(remainday));
            btn->setStyleSheet("border:none; color: rgb(200, 200, 200);");
            btn->setEnabled(false);
        }
    }

    /* 绘制本月日期 */
    int day = 1;
    for (int i = week; i <= 42 && day <= totalDays; i++) {
        QPushButton *btn = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (btn) {
            btn->setText(QString::number(day));
            btn->setStyleSheet("border:none; color: white;");
            day++;
        }
    }

    /* 绘制下月部分日期 */
    int preday = 1;
    for (int i = week + totalDays; i <= 42; i++) {
        QPushButton *btn = qobject_cast<QPushButton*>(buttonGroup->button(i));
        if (btn) {
            btn->setText(QString::number(preday));
            btn->setStyleSheet("border:none; color: rgb(200, 200, 200);");
            btn->setEnabled(false);
            preday++;
        }
    }

    /* 刷新右侧信息 */
    dateInfo();
}

int Calendar::getButtonIdByDay(int day)
{
    for (int id = 1; id <= 42; id++) {
        QPushButton *btn = qobject_cast<QPushButton*>(buttonGroup->button(id));
        if (btn && !btn->text().isEmpty()) {
            if (btn->isEnabled() && btn->text().toInt() == day) {   // 必须是本月，防止同一页出现两个相同日期
                return id;  // 找到日期对应的按钮ID
            }
        }
    }

    return -1;  //没找到
}

void Calendar::onCalendarButtonClicked(int id)
{
    /* 先把组里所有按钮恢复默认样式 */
    for (auto btn : buttonGroup->buttons()) {
        if (!btn->isEnabled()) continue;    // 如果按钮是禁用的，不修改，保持灰色
        btn->setStyleSheet("border: none"); // 只有本月按钮才恢复默认样式
    }

    /* 获取当前点击的按钮 */
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(buttonGroup->button(id));
    if (!clickedBtn) return;

    /* 改变选中的样式表 */
    clickedBtn->setStyleSheet(R"(
                                background-color: rgb(73, 128, 247);
                                color: white;
                                border: none;
                                border-radius: 10px;
                              )");
    /* 更新时间信息 */
    int day = clickedBtn->text().toUInt();
    dateInfo(day);

    /* 判断是否显示返回今日按钮 */
    if (day == todayInfo().date().day()) {
        ui->backTodayPushButton->setVisible(false);
    } else {
        ui->backTodayPushButton->setVisible(true);
    }
}

/* 上一月 */
void Calendar::on_lastMonthPushButton_clicked()
{
    g_currentDateTime = g_currentDateTime.addMonths(-1);
    updateCalendar();

    int day = g_currentDateTime.date().day();
    int dayId = getButtonIdByDay(day);
    if (dayId == -1) dayId = getButtonIdByDay(1);
    if (dayId != -1) onCalendarButtonClicked(dayId);

    /* 判断是否显示返回今日按钮 */
    if (g_currentDateTime.date().year() == todayInfo().date().year()
        && g_currentDateTime.date().month() == todayInfo().date().month()
        && g_currentDateTime.date().day() == todayInfo().date().day()) {
        ui->backTodayPushButton->setVisible(false);
    } else {
        ui->backTodayPushButton->setVisible(true);
    }
}

/* 下一月 */
void Calendar::on_nextMonthPushButton_clicked()
{
    g_currentDateTime = g_currentDateTime.addMonths(1);
    updateCalendar();

    int day = g_currentDateTime.date().day();
    int dayId = getButtonIdByDay(day);
    if (dayId == -1) dayId = getButtonIdByDay(1);
    if (dayId != -1) onCalendarButtonClicked(dayId);

    /* 判断是否显示返回今日按钮 */
    if (g_currentDateTime.date().year() == todayInfo().date().year()
        && g_currentDateTime.date().month() == todayInfo().date().month()
        && g_currentDateTime.date().day() == todayInfo().date().day()) {
        ui->backTodayPushButton->setVisible(false);
    } else {
        ui->backTodayPushButton->setVisible(true);
    }
}

/* 返回今日 */
void Calendar::on_backTodayPushButton_clicked()
{
    g_currentDateTime = todayInfo();
    updateCalendar();

    int todayId = getButtonIdByDay(todayInfo().date().day());
    if (todayId != -1) onCalendarButtonClicked(todayId);

    ui->backTodayPushButton->setVisible(false);
}
