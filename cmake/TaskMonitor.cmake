message(STATUS "Adding TaskMonitor project")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BSWINFRA_DIR ${CMAKE_CURRENT_LIST_DIR}/../bswinfra)
set(INTERFACES_DIR ${CMAKE_CURRENT_LIST_DIR}/../interfaces)
set(CMAKE_MODULE_PATH "${BSWINFRA_DIR}/cmake" "${INTERFACES_DIR}/cmake")

option(WITH_SYSTEMD "Build with systemd watchdog and journald support" Y)
if(WITH_SYSTEMD)
    set(WITH_JOURNALD ON CACHE BOOL "Build with jounrald logger backend")
    add_compile_options("-DWITH_SYSTEMD")
else()
    set(WITH_SYSLOG ON CACHE BOOL "Build with syslog logger backend")
endif()

include(BSWInfra)
include(TaskMonitorInterfaces)

# Build time configuration setup
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(GIT_SHA1 "")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

set(TKM_CONFIG_FILE "/etc/taskmonitor.conf" CACHE PATH "Default config file path")

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/../shared/Defaults.h.in
    ${CMAKE_BINARY_DIR}/taskmonitor/shared/Defaults.h)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/../config/taskmonitor.conf.in
    ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.conf)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/../config/taskmonitor.service.in
    ${CMAKE_BINARY_DIR}/taskmonitor/taskmonitor.service)

# Header files
include_directories(${CMAKE_CURRENT_LIST_DIR}/../shared)
include_directories(${CMAKE_BINARY_DIR}/taskmonitor/shared)

# Dependencies
find_package          (PkgConfig REQUIRED)
find_package          (Threads REQUIRED)
set                   (LIBS ${LIBS} pthread)

# Use libnl
pkg_check_modules     (LIBNL libnl-3.0 REQUIRED)
include_directories   (${LIBNL_INCLUDE_DIRS})
set                   (LIBS ${LIBS} ${LIBNL_LIBRARIES})

# Use libnl-genl-3.0
pkg_check_modules     (LIBNLGENL libnl-genl-3.0 REQUIRED)
include_directories   (${LIBNLGENL_INCLUDE_DIRS})
set                   (LIBS ${LIBS} ${LIBNLGENL_LIBRARIES})

if(WITH_SYSTEMD)
    pkg_check_modules     (LIBSYSTEMD libsystemd REQUIRED)
    include_directories   (${LIBSYSTEMD_INCLUDE_DIRS})
    set                   (LIBS ${LIBS} ${LIBSYSTEMD_LIBRARIES})
endif()

# Build
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../config)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../source)

# Status reporting
message (STATUS "   CMAKE_BUILD_TYPE: "     ${CMAKE_BUILD_TYPE})
message (STATUS "   WITH_SYSTEMD: "         ${WITH_SYSTEMD})
