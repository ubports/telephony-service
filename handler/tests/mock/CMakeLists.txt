include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

find_library(TELEPATHY_QT5_SERVICE_LIBRARIES telepathy-qt5-service)

set(mock_SRCS main.cpp protocol.cpp connection.cpp textchannel.cpp callchannel.cpp mockconnectiondbus.cpp)
qt5_add_dbus_adaptor(mock_SRCS MockConnection.xml mockconnectiondbus.h MockConnectionDBus)

add_executable(telepathy-mock ${mock_SRCS})
qt5_use_modules(telepathy-mock Core DBus)
target_link_libraries(telepathy-mock ${TP_QT5_LIBRARIES} ${TELEPATHY_QT5_SERVICE_LIBRARIES} ${OFONO_QT_LIBRARIES} ${PULSEAUDIO_LIBRARIES})
