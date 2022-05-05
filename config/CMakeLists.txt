install(FILES ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
if(WITH_SYSTEMD)
    install(FILES ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.service DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/systemd/system)
endif()
