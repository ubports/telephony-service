set(icon_SIZES
    16x16
    24x24
    32x32
    48x48
    64x64
    128x128
    256x256
    )

install(DIRECTORY ${icon_SIZES} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor)
