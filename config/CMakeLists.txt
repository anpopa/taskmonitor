install(FILES ${CMAKE_BINARY_DIR}/config/taskmonitor.conf DESTINATION /etc)
install(FILES ${CMAKE_BINARY_DIR}/config/taskmonitor.service DESTINATION /etc/systemd/system)