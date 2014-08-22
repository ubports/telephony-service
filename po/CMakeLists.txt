project(telephony-service-translations)

# for dh_translations to extract the domain
# (regarding syntax consistency, see http://pad.lv/1181187)
set (GETTEXT_PACKAGE "telephony-service")

include(FindGettext)

set(DOMAIN telephony-service)
set(POT_FILE ${DOMAIN}.pot)
file(GLOB PO_FILES *.po)
file(GLOB_RECURSE I18N_SRCS RELATIVE ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/approver/*.cpp
    ${CMAKE_SOURCE_DIR}/handler/*.cpp
    ${CMAKE_SOURCE_DIR}/indicator/*.desktop.in
    ${CMAKE_SOURCE_DIR}/indicator/*.cpp
    ${CMAKE_SOURCE_DIR}/Ubuntu/*.cpp
)

foreach(PO_FILE ${PO_FILES})
    get_filename_component(LANG ${PO_FILE} NAME_WE)
    gettext_process_po_files(${LANG} ALL PO_FILES ${PO_FILE})
    set(INSTALL_DIR ${CMAKE_INSTALL_LOCALEDIR}/${LANG}/LC_MESSAGES)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LANG}.gmo
            DESTINATION ${INSTALL_DIR}
            RENAME ${DOMAIN}.mo)
endforeach(PO_FILE)

find_program(XGETTEXT_EXECUTABLE xgettext)
if(XGETTEXT_EXECUTABLE)
    add_custom_target(${POT_FILE} ALL
        COMMENT "Generating translation template"
        COMMAND ${XGETTEXT_EXECUTABLE} --c++ --qt --add-comments=TRANSLATORS --keyword=GettextMarkExtraction --keyword=tr --keyword=tr:1,2 -D ${CMAKE_SOURCE_DIR} -s -p ${CMAKE_CURRENT_SOURCE_DIR} -o ${POT_FILE} ${I18N_SRCS}
    )

    add_custom_target(translations
        COMMENT "Updating translation files"
    )
    add_dependencies(translations ${POT_FILE})
    foreach(PO_FILE ${PO_FILES})
       add_custom_command(TARGET translations
            COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} ${PO_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/${POT_FILE} -o ${PO_FILE}
       )
    endforeach(PO_FILE)
endif()
