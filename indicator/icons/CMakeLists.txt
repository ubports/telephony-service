set (icon-theme Humanity)

set(icons
    notification-group-call.svg
    notification-unavailable-image-call.svg
    notification-unknown-call.svg
    )

install(FILES ${icons} DESTINATION share/notify-osd/icons/${icon-theme}/scalable/status)
