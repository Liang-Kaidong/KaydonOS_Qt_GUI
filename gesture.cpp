#include "gesture.h"
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QScrollArea>
#include <QScroller>
#include <QTextBrowser>
#include <QApplication>
#include <QLabel>
#include <QScrollBar>
#include <algorithm>
#include <QListWidget>
#include <QAbstractItemView>

Gesture::Gesture(QStackedWidget *stackedWidget, QObject *parent)
    : QObject(parent), targetStackedWidget(stackedWidget)
{
    if (!targetStackedWidget) {
        return;
    }

    /* 给堆叠窗口安装事件过滤器 */
    targetStackedWidget->installEventFilter(this);
    stackedWidgetWidth = targetStackedWidget->width();
    stackedWidgetHeight = targetStackedWidget->height();

    /* 创建动画屏蔽层，防止动画期间用户操作 */
    animationBlockerWidget = new QWidget(targetStackedWidget);
    animationBlockerWidget->setGeometry(targetStackedWidget->rect());
    animationBlockerWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    animationBlockerWidget->setAttribute(Qt::WA_NoSystemBackground);
    animationBlockerWidget->hide();
    animationBlockerWidget->raise();
}

void Gesture::addVerticalScrollWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    /* 如果是列表控件，开启触摸滑动 */
    if (QListWidget *listWidget = qobject_cast<QListWidget*>(widget)) {
        listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        QScroller::grabGesture(listWidget, QScroller::TouchGesture);
        QScroller::grabGesture(listWidget, QScroller::LeftMouseButtonGesture);
    }

    /* 如果是文本浏览控件，开启触摸滑动 */
    if (QTextBrowser *textBrowser = qobject_cast<QTextBrowser*>(widget)) {
        QScroller::grabGesture(textBrowser, QScroller::TouchGesture);
        QScroller::grabGesture(textBrowser, QScroller::LeftMouseButtonGesture);
    }

    /* 如果是滚动区域，给视口开启触摸滑动 */
    if (QScrollArea *scrollArea = qobject_cast<QScrollArea*>(widget)) {
        QWidget *viewport = scrollArea->viewport();
        if (viewport) {
            QScroller::grabGesture(viewport, QScroller::TouchGesture);
            QScroller::grabGesture(viewport, QScroller::LeftMouseButtonGesture);
        }
    }
}

void Gesture::addHorizontalScrollWidget(const QList<int> &pageList)
{
    allowHorizontalSwitchPageList = pageList;
    /* 排序，方便查找位置 */
    std::sort(allowHorizontalSwitchPageList.begin(), allowHorizontalSwitchPageList.end());
}

QRect Gesture::calculateSmallRect(const QRect &originalRect)
{
    int width = originalRect.width() / 10;
    int height = originalRect.height() / 10;
    int x = originalRect.center().x() - width;
    int y = originalRect.center().y() - height;
    return QRect(x, y, width * 2, height * 2);
}

