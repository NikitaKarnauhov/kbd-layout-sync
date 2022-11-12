# vi: ts=4 sw=4 tw=100 et

cmake_minimum_required(VERSION 3.15)
project(kbd-layout-sync)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5Widgets REQUIRED)
find_package(X11 COMPONENTS Xutil Xkb)

if (X11_FOUND)
    set(HAS_X11 ON)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(ICON_IS_MASK OFF)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(ICON_IS_MASK ON)
endif()

configure_file(config.h.in config.h)

add_subdirectory(src)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    install(FILES kbd-layout-sync.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
    install(FILES kbd-layout-sync.svg DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps")
    install(TARGETS kbd-layout-sync BUNDLE DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    install(TARGETS kbd-layout-sync BUNDLE DESTINATION "${CMAKE_INSTALL_PREFIX}/Applications")
endif()
