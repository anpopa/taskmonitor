if(WITH_INSTALL_CONFIG)
    install(FILES
        ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.conf
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR})
endif()

if(WITH_SYSTEMD)
    install(FILES
        ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.service
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR}/systemd/system)
endif()
