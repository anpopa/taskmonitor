cmake_minimum_required(VERSION 3.7.2)
project(TaskMonitor)

set(CMAKE_PROJECT_HOMEPAGE_URL "https://gitlab.com/taskmonitor/taskmonitor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BSWINFRA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bswinfra)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${BSWINFRA_DIR}/cmake")

message (STATUS "Module paths: ${CMAKE_MODULE_PATH}")

option(WITH_LXC "Build with LXC container support" N)
option(WITH_SYSTEMD "Build with systemd watchdog and journald support" Y)
option(WITH_STARTUP_DATA "Build with StartupData module support" Y)
option(WITH_INSTALL_CONFIG "Install default taskmonitor.conf on target" Y)
option(WITH_INSTALL_LICENSE "Install license file on target" Y)
option(WITH_TESTS "Build test suite" N)
option(WITH_TIDY "Build with clang-tidy" N)

if(WITH_STARTUP_DATA)
    add_compile_options("-DWITH_STARTUP_DATA")
endif()

if(WITH_SYSTEMD)
    set(WITH_JOURNALD ON CACHE BOOL "Build with journald logger backend")
    add_compile_options("-DWITH_SYSTEMD")
else()
    set(WITH_SYSLOG ON CACHE BOOL "Build with syslog logger backend")
endif()

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

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "5")
set(PROJECT_VERSION_PATCH ${GIT_SHA1})

include(GNUInstallDirs)
include(Packing)
include(BSWInfra)

# Add support for coverage analysis
if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    set(COVERAGE_EXCLUDES
        "*/bswinfra/*"
        "*/build/*"
        "*/tests/*"
        )
    set(COVERAGE_BASE_DIR ${CMAKE_SOURCE_DIR}/source)
    set(COVERAGE_BRANCH_COVERAGE ON)
    set(COVERAGE_THRESHOLD_LINE 90)
    set(COVERAGE_THRESHOLD_FUNCTION 90)

    include(CoverageTarget)
endif()

set(TKM_CONFIG_FILE "/etc/taskmonitor.conf" CACHE PATH "Default config file path")

configure_file(
    ${CMAKE_SOURCE_DIR}/source/Defaults.h.in
    ${CMAKE_BINARY_DIR}/taskmonitor/source/Defaults.h)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/taskmonitor.conf.in
    ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.conf)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/taskmonitor.service.in
    ${CMAKE_BINARY_DIR}/taskmonitor/config/taskmonitor.service)

if(WITH_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

# Header files
include_directories(${CMAKE_SOURCE_DIR}/source)
include_directories(${CMAKE_BINARY_DIR}/taskmonitor/source)

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

# Use protobuf library (dependency for libtaskmonitor)
include               (FindProtobuf)
find_package          (Protobuf REQUIRED)
set                   (LIBS ${LIBS} ${PROTOBUF_LIBRARY})

# Use libtaskmonitor
pkg_check_modules     (LIBTKM libtaskmonitor REQUIRED)
include_directories   (${LIBTKM_INCLUDE_DIRS})
set                   (LIBS ${LIBS} ${LIBTKM_LIBRARIES})

if(WITH_LXC)
    pkg_check_modules     (LIBLXC lxc>=3.0 REQUIRED)
    include_directories   (${LIBLXC_INCLUDE_DIRS})
    set                   (LIBS ${LIBS} ${LIBLXC_LIBRARIES})
    add_compile_options   ("-DWITH_LXC")
endif()

if(WITH_SYSTEMD)
    pkg_check_modules     (LIBSYSTEMD libsystemd REQUIRED)
    include_directories   (${LIBSYSTEMD_INCLUDE_DIRS})
    set                   (LIBS ${LIBS} ${LIBSYSTEMD_LIBRARIES})
endif()

# Build
add_subdirectory(config)
add_subdirectory(source)

if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Build flags
add_compile_options (
    -Wall
    -Wextra
    -Wno-unused-function
    -Wformat
    -Wno-variadic-macros
    -Wno-strict-aliasing
    -fstack-protector-strong
    -fwrapv
    -Wformat-signedness
    -Wmissing-include-dirs
    -Wimplicit-fallthrough=5
    -Wunused-parameter
    -Wuninitialized
    -Walloca
    -Wduplicated-branches
    -Wduplicated-cond
    -Wfloat-equal
    -Wshadow
    -Wcast-qual
    -Wconversion
    -Wsign-conversion
    -Wlogical-op
    -Werror
    -Wformat-security
    -Walloc-zero
    -Wcast-align
    -Wredundant-decls
    )

# Install license file
if(WITH_INSTALL_LICENSE)
    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX}/share/licenses/taskmonitor/)
endif()

# Status reporting
message (STATUS "CMAKE_BUILD_TYPE: "     ${CMAKE_BUILD_TYPE})
message (STATUS "WITH_LXC: "             ${WITH_LXC})
message (STATUS "WITH_SYSTEMD: "         ${WITH_SYSTEMD})
message (STATUS "WITH_STARTUP_DATA: "    ${WITH_STARTUP_DATA})
message (STATUS "WITH_INSTALL_CONFIG: "  ${WITH_INSTALL_CONFIG})
message (STATUS "WITH_INSTALL_LICENSE: " ${WITH_INSTALL_LICENSE})
message (STATUS "WITH_TESTS: "           ${WITH_TESTS})
message (STATUS "WITH_TIDY: "            ${WITH_TIDY})
