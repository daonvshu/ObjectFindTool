#include "objectfindtool.h"

#include <qapplication.h>
#include <QClipboard>

#include "qdebug.h"
#include <qpainter.h>
#include <qevent.h>
#include <qdatetime.h>

ObjectFinderMaskWidget::ObjectFinderMaskWidget(QWidget* parent)
    : QWidget(parent)
    , activeWindow(nullptr)
    , targetWidget(nullptr)
    , compareTargetWidget(nullptr)
    , lastCopiedTime(0)
    , lastCopiedTag(nullptr)
{
    //Tool类型 + 无边框 + 透明 + 置顶显示 + 鼠标事件穿透
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

/**
 * 绘制焦点控件信息
 * @param event
 */
void ObjectFinderMaskWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    if (targetWidget != nullptr && isActiveWindowChild(targetWidget)) {

        painter.setPen(QPen(displayColor, 1, Qt::DotLine)); //用虚线绘制焦点控件位置

        //绘制目标控件相对顶级父控件位置
        auto widgetTopLeft = targetWidget->mapTo(activeWindow, QPoint(0, 0));
        auto tagRect = targetWidget->rect();
        tagRect.moveTopLeft(widgetTopLeft);
        painter.drawRect(tagRect);

        //绘制顶级父控件在当前屏幕上的位置
        auto parentRect = activeWindow->rect();
        painter.drawRect(parentRect.adjusted(0, 0, -1, -1));

        if (compareTargetWidget != nullptr && isActiveWindowChild(compareTargetWidget)) {
            auto compareRect = compareTargetWidget->rect();
            compareRect.moveTopLeft(compareTargetWidget->mapTo(activeWindow, QPoint(0, 0)));
            painter.drawRect(compareRect);

            if (compareTargetWidget != targetWidget) { //位置比较
                if (compareRect.contains(tagRect) || tagRect.contains(compareRect)) { //位置相交
                    drawInnerRectDistance(painter, compareRect, tagRect);
                } else {
                    drawRectDistance(painter, compareRect, tagRect);
                }
            }
        }

        if (compareTargetWidget == nullptr) { //比较模式下不绘制提示信息
            drawControlInfo(painter, tagRect);
        }
    }
}

/**
 * 绘制矩形相交时之间的距离
 * @param painter
 * @param r1
 * @param r2
 */
void ObjectFinderMaskWidget::drawInnerRectDistance(QPainter &painter, const QRect &r1, const QRect &r2) {

    QRect inner, outer;
    if (r1.contains(r2)) {
        inner = r2;
        outer = r1;
    } else {
        inner = r1;
        outer = r2;
    }

    auto tagCenter = inner.center();

    auto leftLine = QLine(outer.left(), tagCenter.y(), inner.left(), tagCenter.y());
    drawDistanceLine(painter, leftLine, Qt::Horizontal);

    auto topLine = QLine(tagCenter.x(), outer.top(), tagCenter.x(), inner.top());
    drawDistanceLine(painter, topLine, Qt::Vertical);

    auto rightLine = QLine(inner.right(), tagCenter.y(), outer.right(), tagCenter.y());
    drawDistanceLine(painter, rightLine, Qt::Horizontal);

    auto bottomLine = QLine(tagCenter.x(), inner.bottom(), tagCenter.x(), outer.bottom());
    drawDistanceLine(painter, bottomLine, Qt::Vertical);
}

/**
 * 绘制两个矩形之间的距离
 * @param painter
 * @param compare
 * @param target
 */
