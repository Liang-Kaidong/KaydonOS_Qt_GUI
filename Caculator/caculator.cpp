#include "caculator.h"
#include "ui_caculator.h"
#include <QList>
#include <QDebug>
#include <QtMath>
#include <QPushButton>
#include <QStack>

Caculator::Caculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Caculator)
{
    ui->setupUi(this);
    initUI();
}

Caculator::~Caculator()
{
    delete ui;
}

void Caculator::initUI()
{
    QList<QPushButton*> buttonList = findChildren<QPushButton*>();
    buttonGroup = new QButtonGroup(this);

    for(QPushButton *button : buttonList)
        buttonGroup->addButton(button);

    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(onButtonGroupClicked(QAbstractButton*)));

    ui->progressLineEdit->clear();
    ui->resultLineEdit->clear();
    justCalculated = false;
    is2ndMode = false;
    isRadMode = true;
    expression.clear();
    lastDisplay.clear();
}

bool Caculator::isOperator(const QString &op)
{
    return op=="+" || op=="-" || op=="×" || op=="÷" || op=="%" ||
           op=="(" || op==")" || op=="^" || op=="!";
}

void Caculator::onButtonGroupClicked(QAbstractButton *button)
{
    QString buttonText = button->text();
    bool opFlag = isOperator(buttonText);

    if(justCalculated)
    {
        bool isNumber = false;
        if(buttonText.size()==1)
        {
            QChar c = buttonText[0];
            if(c.isDigit() || c=='.') isNumber = true;
        }
        if(isNumber)
        {
            expression.clear();
            lastDisplay.clear();
        }
        else if(opFlag)
        {
            expression = ui->resultLineEdit->text();
            lastDisplay = expression;
        }
        justCalculated = false;
    }

    /* 2nd功能 */
    if(buttonText=="2nd")
    {
        is2ndMode = !is2ndMode;
        ui->sinPushButton->setText(is2ndMode?"arcsin":"sin");
        ui->cosPushButton->setText(is2ndMode?"arccos":"cos");
        ui->tanPushButton->setText(is2ndMode?"arctan":"tan");
        ui->progressLineEdit->setText(lastDisplay);
        return;
    }

    /* 数字和小数点 */
    if(buttonText.size()==1 && (buttonText[0].isDigit() || buttonText=="."))
    {
        expression += buttonText;
        lastDisplay += buttonText;
        ui->resultLineEdit->setText(ui->resultLineEdit->text() + buttonText);
    }
    /* 运算符 */
    else if(opFlag)
    {
        if(!expression.isEmpty())
        {
            QChar lastChar = expression.right(1)[0];
            if(isOperator(QString(lastChar)) && lastChar!='!')
            {
                expression.chop(1);
                lastDisplay.chop(1);
            }
        }
        expression += buttonText;
        lastDisplay += buttonText;
        ui->resultLineEdit->setText(buttonText);
    }
    /* π e */
    else if(buttonText=="π")
    {
        expression += QString::number(M_PI);
        lastDisplay += "π";
        ui->resultLineEdit->setText("π");
    }
    else if(buttonText=="e")
    {
        expression += QString::number(M_E);
        lastDisplay += "e";
        ui->resultLineEdit->setText("e");
    }
    /* 三角函数 */
    else if(buttonText=="sin" || buttonText=="arcsin" ||
            buttonText=="cos" || buttonText=="arccos" ||
            buttonText=="tan" || buttonText=="arctan")
    {
        QString func = buttonText;
        if(func.startsWith("arc")) func = func.mid(3); // arcsin->sin
        expression += func + "(";
        lastDisplay += buttonText + "(";
        ui->resultLineEdit->setText(buttonText);
    }
    /* 对数和平方根 */
    else if(buttonText=="lg" || buttonText=="ln")
    {
        expression += buttonText + "(";
        lastDisplay += buttonText + "(";
        ui->resultLineEdit->setText(buttonText);
    }
    else if(buttonText=="√x")
    {
        expression += "sqrt(";
        lastDisplay += "√x(";
        ui->resultLineEdit->setText("√x");
    }
    /* rad/deg */
    else if(button->objectName()=="radOrDegreePushButton")
    {
        isRadMode = !isRadMode;
        button->setText(isRadMode?"rad":"deg");
    }
    /* x^y */
    else if(buttonText=="x^y")
    {
        expression += "^";
        lastDisplay += "^";
        ui->resultLineEdit->setText("^");
    }
    /* ^(-1) */
    else if(buttonText=="^(-1)")
    {
        if(!expression.isEmpty())
        {
            expression += "POW_NEG1";
            lastDisplay += "^(-1)";
            ui->resultLineEdit->setText("^(-1)");
        }
    }
    /* 00 */
    else if(buttonText=="00")
    {
        expression += "00";
        lastDisplay += "00";
        ui->resultLineEdit->setText(ui->resultLineEdit->text() + "00");
    }
    /* AC清空 */
    else if(buttonText=="AC")
    {
        expression.clear();
        lastDisplay.clear();
        ui->progressLineEdit->clear();
        ui->resultLineEdit->clear();
    }
    /* =计算 */
    else if(buttonText=="=")
    {
        if(expression.isEmpty() || lastDisplay.endsWith('=')) return;
        try
        {
            double result = calculateExpression(expression);
            lastDisplay += "=";
            ui->resultLineEdit->setText(QString::number(result,'g',10));
        }
        catch(...)
        {
            ui->resultLineEdit->setText("计算出错！");
        }
        justCalculated = true;
    }

    /* 阶乘 */
    else if(buttonText == "x!")
    {
        if(!expression.isEmpty())
        {
            expression += "!";          // 加入表达式
            lastDisplay += "!";         // 显示为 !
            ui->resultLineEdit->setText("!");
        }
    }

    ui->progressLineEdit->setText(lastDisplay);
}

