cmake_minimum_required(VERSION 3.7.2)
project(TaskMonitor
        LANGUAGES CXX
        VERSION 1.6.4
)

set(CMAKE_PROJECT_HOMEPAGE_URL "https://gitlab.com/taskmonitor/taskmonitor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/bswinfra/cmake")

# options
option(WITH_STATIC_LIBS "Build with static libraries" N)
option(WITH_LXC "Build with LXC container support" N)
option(WITH_SYSTEMD "Build with systemd watchdog and journald support" Y)
option(WITH_STARTUP_DATA "Build with StartupData module support" Y)
option(WITH_PROC_EVENT "Enable ProcEvent module" Y)
option(WITH_PROC_ACCT "Enable ProcAcct module" Y)
option(WITH_VM_STAT "Enable SysProcVMStat module" Y)
option(WITH_WAKE_LOCK "Support PM wake locks for active TCP collectors" N)
option(WITH_INSTALL_CONFIG "Install default taskmonitor.conf on target" Y)
option(WITH_INSTALL_LICENSE "Install license file on target" Y)
option(WITH_TESTS "Build test suite" N)
option(WITH_TIDY "Build with clang-tidy" N)
option(WITH_ASAN "Build with address sanitize" N)
option(WITH_GCC_HARDEN_FLAGS "Build with GCC harden compile flags" N)

# static library paths
set(LIBTKM_STATIC_PATH "/usr/lib/libtkm.a" CACHE STRING "The libtaskmonitor static library path")
set(LIBPROTOBUF_STATIC_PATH "/usr/lib/libprotobuf.a" CACHE STRING "The libprotobuf static library path")
set(LIBNL3_STATIC_PATH "/usr/lib/libnl-3.a" CACHE STRING "The libnl3 static library path")
set(LIBNLGENL3_STATIC_PATH "/usr/lib/libnl-genl-3.a" CACHE STRING "The libnl-genl-3 static library path")
set(LIBLXC_STATIC_PATH "/usr/lib/liblxc.a" CACHE STRING "The liblxc static library path")
set(LIBSYSTEMD_STATIC_PATH "/usr/lib/libsystemd.a" CACHE STRING "The libsystemd static library path")

# config
if(WITH_PROC_EVENT)
    add_compile_options("-DWITH_PROC_EVENT")
endif()

if(WITH_PROC_ACCT)
    add_compile_options("-DWITH_PROC_ACCT")
endif()

if(WITH_VM_STAT)
    add_compile_options("-DWITH_VM_STAT")
endif()

if(WITH_WAKE_LOCK)
    add_compile_options("-DWITH_WAKE_LOCK")
endif()

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
if(EXISTS .git)
    execute_process(
        COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(PROJECT_VERSION_TWEAK ${GIT_SHA1})
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

if(WITH_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

if(WITH_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(WITH_GCC_HARDEN_FLAGS)
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
endif()

# cmake modules
include(GNUInstallDirs)
include(Packing)
include(BSWInfra)

# Add support for coverage analysis
if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    set(COVERAGE_EXCLUDES
        "*/bswinfra/*"
        "*/build/*"
        "*/tests/*"
        "*/source/Main.cpp"
        )
    set(COVERAGE_BASE_DIR ${CMAKE_SOURCE_DIR}/source)
    set(COVERAGE_BRANCH_COVERAGE ON)
    set(COVERAGE_THRESHOLD_LINE 80)
    set(COVERAGE_THRESHOLD_FUNCTION 80)

    include(CoverageTarget)
endif()

# generate
configure_file(
    ${CMAKE_SOURCE_DIR}/source/Defaults.h.in
    ${CMAKE_BINARY_DIR}/source/Defaults.h)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/taskmonitor.conf.in
    ${CMAKE_BINARY_DIR}/config/taskmonitor.conf)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/taskmonitor.service.in
    ${CMAKE_BINARY_DIR}/config/taskmonitor.service)

# Dependencies
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(tkm REQUIRED)

set(ENV{PKG_CONFIG_SYSROOT_DIR} "")

if(WITH_PROC_ACCT)
    pkg_check_modules     (LIBNL libnl-3.0 REQUIRED)
    pkg_check_modules     (LIBNLGENL libnl-genl-3.0 REQUIRED)
endif()

if(WITH_LXC)
    pkg_check_modules     (LIBLXC lxc>=3.0 REQUIRED)
    include_directories   (${LIBLXC_INCLUDE_DIRS})
    add_compile_options   ("-DWITH_LXC")
endif()

if(WITH_SYSTEMD)
    pkg_check_modules     (LIBSYSTEMD libsystemd REQUIRED)
    include_directories   (${LIBSYSTEMD_INCLUDE_DIRS})
    LIST(APPEND LIBS ${LIBSYSTEMD_LIBRARIES})
endif()

if(WITH_STARTUP_DATA)
    LIST(APPEND BINARY_SRC source/StartupData.cpp)
endif()

if(WITH_PROC_EVENT)
    LIST(APPEND BINARY_SRC source/ProcEvent.cpp)
endif()

if(WITH_VM_STAT)
    LIST(APPEND BINARY_SRC source/SysProcVMStat.cpp)
endif()

if(WITH_PROC_ACCT)
    LIST(APPEND BINARY_SRC source/ProcAcct.cpp)
    # workarounds to get absolute paths
    find_library(LIBNL_LIBRARIES_ABS ${LIBNL_LIBRARIES})
    find_library(LIBNLGENL_LIBRARIES_ABS ${LIBNLGENL_LIBRARIES})
endif()

if(WITH_LXC)
    find_library(LIBLXC_LIBRARIES_ABS ${LIBLXC_LIBRARIES})
endif()

include_directories(
    ${LIBNL_INCLUDE_DIRS}
    ${LIBNLGENL_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_BINARY_DIR}/source/
)

# binary
add_executable(taskmonitor
    source/Options.cpp
    source/Helpers.cpp
    source/Application.cpp
    source/ProcEntry.cpp
    source/ContextEntry.cpp
    source/ProcRegistry.cpp
    source/StateManager.cpp
    source/TCPCollector.cpp
    source/TCPServer.cpp
    source/UDSCollector.cpp
    source/UDSServer.cpp
    source/SysProcStat.cpp
    source/SysProcMemInfo.cpp
    source/SysProcPressure.cpp
    source/SysProcDiskStats.cpp
    source/SysProcBuddyInfo.cpp
    source/SysProcWireless.cpp
    source/Main.cpp
    ${BINARY_SRC}
)

if(WITH_STATIC_LIBS)
    set(LIBTKM_WRAP ${LIBTKM_STATIC_PATH})
    set(LIBPROTOBUF_WRAP ${LIBPROTOBUF_STATIC_PATH})
    if(WITH_PROC_ACCT)
        set(LIBNL3_WRAP ${LIBNL3_STATIC_PATH})
        set(LIBNLGENL3_WRAP ${LIBNLGENL3_STATIC_PATH})
    else()
        set(LIBNL3_WRAP "")
        set(LIBNLGENL3_WRAP "")
    endif()
    if(WITH_LXC)
        set(LIBLXC_WRAP ${LIBLXC_STATIC_PATH})
    else()
        set(LIBLXC_WRAP "")
    endif()
    if(WITH_SYSTEMD)
        set(LIBSYSTEMD_WRAP ${LIBSYSTEMD_STATIC_PATH})
    else()
        set(LIBSYSTEMD_WRAP "")
    endif()
else()
    set(LIBTKM_WRAP tkm::tkm)
    set(LIBPROTOBUF_WRAP ${PROTOBUF_LIBRARY})
    set(LIBNL3_WRAP ${LIBNL_LIBRARIES_ABS})
    set(LIBNLGENL3_WRAP ${LIBNLGENL_LIBRARIES_ABS})
    set(LIBLXC_WRAP ${LIBLXC_LIBRARIES_ABS})
    set(LIBSYSTEMD_WRAP ${LIBSYSTEMD_LIBRARIES})
endif()

target_link_libraries(taskmonitor
    PRIVATE
        BSWInfra
        pthread
        stdc++fs
        ${LIBTKM_WRAP}
        ${LIBPROTOBUF_WRAP}
        ${LIBNL3_WRAP}
        ${LIBNLGENL3_WRAP}
        ${LIBLXC_WRAP}
        ${LIBSYSTEMD_WRAP}
)

if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# install
install(TARGETS taskmonitor RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})

