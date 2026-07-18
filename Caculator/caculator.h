#ifndef CACULATOR_H
#define CACULATOR_H

#include <QWidget>
#include <QButtonGroup>
#include <QStack>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class Caculator; }
QT_END_NAMESPACE

class Caculator : public QWidget
{
    Q_OBJECT
public:
    Caculator(QWidget *parent = nullptr);
    ~Caculator();

    void resetAPP();

private slots:
    void onButtonGroupClicked(QAbstractButton *button);

private:
    Ui::Caculator *ui;
    QButtonGroup *buttonGroup;
    QString expression;    // 内部表达式
    QString lastDisplay;   // 显示表达式
    bool justCalculated;
    bool is2ndMode;
    bool isRadMode;

    void initUI();
    bool isOperator(const QString &op);
    double factorial(double n);
    int getPriority(const QString &op);
    double calculateExpression(const QString &exp);
    double evalRPN(const QStringList &tokens);
};

#endif // CACULATOR_H
