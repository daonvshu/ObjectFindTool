#pragma once

#include <qwidget.h>
#include <qapplication.h>

/**
 * 信息标记控件，用于绘制焦点控件信息的控件
 */
class ObjectFinderMaskWidget : public QWidget {
public:
    explicit ObjectFinderMaskWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* activeWindow; //当前激活窗口
    QWidget* targetWidget; //当前焦点控件
    QWidget* compareTargetWidget; //用于位置比较的控件
    QColor displayColor; //提示信息显示颜色

    qint64 lastCopiedTime; //最后进行objectName复制的时间戳
    void* lastCopiedTag; //最后进行objectName复制的控件对象

    friend class ObjectFinderApplication;

private:
    void drawInnerRectDistance(QPainter& painter, const QRect& r1, const QRect& r2);

    void drawRectDistance(QPainter& painter, const QRect& compare, const QRect& target);

    void drawControlInfo(QPainter& painter, const QRect& tagRect);

    void drawDistanceLine(QPainter& painter, const QLine& line, Qt::Orientation orientation);

    bool isActiveWindowChild(QWidget* target);

    void objectNameCopyToClipboard();

    void pinToCompare(bool toPin);
};

/**
 * 焦点控件查找类，根据鼠标位置实时查找当前位置的控件
 */
class ObjectFinderApplication : public QApplication {
public:
    explicit ObjectFinderApplication(int& argc, char** argv, const Qt::Key& shortcut = Qt::Key_F2, const QColor& color = Qt::red);
    ~ObjectFinderApplication() override;

protected:
    bool notify(QObject*, QEvent*) override;

private:
    Q_DISABLE_COPY(ObjectFinderApplication)

private:
    ObjectFinderMaskWidget* maskWidget; //用于提示信息绘制

    bool findObjectMode; //是否开启了查找模式

    Qt::Key shortcut; //查找模式开启快捷键

private:
    void switchFindMode();
    void setFocusWidget();
    void resizeMaskWidget();
    void testActiveWindowClosed(QObject* receiver);
    void testActiveWindowChanged();
};
