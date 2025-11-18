cmake_minimum_required(VERSION 3.16)
project(QtWidgetsApplication1 LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
    QtWidgetsApplication1.ui
    QtWidgetsApplication1.h
    QtWidgetsApplication1.cpp
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# 移除WIN32_EXECUTABLE TRUE属性，以便在macOS和Linux上正确编译
# 只在Windows平台上设置WIN32_EXECUTABLE
if(WIN32)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
)