set(BINARY_NAME taskmonitor)
set(BINARY_SRC 
    ../shared/Options.cpp
    ../shared/EnvelopeReader.cpp
    ../shared/EnvelopeWriter.cpp
    ../shared/Helpers.cpp
    Dispatcher.cpp
    ProcAcct.cpp
    ProcEvent.cpp
    Application.cpp
    ProcEntry.cpp
    ContextEntry.cpp
    ProcRegistry.cpp
    TCPCollector.cpp
    TCPServer.cpp
    UDSCollector.cpp
    UDSServer.cpp
    SysProcStat.cpp
    SysProcMemInfo.cpp
    SysProcPressure.cpp
    SysProcDiskStats.cpp
    SysProcBuddyInfo.cpp
    SysProcWireless.cpp
    Main.cpp
    )
add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} ${LIBS} BSWInfra TaskMonitorInterfaces)
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})
