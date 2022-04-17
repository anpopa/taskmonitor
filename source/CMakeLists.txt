set(BINARY_NAME taskmonitor)
set(BINARY_SRC 
    ${CMAKE_SOURCE_DIR}/shared/Options.cpp
    ${CMAKE_SOURCE_DIR}/shared/EnvelopeReader.cpp
    ${CMAKE_SOURCE_DIR}/shared/EnvelopeWriter.cpp
    ${CMAKE_SOURCE_DIR}/shared/Helpers.cpp
    ActionManager.cpp
    ProcAcct.cpp
    ProcEvent.cpp
    Application.cpp
    ProcEntry.cpp
    Registry.cpp
    NetClient.cpp
    NetServer.cpp
    SysProcStat.cpp
    SysProcMeminfo.cpp
    SysProcPressure.cpp
    Main.cpp
    )
add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} ${LIBS} BSWInfra TaskMonitorInterfaces)
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin)
