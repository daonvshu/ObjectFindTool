cmake_minimum_required (VERSION 3.8)
project(objectfindtool)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH F:/Qt/5.15.1/msvc2019_64)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(${PROJECT_NAME}-test WIN32
        test/main.cpp

        test/widget.h
        test/widget.cpp
        test/widget.ui  #copied from QtLogFilter

        src/objectfindtool.h
        src/objectfindtool.cpp
)

target_link_libraries(${PROJECT_NAME}-test
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)