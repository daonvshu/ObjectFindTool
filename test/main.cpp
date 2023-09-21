#include <QApplication>

#ifdef QT_DEBUG
#include "src/objectfindtool.h"
#endif

#include "test/widget.h"

int main(int argc, char* argv[]) {
#ifdef QT_DEBUG
    ObjectFinderApplication a(argc, argv);
#else
    QApplication a(argc, argv);
#endif

    MyWidget w;
    w.show();

    MyWidget2 w2;
    w2.show();

    return a.exec();
}
