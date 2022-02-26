#pragma once

#include <qwidget.h>
#include <qapplication.h>

class ObjectFinderMaskWidget : public QWidget {
public:
     explicit ObjectFinderMaskWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* targetWidget;
    QColor displayColor;

    qint64 lastCopiedTime;
    void* lastCopiedTag;

    friend class ObjectFinderApplication;

private:
    QWidget* topParentWidget() const;

    void objectNameCopyToClipboard();
};

class ObjectFinderApplication : public QApplication {
public:
    explicit ObjectFinderApplication(int& argc, char** argv, const Qt::Key& shortcut = Qt::Key_F2, const QColor& color = Qt::red);
    ~ObjectFinderApplication() override;

protected:
    bool notify(QObject*, QEvent*) override;

private:
    Q_DISABLE_COPY(ObjectFinderApplication)

private:
    ObjectFinderMaskWidget* maskWidget;

    bool findObjectMode;

    Qt::Key shortcut;
};
