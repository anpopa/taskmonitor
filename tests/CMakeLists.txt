include(GNUInstallDirs)

# GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/source)

set(MODULES_SRC
    ${CMAKE_SOURCE_DIR}/shared/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/Application.cpp
    ${CMAKE_SOURCE_DIR}/source/ActionManager.cpp
    ${CMAKE_SOURCE_DIR}/source/NetLink.cpp
    )

configure_file(
    ${CMAKE_SOURCE_DIR}/tests/assets/taskmonitor.conf
    ${CMAKE_BINARY_DIR}/tests/assets/taskmonitor.conf)

# Testcases
add_executable(gtest_application ${MODULES_SRC} gtest_application.cpp)
target_link_libraries(gtest_application
	${LIBS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES}
	BSWInfra)
add_test(NAME gtest_application WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND gtest_application)

if(WITH_DEBUG_DEPLOY)
    install(TARGETS gtest_application RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