if(WITH_INSTALL_CONFIG)
    install(FILES
        ${CMAKE_BINARY_DIR}/config/taskmonitor.conf
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR})
endif()

if(WITH_SYSTEMD)
    install(FILES
        ${CMAKE_BINARY_DIR}/config/taskmonitor.service
        DESTINATION
        ${CMAKE_INSTALL_FULL_SYSCONFDIR}/systemd/system)
endif()

# Install license file
if(WITH_INSTALL_LICENSE)
    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX}/share/licenses/taskmonitor/)
endif()

# Status reporting
message (STATUS "CMAKE_BUILD_TYPE: "        ${CMAKE_BUILD_TYPE})
message (STATUS "WITH_STATIC_LIBS: "        ${WITH_STATIC_LIBS})
if(WITH_STATIC_LIBS)
    message (STATUS "   LIBTKM_STATIC_PATH: "        ${LIBTKM_STATIC_PATH})
    message (STATUS "   LIBPROTOBUF_STATIC_PATH: "   ${LIBPROTOBUF_STATIC_PATH})
    message (STATUS "   LIBNL3_STATIC_PATH: "        ${LIBNL3_STATIC_PATH})
    message (STATUS "   LIBNLGENL3_STATIC_PATH: "    ${LIBNLGENL3_STATIC_PATH})
    message (STATUS "   LIBLXC_STATIC_PATH: "        ${LIBLXC_STATIC_PATH})
    message (STATUS "   LIBSYSTEMD_STATIC_PATH: "    ${LIBSYSTEMD_STATIC_PATH})
endif()
message (STATUS "WITH_LXC: "                ${WITH_LXC})
message (STATUS "WITH_SYSTEMD: "            ${WITH_SYSTEMD})
message (STATUS "WITH_STARTUP_DATA: "       ${WITH_STARTUP_DATA})
message (STATUS "WITH_PROC_EVENT: "         ${WITH_PROC_EVENT})
message (STATUS "WITH_PROC_ACCT: "          ${WITH_PROC_ACCT})
message (STATUS "WITH_VM_STAT: "            ${WITH_VM_STAT})
message (STATUS "WITH_WAKE_LOCK: "          ${WITH_WAKE_LOCK})
message (STATUS "WITH_INSTALL_CONFIG: "     ${WITH_INSTALL_CONFIG})
message (STATUS "WITH_INSTALL_LICENSE: "    ${WITH_INSTALL_LICENSE})
message (STATUS "WITH_TESTS: "              ${WITH_TESTS})
message (STATUS "WITH_TIDY: "               ${WITH_TIDY})
message (STATUS "WITH_ASAN: "               ${WITH_ASAN})
message (STATUS "WITH_GCC_HARDEN_FLAGS: "   ${WITH_GCC_HARDEN_FLAGS})