/* 阶乘 */
double Caculator::factorial(double n)
{
    if(n<0) throw std::runtime_error("阶乘不能为负数");
    if(n != floor(n)) throw std::runtime_error("阶乘只能作用于整数");
    double res = 1;
    for(int i=2;i<=int(n);i++)
        res *= i;
    return res;
}

/* 优先级 */
int Caculator::getPriority(const QString &op)
{
    if(op=="sin" || op=="cos" || op=="tan" || op=="lg" || op=="ln" || op=="sqrt") return 4;
    if(op=="!" || op=="^" || op=="POW_NEG1") return 3;
    if(op=="×" || op=="÷" || op=="%") return 2;
    if(op=="+" || op=="-") return 1;
    return 0;
}

/* 表达式转逆波兰 */
double Caculator::calculateExpression(const QString &exp)
{
    QStack<QString> opStack;
    QStringList output;
    QString num;

    for(int i=0;i<exp.size();i++)
    {
        QChar c = exp[i];
        if(c.isDigit() || c=='.')
            num += c;
        else
        {
            if(!num.isEmpty())
            {
                output << num;
                num.clear();
            }

            QString sub3 = exp.mid(i,3);
            if(sub3=="sin" || sub3=="cos" || sub3=="tan") { opStack.push(sub3); i+=2; continue; }
            QString sub2 = exp.mid(i,2);
            if(sub2=="lg" || sub2=="ln") { opStack.push(sub2); i+=1; continue; }
            QString sub4 = exp.mid(i,4);
            if(sub4=="sqrt") { opStack.push(sub4); i+=3; continue; }
            QString sub8 = exp.mid(i,8);
            if(sub8=="POW_NEG1") { opStack.push(sub8); i+=7; continue; }

            if(c=='(') opStack.push("(");
            else if(c==')')
            {
                while(!opStack.isEmpty() && opStack.top()!="(")
                    output << opStack.pop();
                if(!opStack.isEmpty()) opStack.pop();

                if(!opStack.isEmpty() && (opStack.top()=="sin"||opStack.top()=="cos"||opStack.top()=="tan"
                                        ||opStack.top()=="lg"||opStack.top()=="ln"||opStack.top()=="sqrt"))
                {
                    output << opStack.pop();
                }
            }
            else
            {
                QString op = c;
                if(op == " ") continue;

                while(!opStack.isEmpty() && getPriority(opStack.top()) >= getPriority(op))
                    output << opStack.pop();
                opStack.push(op);
            }
        }
    }

    if(!num.isEmpty()) output << num;
    while(!opStack.isEmpty()) output << opStack.pop();

    return evalRPN(output);
}

