#ifndef BEEPCONTROL_H
#define BEEPCONTROL_H

#include <QObject>
#include <QProcess>

class BeepControl : public QObject
{
    Q_OBJECT
public:
    explicit BeepControl(QObject *parent = nullptr);

    /* 控制蜂鸣器开关 */
    void setBeep(bool on);

    /* 返回当前状态 */
    bool getState() const;

private:
    bool isOn;
};

#endif // BEEPCONTROL_H
