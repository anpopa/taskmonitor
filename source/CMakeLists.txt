set(BINARY_NAME taskmanager)
set(BINARY_SRC 
    ${CMAKE_SOURCE_DIR}/shared/Options.cpp
    ActionManager.cpp
    NetLink.cpp
    Application.cpp
    Main.cpp
    )
add_executable(${BINARY_NAME} ${BINARY_SRC})
target_link_libraries(${BINARY_NAME} BSWInfra)
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION bin)
