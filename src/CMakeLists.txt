# vi: ts=4 sw=4 tw=100 et

set(EXECUTABLE kbd-layout-sync)

set(SOURCES
    ${PROJECT_BINARY_DIR}/config.h
    main.cpp
    xkb_switch_lib.cpp
    xkb_switch_lib.h
    listener.cpp
    listener.h
    worker.cpp
    worker.h
    settings.cpp
    settings.h
    settings_window.cpp
    settings_window.h)

qt5_add_resources(SOURCES kbd-layout-sync.qrc)

if (HAS_X11)
    list(APPEND SOURCES
        sender.cpp
        sender.h)
endif()

add_executable(${EXECUTABLE} ${SOURCES})

# target_compile_options(${EXECUTABLE} PRIVATE "-fsanitize=address")
# target_link_libraries(${EXECUTABLE} PRIVATE asan)

if (HAS_X11)
    target_include_directories(${EXECUTABLE} PRIVATE ${X11_Xutil_INCLUDE_PATH} ${X11_Xkb_INCLUDE_PATH})
    target_link_libraries(${EXECUTABLE} PRIVATE ${X11_LIBRARIES})
endif()

target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(${EXECUTABLE} PRIVATE Qt5::Core Qt5::Widgets)