bool Gesture::eventFilter(QObject *targetObject, QEvent *event)
{
    /* 窗口大小改变时更新缓存尺寸 */
    if (targetObject == targetStackedWidget && event->type() == QEvent::Resize) {
        stackedWidgetWidth = targetStackedWidget->width();
        stackedWidgetHeight = targetStackedWidget->height();
        if (animationBlockerWidget) {
            animationBlockerWidget->setGeometry(targetStackedWidget->rect());
        }
    }

    /* 动画运行中不处理事件 */
    if (isAnimationRunning) {
        return false;
    }

    QWidget *widget = qobject_cast<QWidget*>(targetObject);
    if (!widget) {
        return false;
    }

    /* 判断是否在可滚动控件内部 */
    bool inScroll = false;
    foreach (QWidget *w, verticalScrollWidgetSet) {
        if (w == widget || w->isAncestorOf(widget)) {
            inScroll = true;
            break;
        }
    }

    /* 当前页面不在允许横向滑动列表中，直接返回 */
    int currentIndex = targetStackedWidget->currentIndex();
    if (!allowHorizontalSwitchPageList.contains(currentIndex)) {
        return false;
    }

    /* 计算目标页面索引 */
    int pos = allowHorizontalSwitchPageList.indexOf(currentIndex);
    int targetIndex = -1;
    int totalCount = allowHorizontalSwitchPageList.size();

    if (pos != -1 && totalCount > 1) {
        if (pos == 0) {
            targetIndex = allowHorizontalSwitchPageList[1];
        } else if (pos == totalCount - 1) {
            targetIndex = allowHorizontalSwitchPageList[pos - 1];
        } else {
            if (lastDragDeltaX < 0) {
                targetIndex = allowHorizontalSwitchPageList[pos + 1];
            } else {
                targetIndex = allowHorizontalSwitchPageList[pos - 1];
            }
        }
    }

    /* 目标页面无效，返回 */
    if (targetIndex < 0 || targetIndex >= targetStackedWidget->count()) {
        return false;
    }

    QWidget *currentPage = targetStackedWidget->widget(currentIndex);
    QWidget *targetPage = targetStackedWidget->widget(targetIndex);
    int stackWidth = stackedWidgetWidth;

    /* 鼠标按下：记录起点 */
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        dragStartX = mouseEvent->globalX();
        dragStartY = mouseEvent->globalY();
        lastDragDeltaX = 0;
        isDragging = true;

        /* 初始化目标页面位置 */
        targetPage->setGeometry(targetStackedWidget->rect());
        if (currentIndex < targetIndex) {
            targetPage->move(stackWidth, 0);
        } else {
            targetPage->move(-stackWidth, 0);
        }
        targetPage->show();
        targetPage->raise();
    }
    /* 鼠标移动：跟随拖动 */
    else if (event->type() == QEvent::MouseMove && isDragging) {
        if (inScroll) {
            return false;
        }

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int deltaX = mouseEvent->globalX() - dragStartX;
        lastDragDeltaX = deltaX;

        /* 限制拖动范围 */
        if (deltaX < -stackWidth) {
            deltaX = -stackWidth;
        } else if (deltaX > stackWidth) {
            deltaX = stackWidth;
        }

        /* 移动两个页面 */
        currentPage->move(deltaX, 0);
        if (currentIndex < targetIndex) {
            targetPage->move(stackWidth + deltaX, 0);
        } else {
            targetPage->move(-stackWidth + deltaX, 0);
        }
        targetPage->show();
        targetPage->raise();
        return true;
    }
    /* 鼠标松开：判断是否切页或回弹 */
    else if (event->type() == QEvent::MouseButtonRelease && isDragging) {
        isDragging = false;
        if (qAbs(lastDragDeltaX) < 8) {
            return false;
        }

        bool canSwitch = false;
        SlideDirection direction;

        /* 判断是否满足切页条件 */
        if (lastDragDeltaX < -swipeThresholdValue && currentIndex < targetIndex) {
            direction = SlideRightToLeft;
            canSwitch = true;
        } else if (lastDragDeltaX > swipeThresholdValue && currentIndex > targetIndex) {
            direction = SlideLeftToRight;
            canSwitch = true;
        }

        if (canSwitch) {
            /* 执行切换动画 */
            startSwitchAnimation(currentIndex, targetIndex, direction);
        } else {
            /* 不满足条件，回弹动画 */
            QPropertyAnimation *animationCurrent = new QPropertyAnimation(currentPage, "pos");
            animationCurrent->setDuration(150);
            animationCurrent->setEndValue(QPoint(0, 0));
            animationCurrent->setEasingCurve(QEasingCurve::OutSine);

            QPropertyAnimation *animationTarget = new QPropertyAnimation(targetPage, "pos");
            animationTarget->setDuration(150);
            if (currentIndex < targetIndex) {
                animationTarget->setEndValue(QPoint(stackWidth, 0));
            } else {
                animationTarget->setEndValue(QPoint(-stackWidth, 0));
            }
            animationTarget->setEasingCurve(QEasingCurve::OutSine);

            QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
            group->addAnimation(animationCurrent);
            group->addAnimation(animationTarget);
            group->start(QAbstractAnimation::DeleteWhenStopped);
        }
        return true;
    }

    return false;
}

