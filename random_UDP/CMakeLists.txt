cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(random_udp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(RANDOM_UDP_SOURCES
    main.cpp
    SimulatorWidget.cpp
    SimulatorWidget.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(random_udp
        MANUAL_FINALIZATION
        ${RANDOM_UDP_SOURCES}
    )
else()
    if(ANDROID)
        add_library(random_udp SHARED
            ${RANDOM_UDP_SOURCES}
        )
    else()
        add_executable(random_udp
            ${RANDOM_UDP_SOURCES}
        )
    endif()
endif()

target_link_libraries(random_udp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

target_include_directories(random_udp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(random_udp)
endif()