void ObjectFinderMaskWidget::drawRectDistance(QPainter &painter, const QRect &compare, const QRect &target) {

    //测试x轴方向
    auto baseX1 = compare.left();
    auto baseX2 = compare.right();
    auto baseY1 = compare.top();
    auto baseY2 = compare.bottom();

    auto testX1 = target.left();
    auto testX2 = target.right();
    auto testY1 = target.top();
    auto testY2 = target.bottom();

    QLine lineX1, lineX2, lineY1, lineY2;
    QList<QLine> assistLines; //辅助线

    //添加Y方向辅助线
    const auto& appendAssistLine = [&] (const QPoint& testPoint, const QRect& rect, Qt::Orientation orientation) {
        if (orientation == Qt::Horizontal) { //水平辅助线
            if (testPoint.x() > rect.right()) {
                assistLines.append(QLine(testPoint.x(), testPoint.y(), rect.right(), testPoint.y()));
            } else if (testPoint.x() < rect.left()) {
                assistLines.append(QLine(testPoint.x(), testPoint.y(), rect.left(), testPoint.y()));
            }
        } else {
            if (testPoint.y() > rect.bottom()) {
                assistLines.append(QLine(testPoint.x(), testPoint.y(), testPoint.x(), rect.bottom()));
            } else if (testPoint.y() < rect.top()) {
                assistLines.append(QLine(testPoint.x(), testPoint.y(), testPoint.x(), rect.top()));
            }
        }
    };

    if (baseX1 > testX2) { //左边
        //x设置为两个端点x，y设置为compare中心
        int y = compare.center().y();
        lineX1.setLine(testX2, y, baseX1, y);
        //测试是否需要垂直辅助线
        appendAssistLine(lineX1.p1(), target, Qt::Vertical);

    } else if (baseX2 < testX1) { //右边
        int y = compare.center().y();
        lineX2.setLine(testX1, y, baseX2, y);
        //测试是否需要垂直辅助线
        appendAssistLine(lineX2.p1(), target, Qt::Vertical);

    } else { //其他情况

        //判断左边距离线位置
        if (testX1 < baseX1) { //左端点在左边，y为compare中心
            int y = compare.center().y();
            lineX1.setLine(testX1, y, baseX1, y);

            //测试是否需要垂直辅助线
            appendAssistLine(lineX1.p1(), target, Qt::Vertical);

        } else if (testX1 > baseX1) { //左端点在右边，y为target中心
            int y = target.center().y();
            lineX1.setLine(baseX1, y, testX1, y);

            //测试是否需要垂直辅助线
            appendAssistLine(lineX1.p1(), compare, Qt::Vertical);

        } //x1坐标相同，不绘制左边距离

        //判断右边距离线位置，与左边同理
        if (testX2 < baseX2) {
            int y = target.center().y();
            lineX2.setLine(baseX2, y, testX2, y);

            //测试是否需要垂直辅助线
            appendAssistLine(lineX2.p1(), compare, Qt::Vertical);

        } else if (testX2 > baseX2) {
            int y = compare.center().y();
            lineX2.setLine(testX2, y, baseX2, y);

            //测试是否需要垂直辅助线
            appendAssistLine(lineX2.p1(), target, Qt::Vertical);

        } //x2坐标相同，不绘制右边距离
    }

    if (baseY1 > testY2) { //上边
        //y设置为两个端点y，x设置为compare中心
        int x = compare.center().x();
        lineY1.setLine(x, testY2, x, baseY1);
        //测试是否需要水平辅助线
        appendAssistLine(lineY1.p1(), target, Qt::Horizontal);

    } else if (baseY2 < testY1) { //下边
        int x = compare.center().x();
        lineY2.setLine(x, testY1, x, baseY2);
        //测试是否需要水平辅助线
        appendAssistLine(lineY2.p1(), target, Qt::Horizontal);

    } else { //其他情况

        //判断上边距离线位置
        if (testY1 < baseY1) {
            int x = compare.center().x();
            lineY1.setLine(x, testY1, x, baseY1);

            //测试是否需要水平辅助线
            appendAssistLine(lineY1.p1(), target, Qt::Horizontal);

        } else if (testY1 > baseY1) {
            int x = target.center().x();
            lineY1.setLine(x, baseY1, x, testY1);

            //测试是否需要水平辅助线
            appendAssistLine(lineY1.p1(), compare, Qt::Horizontal);

        } //y1坐标相同，不绘制左边距离

        //判断下边距离线位置，与上边同理
        if (testY2 < baseY2) {
            int x = target.center().x();
            lineY2.setLine(x, baseY2, x, testY2);

            //测试是否需要水平辅助线
            appendAssistLine(lineY2.p1(), compare, Qt::Horizontal);

        } else if (testY2 > baseY2) {
            int x = compare.center().x();
            lineY2.setLine(x, testY2, x, baseY2);

            //测试是否需要水平辅助线
            appendAssistLine(lineY2.p1(), target, Qt::Horizontal);

        } //y2坐标相同，不绘制右边距离
    }

    if (!lineX1.isNull()) {
        drawDistanceLine(painter, lineX1, Qt::Horizontal);
    }

    if (!lineX2.isNull()) {
        drawDistanceLine(painter, lineX2, Qt::Horizontal);
    }

    if (!lineY1.isNull()) {
        drawDistanceLine(painter, lineY1, Qt::Vertical);
    }

    if (!lineY2.isNull()) {
        drawDistanceLine(painter, lineY2, Qt::Vertical);
    }

    for (const auto& line: assistLines) {
        painter.drawLine(line);
    }
}

/**
 * 绘制相对距离线
 * @param painter
 * @param line
 * @param orientation 垂直线还是水平线
 */
