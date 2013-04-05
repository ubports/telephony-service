file(GLOB QML_JS_FILES *.qml *.js)

set(QML_DIRS
    DetailViewContact
    DetailViewLiveCall
    DetailViewVoicemail
    DetailViewCommunication
    PanelDialer
    PanelContacts
    PanelCommunications
    Panes
    Widgets
    )

set(ASSETS_DIR assets)

install(FILES ${QML_JS_FILES} DESTINATION ${PHONE_APP_DIR})
install(DIRECTORY ${QML_DIRS} DESTINATION ${PHONE_APP_DIR})
install(DIRECTORY ${ASSETS_DIR} DESTINATION ${PHONE_APP_DIR})
