#include "objectfindtool.h"

#include <qapplication.h>
#include <QClipboard>

#include "qdebug.h"
#include <qpainter.h>
#include <qevent.h>
#include <qdatetime.h>

ObjectFinderMaskWidget::ObjectFinderMaskWidget(QWidget* parent)
    : QWidget(parent)
    , targetWidget(nullptr)
    , lastCopiedTime(0)
    , lastCopiedTag(nullptr)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void ObjectFinderMaskWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setPen(QPen(displayColor, 1, Qt::DotLine));

    if (targetWidget != nullptr) {

        auto parentWidget = topParentWidget();

        auto widgetTopLeft = targetWidget->mapTo(parentWidget, QPoint(0, 0));
        auto tagRect = targetWidget->rect();
        tagRect.moveTopLeft(widgetTopLeft);

        painter.drawRect(tagRect);

        auto parentRect = parentWidget->rect();
        painter.drawRect(parentRect.adjusted(0, 0, -1, -1));

        auto name = targetWidget->objectName();
        if (name.isEmpty()) {
            name = "<empty>";
        }
        if (QDateTime::currentMSecsSinceEpoch() - lastCopiedTime < 3000 && lastCopiedTag == targetWidget) {
            name += " [copied!]";
        }
        auto nameRect = fontMetrics().boundingRect(name).adjusted(-2, -2, 2, 2);
        if (widgetTopLeft.y() - nameRect.height() > 0) {
            nameRect.moveBottomLeft(widgetTopLeft);
        } else {
            nameRect.moveBottomLeft(QPoint(widgetTopLeft.x(), widgetTopLeft.y() + tagRect.height() + nameRect.height()));
        }
        if (nameRect.right() > parentRect.right()) {
            nameRect.moveRight(widgetTopLeft.x());
        }

        painter.fillRect(nameRect, Qt::gray);
        painter.drawText(nameRect, Qt::AlignCenter, name);
    }
}

QWidget* ObjectFinderMaskWidget::topParentWidget() const{
    if (targetWidget == nullptr) {
        return nullptr;
    }
    QWidget* parent = targetWidget;
    while (parent->parentWidget() != nullptr) {
        parent = parent->parentWidget();
    }
    return parent;
}

void ObjectFinderMaskWidget::objectNameCopyToClipboard() {
    if (targetWidget == nullptr) {
        return;
    }
    qApp->clipboard()->setText(targetWidget->objectName());
    lastCopiedTime = QDateTime::currentMSecsSinceEpoch();
    lastCopiedTag = targetWidget;
    update();
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

        if (keyEvent->key() == shortcut) {
            findObjectMode = !findObjectMode;

            if (findObjectMode) {
                auto curWidget = qApp->activeWindow();
                maskWidget->setGeometry(curWidget->geometry());
            }
            maskWidget->targetWidget = nullptr;
            maskWidget->setVisible(findObjectMode);

            return true;
        }
        if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers() & Qt::ControlModifier) {
            if (findObjectMode) {
                maskWidget->objectNameCopyToClipboard();
            }
        }

    } else if (e->type() == QEvent::MouseMove) {
        if (findObjectMode) {
            auto target = qApp->widgetAt(QCursor::pos());
            maskWidget->targetWidget = target;
            maskWidget->update();
        }
    } else if (e->type() == QEvent::Move || e->type() == QEvent::Resize) {
        if (findObjectMode) {
            auto curWidget = qApp->activeWindow();
            if (curWidget != nullptr) {
                maskWidget->setGeometry(curWidget->geometry());
            }
        }
    } else if (e->type() == QEvent::Close) {
        if (findObjectMode && receiver == maskWidget->topParentWidget()) {
            maskWidget->targetWidget = nullptr;
            maskWidget->update();
        }
    } else if (e->type() == QEvent::ActivationChange) {
        if (findObjectMode) {
            auto curWidget = qApp->activeWindow();
            if (curWidget != nullptr && curWidget->objectName() != maskWidget->objectName()) {
                maskWidget->setGeometry(curWidget->geometry());
            }
        }
    }
    return QApplication::notify(receiver, e);
}
