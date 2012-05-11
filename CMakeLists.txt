project(telephony-prototype)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 REQUIRED)

include_directories(
    ${QT_INCLUDE_DIR}
    )

set(TELEPHONY_APP telephony-prototype)

add_executable(${TELEPHONY_APP} main.cpp)

target_link_libraries(${TELEPHONY_APP}
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARIES}
    )

