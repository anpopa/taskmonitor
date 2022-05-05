message(STATUS "Adding TaskMonitor project")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# BaseSoftwareInfrastructure
set(BSWINFRA_DIR ${CMAKE_CURRENT_LIST_DIR}/../bswinfra)
# TaskMonitorInterfaces
set(INTERFACES_DIR ${CMAKE_CURRENT_LIST_DIR}/../interfaces)
set(CMAKE_MODULE_PATH "${BSWINFRA_DIR}/cmake" "${INTERFACES_DIR}/cmake")

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

if(WITH_SYSTEMD)
    add_compile_options("-DWITH_SYSTEMD")
endif()

# Header files
include_directories(${CMAKE_CURRENT_LIST_DIR}/../shared)
include_directories(${CMAKE_BINARY_DIR}/taskmonitor/shared)

# Dependencies
find_package          (PkgConfig REQUIRED)
find_package		  (Threads REQUIRED)
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
message (STATUS "   SYSTEM_TYPE: "          ${CMAKE_SYSTEM_NAME})
message (STATUS "   CMAKE_BUILD_TYPE: "     ${CMAKE_BUILD_TYPE})
message (STATUS "   WITH_SYSTEMD: "         ${WITH_SYSTEMD})
message (STATUS "   WITH_SYSLOG: "          ${WITH_SYSLOG})
message (STATUS "   WITH_JOURNALD: "        ${WITH_JOURNALD})
