# ObjectFindTool
qt widget's object name find tool

![image](https://github.com/daonvshu/ObjectFindTool/blob/master/screenshot/p0.gif?raw=true)
## How to use
relace your application to `ObjectFinderApplication`
```cpp
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

    return a.exec();
}
```
- press key `F2(default)` to display current focus widget's object name
- press `Ctrl+C` to copy current focus widget's object name