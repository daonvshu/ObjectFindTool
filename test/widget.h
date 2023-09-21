#pragma once

#include <qwidget.h>
#include <qevent.h>

#include "ui_widget.h"
#include "ui_widget2.h"

class MyWidget : public QWidget {
public:
    MyWidget() {
        ui.setupUi(this);
    }

private:
    Ui::QtLogFilterClass ui;
};

class MyWidget2 : public QWidget {
public:
    MyWidget2() {
        ui.setupUi(this);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            lastPos = event->pos();
            target = childAt(lastPos);
            if (target != nullptr) {
                lastPos -= target->pos();
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton) {
            if (target != nullptr) {
                target->move(event->pos() - lastPos);
            }
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        target = nullptr;
    }

private:
    Ui::Form ui;
    QPoint lastPos;
    QWidget* target = nullptr;
};