void ObjectFinderMaskWidget::drawDistanceLine(QPainter& painter, const QLine &line, Qt::Orientation orientation) {
    //绘制线
    painter.drawLine(line);
    //绘制端点
    bool isHorizontal = orientation == Qt::Horizontal;
    painter.save();
    auto pen = painter.pen();
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    if (isHorizontal) { //水平线绘制垂直上下3px端点
        painter.drawLine(line.p1().x(), line.p1().y() - 3, line.p1().x(), line.p1().y() + 3);
        painter.drawLine(line.p2().x(), line.p2().y() - 3, line.p2().x(), line.p2().y() + 3);
    } else {
        painter.drawLine(line.p1().x() - 3, line.p1().y(), line.p1().x() + 3, line.p1().y());
        painter.drawLine(line.p2().x() - 3, line.p2().y(), line.p2().x() + 3, line.p2().y());
    }
    painter.restore();
    //绘制距离文字
    int distance = isHorizontal ? qAbs(line.p1().x() - line.p2().x()) : qAbs(line.p1().y() - line.p2().y());
    auto distanceName = QString::number(distance);
    auto textRect = fontMetrics().boundingRect(distanceName).adjusted(-2, -2, 2, 2);

    if (isHorizontal) { //水平方向上，在线中间绘制
        auto left = line.center().x() - textRect.width() / 2;
        left = qMax(left, 0);
        if (left + textRect.width() > this->width()) { //超出右边界对齐右边
            textRect.moveRight(this->width());
        } else {
            textRect.moveLeft(left);
        }

        int centerY = line.p1().y();
        if (centerY - textRect.height() > 0) { //上方空间足够
            textRect.moveBottom(centerY);
        } else {
            textRect.moveTop(centerY);
        }
    } else {
        auto top = line.center().y() - textRect.height() / 2;
        top = qMax(top, 0);
        if (top + textRect.height() > this->height()) { //超出下边界对齐下边
            textRect.moveBottom(this->height());
        } else {
            textRect.moveTop(top);
        }

        int centerX = line.p1().x();
        if (centerX + textRect.width() < this->width()) { //右边空间足够
            textRect.moveLeft(centerX);
        } else {
            textRect.moveRight(centerX);
        }
    }

    painter.fillRect(textRect, Qt::gray); //背景灰色
    painter.drawText(textRect, Qt::AlignCenter, distanceName); //区域居中绘制提示信息
}

/**
 * 绘制控件信息
 * @param painter
 * @param tagRect 控件位置
 */
void ObjectFinderMaskWidget::drawControlInfo(QPainter& painter, const QRect &tagRect) {
    //绘制对象名和大小信息
    auto name = targetWidget->objectName();
    if (name.isEmpty()) {
        name = "<empty>";
    }
    auto widgetTopLeft = tagRect.topLeft();

    //左上角位置和控件大小
    name += QString("(tl:%1,%2 size:%3x%4)")
            .arg(widgetTopLeft.x()).arg(widgetTopLeft.y())
            .arg(tagRect.width()).arg(tagRect.height());

    //3s内进行了复制，加上一个提示信息
    if (QDateTime::currentMSecsSinceEpoch() - lastCopiedTime < 3000 && lastCopiedTag == targetWidget) {
        name += " [copied!]";
    }

    //测量绘制提示信息需要的大小，如果超出绘制边界，将其移动到绘制区域内
    auto nameRect = fontMetrics().boundingRect(name).adjusted(-2, -2, 2, 2);
    if (widgetTopLeft.y() - nameRect.height() > 0) { //检测上方是否有足够空间绘制
        nameRect.moveBottomLeft(widgetTopLeft);
    } else { //上方不够移动到下方绘制
        nameRect.moveBottomLeft(
                QPoint(widgetTopLeft.x(), widgetTopLeft.y() + tagRect.height() + nameRect.height()));
    }
    if (nameRect.right() > this->width()) { //如果右方没有足够空间绘制，则对齐到控件右边绘制
        nameRect.moveRight(widgetTopLeft.x());
    }

    painter.fillRect(nameRect, Qt::gray); //背景灰色
    painter.drawText(nameRect, Qt::AlignCenter, name); //区域居中绘制提示信息
}

/**
 * 检查是否是当前激活窗口的子控件
 * @return 顶级父控件对象指针
 */
bool ObjectFinderMaskWidget::isActiveWindowChild(QWidget* target) {
    if (target == nullptr || activeWindow == nullptr) {
        return false;
    }
    QWidget* parent = target;
    while (parent->parentWidget() != nullptr) {
        parent = parent->parentWidget();
        if (parent == activeWindow) {
            return true;
        }
    }
    return false;
}

/**
 * 复制焦点控件对象名到粘贴板，并刷新提示信息
 */
void ObjectFinderMaskWidget::objectNameCopyToClipboard() {
    if (targetWidget == nullptr) {
        return;
    }
    qApp->clipboard()->setText(targetWidget->objectName());
    lastCopiedTime = QDateTime::currentMSecsSinceEpoch();
    lastCopiedTag = targetWidget;
    update();
}