/* 逆波兰求值 */
double Caculator::evalRPN(const QStringList &tokens)
{
    QStack<double> st;
    for(QString t : tokens)
    {
        bool isNum;
        double val = t.toDouble(&isNum);
        if(isNum) {
            st.push(val);
        } else {
            if(t == "POW_NEG1") {
                if(st.isEmpty()) throw std::runtime_error("运算错误");
                double a = st.pop();
                st.push(pow(a, -1)); // 正确计算倒数
            }
            else if(t == "!") {
                if(st.isEmpty()) throw std::runtime_error("阶乘错误");
                double a = st.pop();
                st.push(factorial(a));
            }
            else if(t == "sin") {
                double a = st.pop();
                if(is2ndMode) { // 反正弦
                    double res = asin(a);
                    if(!isRadMode) res = res * 180.0 / M_PI;
                    st.push(res);
                } else { // 正弦
                    if(!isRadMode) a = a * M_PI / 180.0;
                    st.push(sin(a));
                }
            }
            else if(t == "cos") {
                double a = st.pop();
                if(is2ndMode) {
                    double res = acos(a);
                    if(!isRadMode) res = res * 180.0 / M_PI;
                    st.push(res);
                } else {
                    if(!isRadMode) a = a * M_PI / 180.0;
                    st.push(cos(a));
                }
            }
            else if(t == "tan") {
                double a = st.pop();
                if(is2ndMode) {
                    double res = atan(a);
                    if(!isRadMode) res = res * 180.0 / M_PI;
                    st.push(res);
                } else {
                    if(!isRadMode) a = a * M_PI / 180.0;
                    st.push(tan(a));
                }
            }
            else if(t == "lg") st.push(log10(st.pop()));
            else if(t == "ln") st.push(log(st.pop()));
            else if(t == "sqrt") st.push(sqrt(st.pop()));
            else if(t == "^") {
                double b = st.pop();
                double a = st.pop();
                st.push(pow(a, b));
            }
            else if(t == "%") {
                double b = st.pop();
                double a = st.pop();
                st.push(fmod(a, b));
            }
            else if(t == "+") {
                double b = st.pop();
                double a = st.pop();
                st.push(a + b);
            }
            else if(t == "-") {
                double b = st.pop();
                double a = st.pop();
                st.push(a - b);
            }
            else if(t == "×") {
                double b = st.pop();
                double a = st.pop();
                st.push(a * b);
            }
            else if(t == "÷") {
                double b = st.pop();
                double a = st.pop();
                st.push(a / b);
            }
        }
    }

    if(st.isEmpty()) throw std::runtime_error("表达式错误");
    return st.top();
}

void Caculator::resetAPP()
{
    /* 重置成员变量 */
    justCalculated = false;
    is2ndMode = false;
    isRadMode = true;
    expression.clear();
    lastDisplay.clear();

    /* 重置界面元素 */
    ui->progressLineEdit->clear();
    ui->resultLineEdit->clear();

    /* 如果需要，可以重置按钮状态，或其他UI元素的状态 */
    ui->sinPushButton->setText("sin");
    ui->cosPushButton->setText("cos");
    ui->tanPushButton->setText("tan");
}
