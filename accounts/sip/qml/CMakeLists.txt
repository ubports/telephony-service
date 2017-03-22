file(GLOB QML_PLUGIN_FILES *.qml)

# The path (including plug-in name) where the QML files are installed
set(QML_PLUGIN_INSTALL_DIR share/accounts/qml-plugins/telephony-sip/)

# Do not install symbolic links
set (QML_PLUGIN_RESOLVED_FILES "")
foreach (QML_PLUGIN_FILE ${QML_PLUGIN_FILES})
    get_filename_component(resolvedFile "${QML_PLUGIN_FILE}" REALPATH)
    list (APPEND QML_PLUGIN_RESOLVED_FILES "${resolvedFile}")
endforeach()

install(FILES ${QML_PLUGIN_RESOLVED_FILES} DESTINATION ${QML_PLUGIN_INSTALL_DIR})