/**
 * 固定当前焦点控件位置，用于比较下一个控件的相对位置
 */
void ObjectFinderMaskWidget::pinToCompare(bool toPin) {
    if (!toPin) {
        compareTargetWidget = nullptr;
        update();
    } else {
        if (targetWidget != nullptr) { //前提是当前位置有焦点控件
            if (compareTargetWidget == nullptr) { //只管第一次标记
                compareTargetWidget = targetWidget;
                update();
            }
        }
    }
}

ObjectFinderApplication::ObjectFinderApplication(int& argc, char** argv, const Qt::Key& shortcut, const QColor& color)
    : QApplication(argc, argv)
    , findObjectMode(false)
    , shortcut(shortcut)
{
    maskWidget = new ObjectFinderMaskWidget;
    maskWidget->setObjectName("_objectFindTool_MaskWidget");
    maskWidget->setVisible(false);
    maskWidget->displayColor = color;
}

ObjectFinderApplication::~ObjectFinderApplication() {
    delete maskWidget;
}

bool ObjectFinderApplication::notify(QObject* receiver, QEvent* e) {
    if (e->type() == QEvent::KeyPress) {
        auto keyEvent = dynamic_cast<QKeyEvent*>(e);

        //检查是否激活快捷键
        if (keyEvent->key() == shortcut) {
            switchFindMode();
            return true;
        }

        //ctrl+c复制焦点控件对象名
        if (findObjectMode) {
            if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers() & Qt::ControlModifier) {
                maskWidget->objectNameCopyToClipboard();
            }

            if (keyEvent->key() == Qt::Key_Alt) {
                maskWidget->pinToCompare(true);
            }
        }

    } else if (e->type() == QEvent::KeyRelease) {
        auto keyEvent = dynamic_cast<QKeyEvent*>(e);
        if (findObjectMode) {
            if (keyEvent->key() == Qt::Key_Alt) {
                maskWidget->pinToCompare(false);
            }
        }

    } else {
        if (findObjectMode) { //检查窗口和鼠标事件
            if (e->type() == QEvent::MouseMove) {
                setFocusWidget();
            } else if (e->type() == QEvent::Move || e->type() == QEvent::Resize) {
                resizeMaskWidget();
            } else if (e->type() == QEvent::Close) {
                testActiveWindowClosed(receiver);
            } else if (e->type() == QEvent::ActivationChange) {
                testActiveWindowChanged();
            }
        }
    }
    return QApplication::notify(receiver, e);
}

/**
 * 切换查找模式，如果开启查找，将提示控件设置到当前激活顶级窗口位置和大小
 */
void ObjectFinderApplication::switchFindMode() {
    findObjectMode = !findObjectMode;

    if (findObjectMode) {
        auto curWidget = activeWindow();
        maskWidget->activeWindow = curWidget;
        maskWidget->setGeometry(curWidget->geometry()); //设置标记控件到当前主窗口大小
    }
    maskWidget->targetWidget = nullptr; //清除焦点控件
    maskWidget->setVisible(findObjectMode);
}

/**
 * 鼠标移动时，切换焦点控件到鼠标当前位置控件
 */
void ObjectFinderApplication::setFocusWidget() {
    auto target = widgetAt(QCursor::pos());
    if (maskWidget->isActiveWindowChild(target)) {
        maskWidget->targetWidget = target;
        maskWidget->update();
    }
}

/**
 * 窗口发生移动和大小变化，重新设置提示控件大小
 */
void ObjectFinderApplication::resizeMaskWidget() {
    auto curWidget = activeWindow();
    if (curWidget != nullptr) {
        maskWidget->setGeometry(curWidget->geometry());
    }
}

/**
 * 如果当前窗口关闭，提示控件清除标记
 * @param receiver
 */
void ObjectFinderApplication::testActiveWindowClosed(QObject *receiver) {
    if (receiver == maskWidget->activeWindow) {
        maskWidget->targetWidget = nullptr;
        maskWidget->update();
    }
}

/**
 * 激活窗口发生变化，重置提示控件位置
 */
void ObjectFinderApplication::testActiveWindowChanged() {
    auto curWidget = activeWindow();
    if (curWidget != nullptr && curWidget->objectName() != maskWidget->objectName()) {
        maskWidget->activeWindow = curWidget;
        maskWidget->setGeometry(curWidget->geometry());
        if (curWidget->windowFlags().testFlag(Qt::WindowStaysOnTopHint)) {
            maskWidget->raise(); //提示窗口具有置顶属性，再次拉起提示控件到顶层
        }
    }
}
