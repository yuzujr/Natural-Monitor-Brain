cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Natural-Monitor-Brain VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql PrintSupport Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql PrintSupport Network)

set(NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT OFF)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/qcustomplot.h" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/qcustomplot.cpp")
    set(NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT ON)
endif()

option(NATURAL_MONITOR_USE_QCUSTOMPLOT "Use bundled QCustomPlot sources when available" ${NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT})
if(NATURAL_MONITOR_USE_QCUSTOMPLOT_DEFAULT)
    set(NATURAL_MONITOR_USE_QCUSTOMPLOT ON CACHE BOOL "Use bundled QCustomPlot sources when available" FORCE)
endif()

set(QCUSTOMPLOT_SOURCES)
if(NATURAL_MONITOR_USE_QCUSTOMPLOT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/qcustomplot.h" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/qcustomplot.cpp")
        list(APPEND QCUSTOMPLOT_SOURCES
            extern/qcustomplot.h
            extern/qcustomplot.cpp
        )
        add_compile_definitions(NATURAL_MONITOR_USE_QCUSTOMPLOT)

        # ========== 新增部分：解决 QCustomPlot 编译节数过多问题 ==========
        # 为 MinGW 编译器添加 -Wa,-mbig-obj 选项
        if(MINGW)
            set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/extern/qcustomplot.cpp
                PROPERTIES COMPILE_FLAGS "-Wa,-mbig-obj"
            )
        endif()
        # 为 MSVC 编译器添加 /bigobj 选项（备用）
        if(MSVC)
            set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/extern/qcustomplot.cpp
                PROPERTIES COMPILE_FLAGS "/bigobj"
            )
        endif()
        # ========== 新增部分结束 ==========

    else()
        message(WARNING "NATURAL_MONITOR_USE_QCUSTOMPLOT=ON, but extern/qcustomplot.h/.cpp were not found. Falling back to the built-in chart renderer.")
    endif()
endif()

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    languageutils.cpp
    languageutils.h
    databasemanager.cpp
    databasemanager.h
    logindialog.cpp
    logindialog.h
    linechartwidget.cpp
    linechartwidget.h
    themeutils.cpp
    themeutils.h
    common/moduletypes.h
    common/uistyles.cpp
    common/uistyles.h
    common/animatedtabwidget.cpp
    common/animatedtabwidget.h
    common/progressdialog.cpp
    common/progressdialog.h
    repositories/alarmrepository.cpp
    repositories/alarmrepository.h
    repositories/samplerepository.cpp
    repositories/samplerepository.h
    repositories/userrepository.cpp
    repositories/userrepository.h
    services/csvexporter.cpp
    services/csvexporter.h
    services/alarmservice.cpp
    services/alarmservice.h
    services/udpsensorclient.cpp
    services/udpsensorclient.h
    pages/realtimepagewidget.cpp
    pages/realtimepagewidget.h
    pages/historypagewidget.cpp
    pages/historypagewidget.h
    pages/alarmpagewidget.cpp
    pages/alarmpagewidget.h
    pages/exportpagewidget.cpp
    pages/exportpagewidget.h
    pages/settingspagewidget.cpp
    pages/settingspagewidget.h
    pages/usermanagementpagewidget.cpp
    pages/usermanagementpagewidget.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/pages
    ${CMAKE_CURRENT_SOURCE_DIR}/repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
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
