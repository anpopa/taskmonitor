include(GNUInstallDirs)

# GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/tests/dummy)
include_directories(${CMAKE_SOURCE_DIR}/source)

configure_file(
    ${CMAKE_SOURCE_DIR}/tests/assets/taskmonitor.conf
    ${CMAKE_BINARY_DIR}/tests/assets/taskmonitor.conf)

configure_file(
    ${CMAKE_SOURCE_DIR}/tests/assets/taskmonitor_var1.conf
    ${CMAKE_BINARY_DIR}/tests/assets/taskmonitor_var1.conf)

configure_file(
    ${CMAKE_SOURCE_DIR}/tests/assets/taskmonitor_var2.conf
    ${CMAKE_BINARY_DIR}/tests/assets/taskmonitor_var2.conf)

# Application module tests
set(APPLICATION_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEvent.cpp
    ${CMAKE_SOURCE_DIR}/source/Application.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/source/TCPCollector.cpp
    ${CMAKE_SOURCE_DIR}/source/TCPServer.cpp
    ${CMAKE_SOURCE_DIR}/source/UDSCollector.cpp
    ${CMAKE_SOURCE_DIR}/source/UDSServer.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcStat.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcMemInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcPressure.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcDiskStats.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcBuddyInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcWireless.cpp
    )
if(WITH_STARTUP_DATA)
    LIST(APPEND APPLICATION_TEST_SRCS ${CMAKE_SOURCE_DIR}/source/StartupData.cpp)