bool Gesture::startSwitchAnimation(int fromIndex, int toIndex, SlideDirection direction)
{
    if (isAnimationRunning) {
        return false;
    }

    QWidget *fromWidget = targetStackedWidget->widget(fromIndex);
    QWidget *toWidget = targetStackedWidget->widget(toIndex);
    if (!fromWidget || !toWidget) {
        return false;
    }

    isAnimationRunning = true;
    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    int width = stackedWidgetWidth;
    int height = stackedWidgetHeight;

    /* 离开页面动画 */
    QPropertyAnimation *animationFrom = new QPropertyAnimation(fromWidget, "pos");
    /* 进入页面动画 */
    QPropertyAnimation *animationTo = new QPropertyAnimation(toWidget, "pos");

    animationFrom->setDuration(180);
    animationTo->setDuration(180);
    animationFrom->setEasingCurve(QEasingCurve::OutSine);
    animationTo->setEasingCurve(QEasingCurve::OutSine);

    /* 根据方向设置动画终点 */
    if (direction == SlideLeftToRight) {
        animationTo->setEndValue(QPoint(0, 0));
        animationFrom->setEndValue(QPoint(width, 0));
    } else if (direction == SlideRightToLeft) {
        animationTo->setEndValue(QPoint(0, 0));
        animationFrom->setEndValue(QPoint(-width, 0));
    } else if (direction == SlideUpToDown) {
        animationTo->setEndValue(QPoint(0, 0));
        animationFrom->setEndValue(QPoint(0, height));
    } else if (direction == SlideDownToUp) {
        animationTo->setEndValue(QPoint(0, 0));
        animationFrom->setEndValue(QPoint(0, -height));
    }

    animationGroup->addAnimation(animationFrom);
    animationGroup->addAnimation(animationTo);
    animationFromWidget = fromWidget;
    animationToWidget = toWidget;

    /* 旧版信号槽语法 */
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onSwitchAnimationFinished()));
    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
    return true;
}

void Gesture::onSwitchAnimationFinished()
{
    if (!animationFromWidget || !animationToWidget) {
        return;
    }

    /* 隐藏离开页面，复位位置 */
    animationFromWidget->hide();
    animationFromWidget->move(0, 0);
    /* 切换到目标页面 */
    targetStackedWidget->setCurrentWidget(animationToWidget);
    isAnimationRunning = false;
    animationFromWidget = nullptr;
    animationToWidget = nullptr;
}

