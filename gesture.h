#ifndef GESTURE_H
#define GESTURE_H

#include <QObject>
#include <QStackedWidget>
#include <QSet>
#include <QWidget>
#include <QList>

class QLabel;

/*
    手势滑动与动画切换类
    功能：控制QStackedWidget实现左右/上下滑动、缩放打开关闭动画
    继承QObject以支持信号槽机制
*/
class Gesture : public QObject
{
    Q_OBJECT
public:
    explicit Gesture(QStackedWidget *stackedWidget, QObject *parent = nullptr);


    void openApp(int pageIndex);    // 打开页面，执行中心缩放动画 pageIndex：目标页面索引
    void closeApp(int pageIndex);   // 关闭页面，执行缩小动画 pageIndex：返回的目标页面索引
    void slidePageLeftToRight(int pageIndex);   // 从左向右滑入页面
    void slidePageRightToLeft(int pageIndex);   // 从右向左滑入页面
    void slidePageUpToDown(int pageIndex);      // 从上向下滑入页面
    void slidePageDownToUp(int pageIndex);      // 从下向上滑入页面
    void addVerticalScrollWidget(QWidget *widget);              // 注册可纵向滚动的控件, 内部自动开启触摸滑动与像素级滚动
    void addHorizontalScrollWidget(const QList<int> &pageList); // 注册允许横向滑动切换的页面列表, 只有这些页面可以左右滑动

protected:
    bool eventFilter(QObject *targetObject, QEvent *event) override;

private:
    /* 滑动方向枚举 */
    enum SlideDirection {
        SlideLeftToRight,   // 从左向右滑动
        SlideRightToLeft,   // 从右向左滑动
        SlideUpToDown,      // 从上向下滑动
        SlideDownToUp       // 从下向上滑动
    };

    /*
     * 统一页面切换动画
     * fromPageIndex：当前页面索引
     * toPageIndex：目标页面索引
     * direction：滑动方向
     * 返回值：动画是否成功启动
     */
    bool startSwitchAnimation(int fromPageIndex, int toPageIndex, SlideDirection direction);

    /*
     * 计算中心小矩形，用于缩放动画
     * originalRect：原始窗口大小
     * 返回值：中心小窗口矩形
     */
    QRect calculateSmallRect(const QRect &originalRect);

    QStackedWidget *targetStackedWidget = nullptr;  // 需要控制的堆叠窗口
    QLabel *animationOverlayLabel = nullptr;        // 缩放动画使用的覆盖图片标签
    QWidget *animationBlockerWidget = nullptr;      // 动画期间屏蔽用户操作的透明控件


    int dragStartX = 0;                 // 鼠标拖动起始X坐标
    int dragStartY = 0;                 // 鼠标拖动起始Y坐标
    int lastDragDeltaX = 0;             // 最后一次拖动的X偏移量
    int swipeThresholdValue = 50;       // 滑动触发切页的最小像素距离
    bool isAnimationRunning = false;    // 是否正在播放动画
    bool isDragging = false;            // 是否正在拖动页面


    QSet<QWidget*> verticalScrollWidgetSet;     // 存储可纵向滚动的控件集合
    QList<int> allowHorizontalSwitchPageList;   // 允许横向滑动切换的页面索引
    QWidget *animationFromWidget = nullptr;     // 动画来源页面（即将离开的页面）
    QWidget *animationToWidget = nullptr;       // 动画目标页面（即将进入的页面）

    int stackedWidgetWidth = 0;     // 缓存堆叠窗口宽度
    int stackedWidgetHeight = 0;    // 缓存堆叠窗口高度

private slots:

    void onSwitchAnimationFinished();   // 页面滑动动画结束槽函数
    void onOpenAppAnimationFinished();  // 打开动画结束槽函数
    void onCloseAppAnimationFinished(); // 关闭动画结束槽函数
};
#endif
