#pragma once

#include <qwidget.h>

#include "ui_widget.h"

class MyWidget : public QWidget {
public:
    explicit MyWidget(QWidget* parent = nullptr);

private:
    Ui::QtLogFilterClass ui;
};