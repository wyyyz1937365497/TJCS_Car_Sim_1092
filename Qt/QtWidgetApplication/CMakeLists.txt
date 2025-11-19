cmake_minimum_required(VERSION 3.14)
project(QtWidgetApplication VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(PROJECT_SOURCES
    main.cpp
    qtwidgetapplication.cpp
    qtwidgetapplication.h
    qtwidgetapplication.ui
    qtwidgetapplication.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtWidgetApplication
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(QtWidgetApplication
        ${PROJECT_SOURCES}
    )
endif()
find_package(Qt6 REQUIRED)
target_include_directories(QtWidgetApplication PRIVATE ${Qt6_INCLUDE_DIRS})
target_link_libraries(QtWidgetApplication PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(QtWidgetApplication PRIVATE Qt6::Core Qt6::Gui)

set(CMAKE_MACOSX_BUNDLE TRUE)
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QtWidgetApplication)
endif()
set_target_properties(QtWidgetApplication PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtWidgetApplication)
endif()
