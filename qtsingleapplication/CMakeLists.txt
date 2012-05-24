# qtsingleapplication from git://gitorious.org/qt-solutions/qt-solutions.git

set(qtsingleapplication_SRCS
    qtlocalpeer.cpp
    qtlockedfile.cpp
    qtlockedfile_unix.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
    )

set(qtsingleapplication_HDRS
    qtlocalpeer.h
    qtlockedfile.h
    qtsingleapplication.h
    qtsinglecoreapplication.h
    )

qt4_wrap_cpp(qtsingleapplication_MOC_SRCS ${qtsingleapplication_HDRS})

add_library(qtsingleapplication STATIC
    ${qtsingleapplication_SRCS}
    ${plugin_MOC_SRCS}
    )

target_link_libraries(qtsingleapplication
    ${QT_QTCORE_LIBRARIES}
    )
