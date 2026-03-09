cmake_minimum_required(VERSION 3.16)

project(Natural-Monitor-Brain VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql PrintSupport Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql PrintSupport Network)

set(NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT OFF)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/qcustomplot.h" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/qcustomplot.cpp")
    set(NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT ON)
endif()

option(NATURAL_MONITOR_USE_QCUSTOMPLOT "Use bundled QCustomPlot sources when available" ${NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT})
if(NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT)
    set(NATURAL_MONITOR_USE_QCUSTOMPLOT ON CACHE BOOL "Use bundled QCustomPlot sources when available" FORCE)
endif()

set(QCUSTOMPLOT_SOURCES)
if(NATURAL_MONITOR_USE_QCUSTOMPLOT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/qcustomplot.h" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/qcustomplot.cpp")
        list(APPEND QCUSTOMPLOT_SOURCES
            src/extern/qcustomplot.h
            src/extern/qcustomplot.cpp
        )
        add_compile_definitions(NATURAL_MONITOR_USE_QCUSTOMPLOT)
    else()
        message(WARNING "NATURAL_MONITOR_USE_QCUSTOMPLOT=ON, but src/extern/qcustomplot.h/.cpp were not found. Falling back to the built-in chart renderer.")
    endif()
endif()

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/languageutils.cpp
    src/languageutils.h
    src/databasemanager.cpp
    src/databasemanager.h
    src/logindialog.cpp
    src/logindialog.h
    src/linechartwidget.cpp
    src/linechartwidget.h
    src/themeutils.cpp
    src/themeutils.h
    src/common/moduletypes.h
    src/common/uistyles.cpp
    src/common/uistyles.h
    src/repositories/alarmrepository.cpp
    src/repositories/alarmrepository.h
    src/repositories/samplerepository.cpp
    src/repositories/samplerepository.h
    src/repositories/userrepository.cpp
    src/repositories/userrepository.h
    src/services/csvexporter.cpp
    src/services/csvexporter.h
    src/services/alarmservice.cpp
    src/services/alarmservice.h
    src/services/udpsensorclient.cpp
    src/services/udpsensorclient.h
    src/pages/realtimepagewidget.cpp
    src/pages/realtimepagewidget.h
    src/pages/historypagewidget.cpp
    src/pages/historypagewidget.h
    src/pages/alarmpagewidget.cpp
    src/pages/alarmpagewidget.h
    src/pages/exportpagewidget.cpp
    src/pages/exportpagewidget.h
    src/pages/settingspagewidget.cpp
    src/pages/settingspagewidget.h
    src/pages/usermanagementpagewidget.cpp
    src/pages/usermanagementpagewidget.h
    ${QCUSTOMPLOT_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Natural-Monitor-Brain
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Natural-Monitor-Brain APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Natural-Monitor-Brain SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Natural-Monitor-Brain
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Natural-Monitor-Brain PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Network
)

target_include_directories(Natural-Monitor-Brain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/src/repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extern
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Natural-Monitor-Brain)
endif()
set_target_properties(Natural-Monitor-Brain PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Natural-Monitor-Brain
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Natural-Monitor-Brain)
endif()