void Gesture::openApp(int pageIndex)
{
    if (isAnimationRunning || pageIndex == targetStackedWidget->currentIndex()) {
        return;
    }

    isAnimationRunning = true;
    animationBlockerWidget->show();
    animationBlockerWidget->raise();

    QWidget *targetWidget = targetStackedWidget->widget(pageIndex);
    /* 抓取页面图片 */
    QPixmap pixmap = targetWidget->grab();
    pixmap = pixmap.scaled(pixmap.size() / 8, Qt::KeepAspectRatio, Qt::FastTransformation);

    /* 创建动画图片标签 */
    animationOverlayLabel = new QLabel(targetStackedWidget);
    animationOverlayLabel->setAttribute(Qt::WA_TranslucentBackground);
    animationOverlayLabel->setPixmap(pixmap);
    animationOverlayLabel->setScaledContents(true);
    animationOverlayLabel->setGeometry(calculateSmallRect(targetStackedWidget->rect()));
    animationOverlayLabel->show();
    animationOverlayLabel->raise();

    /* 缩放动画 */
    QPropertyAnimation *animation = new QPropertyAnimation(animationOverlayLabel, "geometry");
    animation->setDuration(350);
    animation->setStartValue(animationOverlayLabel->geometry());
    animation->setEndValue(targetStackedWidget->rect());
    animation->setEasingCurve(QEasingCurve::OutCubic);

    animationToWidget = targetWidget;
    connect(animation, SIGNAL(finished()), this, SLOT(onOpenAppAnimationFinished()));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Gesture::onOpenAppAnimationFinished()
{
    targetStackedWidget->setCurrentWidget(animationToWidget);
    if (animationOverlayLabel) {
        animationOverlayLabel->deleteLater();
        animationOverlayLabel = nullptr;
    }
    animationBlockerWidget->hide();
    isAnimationRunning = false;
}

void Gesture::closeApp(int pageIndex)
{
    if (isAnimationRunning || pageIndex == targetStackedWidget->currentIndex()) {
        return;
    }

    isAnimationRunning = true;
    QWidget *currentWidget = targetStackedWidget->currentWidget();
    QWidget *targetWidget = targetStackedWidget->widget(pageIndex);

    animationBlockerWidget->show();
    animationBlockerWidget->raise();

    /* 抓取当前页面图片 */
    QPixmap pixmap = currentWidget->grab();
    pixmap = pixmap.scaled(pixmap.size() / 8, Qt::KeepAspectRatio, Qt::FastTransformation);

    animationOverlayLabel = new QLabel(targetStackedWidget);
    animationOverlayLabel->setAttribute(Qt::WA_TranslucentBackground);
    animationOverlayLabel->setPixmap(pixmap);
    animationOverlayLabel->setScaledContents(true);
    animationOverlayLabel->setGeometry(targetStackedWidget->rect());
    animationOverlayLabel->show();
    animationOverlayLabel->raise();

    currentWidget->hide();

    /* 缩小动画 */
    QPropertyAnimation *animation = new QPropertyAnimation(animationOverlayLabel, "geometry");
    animation->setDuration(350);
    animation->setStartValue(animationOverlayLabel->geometry());
    animation->setEndValue(calculateSmallRect(targetStackedWidget->rect()));
    animation->setEasingCurve(QEasingCurve::InCubic);

    animationToWidget = targetWidget;
    connect(animation, SIGNAL(finished()), this, SLOT(onCloseAppAnimationFinished()));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Gesture::onCloseAppAnimationFinished()
{
    if (!animationToWidget) {
        return;
    }

    targetStackedWidget->setCurrentWidget(animationToWidget);
    animationToWidget->show();

    if (animationOverlayLabel) {
        animationOverlayLabel->deleteLater();
        animationOverlayLabel = nullptr;
    }
    animationBlockerWidget->hide();
    isAnimationRunning = false;
}

void Gesture::slidePageLeftToRight(int pageIndex)
{
    if (!targetStackedWidget || isAnimationRunning) {
        return;
    }
    int current = targetStackedWidget->currentIndex();
    if (current == pageIndex) {
        return;
    }

    QWidget *widget = targetStackedWidget->widget(pageIndex);
    widget->setGeometry(targetStackedWidget->rect());
    widget->move(-stackedWidgetWidth, 0);
    widget->show();
    widget->raise();
    startSwitchAnimation(current, pageIndex, SlideLeftToRight);
}

void Gesture::slidePageRightToLeft(int pageIndex)
{
    if (!targetStackedWidget || isAnimationRunning) {
        return;
    }
    int current = targetStackedWidget->currentIndex();
    if (current == pageIndex) {
        return;
    }

    QWidget *widget = targetStackedWidget->widget(pageIndex);
    widget->setGeometry(targetStackedWidget->rect());
    widget->move(stackedWidgetWidth, 0);
    widget->show();
    widget->raise();
    startSwitchAnimation(current, pageIndex, SlideRightToLeft);
}

void Gesture::slidePageUpToDown(int pageIndex)
{
    if (!targetStackedWidget || isAnimationRunning) {
        return;
    }
    int current = targetStackedWidget->currentIndex();
    if (current == pageIndex) {
        return;
    }

    QWidget *widget = targetStackedWidget->widget(pageIndex);
    widget->setGeometry(targetStackedWidget->rect());
    widget->move(0, -stackedWidgetHeight);
    widget->show();
    widget->raise();
    startSwitchAnimation(current, pageIndex, SlideUpToDown);
}

void Gesture::slidePageDownToUp(int pageIndex)
{
    if (!targetStackedWidget || isAnimationRunning) {
        return;
    }
    int current = targetStackedWidget->currentIndex();
    if (current == pageIndex) {
        return;
    }

    QWidget *widget = targetStackedWidget->widget(pageIndex);
    widget->setGeometry(targetStackedWidget->rect());
    widget->move(0, stackedWidgetHeight);
    widget->show();
    widget->raise();
    startSwitchAnimation(current, pageIndex, SlideDownToUp);
}
