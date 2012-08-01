set(icon_DIRS
    hicolor
    ubuntu-mono-dark
    ubuntu-mono-light
    )

install(DIRECTORY ${icon_DIRS} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
