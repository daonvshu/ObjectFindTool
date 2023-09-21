cmake_minimum_required (VERSION 3.8)
project(objectfindtool)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
    #编译使用utf8文件
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(${PROJECT_NAME}-test WIN32
        test/main.cpp

        test/widget.h

        src/objectfindtool.h
        src/objectfindtool.cpp
)

target_link_libraries(${PROJECT_NAME}-test
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)