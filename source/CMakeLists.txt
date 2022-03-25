set(BINARY_NAME taskmonitor)
set(BINARY_SRC 
    ${CMAKE_SOURCE_DIR}/shared/Options.cpp
    JsonWriter.cpp
    ActionManager.cpp
    NetLinkStats.cpp
    NetLinkProc.cpp
    Application.cpp
    ProcEntry.cpp
    Registry.cpp
    SysProcStat.cpp
    SysProcPressure.cpp
    Main.cpp
    )
add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} ${LIBS} BSWInfra)
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin)