endif()
add_executable(GTestApplication ${APPLICATION_TEST_SRCS} GTestApplication.cpp)
target_link_libraries(GTestApplication
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
    ${LIBLXC_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestApplication WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestApplication)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestApplication RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# Options module tests
set(OPTIONS_TEST_SRCS ${CMAKE_SOURCE_DIR}/source/Options.cpp)
add_executable(GTestOptions ${OPTIONS_TEST_SRCS} GTestOptions.cpp)
target_link_libraries(GTestOptions
	BSWInfra
    pthread
    stdc++fs
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestOptions WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestOptions)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestOptions RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# ContextEntry module tests
set(CONTEXTENTRY_TEST_SRCS ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp)
add_executable(GTestContextEntry ${CONTEXTENTRY_TEST_SRCS} GTestContextEntry.cpp)
target_link_libraries(GTestContextEntry
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestContextEntry WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestContextEntry)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestContextEntry RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# Helpers module tests
set(HELPERS_TEST_SRCS ${CMAKE_SOURCE_DIR}/source/Helpers.cpp)
add_executable(GTestHelpers ${HELPERS_TEST_SRCS} GTestHelpers.cpp)
target_link_libraries(GTestHelpers
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestHelpers WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestHelpers)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestHelpers RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# ProcAcct module tests
set(PROCACCT_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    )
add_executable(GTestProcAcct ${PROCACCT_TEST_SRCS} GTestProcAcct.cpp)
target_link_libraries(GTestProcAcct
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestProcAcct WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestProcAcct)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestProcAcct RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# ProcEntry module tests
set(PROCENTRY_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    )
add_executable(GTestProcEntry ${PROCENTRY_TEST_SRCS} GTestProcEntry.cpp)
target_link_libraries(GTestProcEntry
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestProcEntry WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestProcEntry)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestProcEntry RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# ProcEvent module tests
set(PROCENTRY_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEvent.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    )
add_executable(GTestProcEvent ${PROCENTRY_TEST_SRCS} GTestProcEvent.cpp)
target_link_libraries(GTestProcEvent
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestProcEvent WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestProcEvent)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestProcEvent RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# ProcRegistry module tests
set(PROCREGISTRY_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEvent.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    )
add_executable(GTestProcRegistry ${PROCREGISTRY_TEST_SRCS} GTestProcRegistry.cpp)
target_link_libraries(GTestProcRegistry
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestProcRegistry WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestProcRegistry)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestProcRegistry RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# SysProcStat module tests
set(SYSPROCSTAT_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcStat.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestSysProcStat ${SYSPROCSTAT_TEST_SRCS} GTestSysProcStat.cpp)
target_link_libraries(GTestSysProcStat
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestSysProcStat WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestSysProcStat)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestSysProcStat RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# SysProcWireless module tests
set(SYSPROCWIRELESS_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcWireless.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestSysProcWireless ${SYSPROCWIRELESS_TEST_SRCS} GTestSysProcWireless.cpp)
target_link_libraries(GTestSysProcWireless
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestSysProcWireless WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestSysProcWireless)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestSysProcWireless RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# SysProcPressure module tests
set(SYSPROCPRESSURE_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcPressure.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestSysProcPressure ${SYSPROCPRESSURE_TEST_SRCS} GTestSysProcPressure.cpp)
target_link_libraries(GTestSysProcPressure
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestSysProcPressure WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestSysProcPressure)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestSysProcPressure RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# SysProcMemInfo module tests
set(SYSPROCMEMINFO_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcMemInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestSysProcMemInfo ${SYSPROCMEMINFO_TEST_SRCS} GTestSysProcMemInfo.cpp)
target_link_libraries(GTestSysProcMemInfo
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestSysProcMemInfo WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestSysProcMemInfo)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestSysProcMemInfo RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# SysProcDiskStats module tests
set(SYSPROCDISKSTATS_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcDiskStats.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestSysProcDiskStats ${SYSPROCDISKSTATS_TEST_SRCS} GTestSysProcDiskStats.cpp)
target_link_libraries(GTestSysProcDiskStats
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestSysProcDiskStats WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestSysProcDiskStats)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestSysProcDiskStats RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# SysProcBuddyInfo module tests
set(SYSPROCDBUDDYINFO_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcBuddyInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestSysProcBuddyInfo ${SYSPROCDBUDDYINFO_TEST_SRCS} GTestSysProcBuddyInfo.cpp)
target_link_libraries(GTestSysProcBuddyInfo
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestSysProcBuddyInfo WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestSysProcBuddyInfo)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestSysProcBuddyInfo RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# StartupData module tests
set(STARTUPDATA_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Collector.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Client.cpp
    )
add_executable(GTestStartupData ${STARTUPDATA_TEST_SRCS} GTestStartupData.cpp)
target_link_libraries(GTestStartupData
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestStartupData WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestStartupData)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestStartupData RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# TCPInterface module tests
set(TCPINTERFACE_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/TCPServer.cpp
    ${CMAKE_SOURCE_DIR}/source/TCPCollector.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEvent.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcStat.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcMemInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcPressure.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcDiskStats.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcBuddyInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcWireless.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Reader.cpp
    )
add_executable(GTestTCPInterface ${TCPINTERFACE_TEST_SRCS} GTestTCPInterface.cpp)
target_link_libraries(GTestTCPInterface
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestTCPInterface WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestTCPInterface)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestTCPInterface RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# UDSInterface module tests
set(UDSINTERFACE_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/source/Helpers.cpp
    ${CMAKE_SOURCE_DIR}/source/Options.cpp
    ${CMAKE_SOURCE_DIR}/source/UDSServer.cpp
    ${CMAKE_SOURCE_DIR}/source/UDSCollector.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcAcct.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEvent.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ContextEntry.cpp
    ${CMAKE_SOURCE_DIR}/source/ProcRegistry.cpp
    ${CMAKE_SOURCE_DIR}/source/StartupData.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcStat.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcMemInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcPressure.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcDiskStats.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcBuddyInfo.cpp
    ${CMAKE_SOURCE_DIR}/source/SysProcWireless.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Application.cpp
    ${CMAKE_SOURCE_DIR}/tests/dummy/Reader.cpp
    )
add_executable(GTestUDSInterface ${UDSINTERFACE_TEST_SRCS} GTestUDSInterface.cpp)
target_link_libraries(GTestUDSInterface
	BSWInfra
    pthread
    stdc++fs
    tkm::tkm
    ${PROTOBUF_LIBRARY}
    ${LIBLXC_LIBRARIES_ABS}
    ${LIBNL_LIBRARIES_ABS}
    ${LIBNLGENL_LIBRARIES_ABS}
	${GMOCK_LIBRARIES}
	${GTEST_LIBRARIES})
add_test(NAME GTestUDSInterface WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests COMMAND GTestUDSInterface)
if(WITH_DEBUG_DEPLOY)
    install(TARGETS GTestUDSInterface RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
