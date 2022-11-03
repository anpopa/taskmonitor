set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Performance monitoring service for embedded systems")
set(CPACK_VERBATIM_VARIABLES YES)

set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_BINARY_DIR}/packages")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_VENDOR "Alin Popa")
set(CPACK_PACKAGE_CONTACT "alin.popa@fxdata.ro")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# RPM
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_DEBUGINFO_PACKAGE OFF)
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_CHANGELOG_FILE "${CMAKE_SOURCE_DIR}/CHANGELOG")
set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_REQUIRES
  "libtaskmonitor-bin >= 1.0.3, systemd >= 243, libnl3 >= 3.4.0, protobuf >= 3.14.0")
list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/sbin")

# DEB
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_DEBUGINFO_PACKAGE OFF)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alin Popa")
set(CPACK_DEBIAN_PACKAGE_SECTION "Utilities")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
  "libtaskmonitor-bin (>=1.0.3), systemd (>= 243), libnl-3-200 (>= 3.4.0), libnl-genl-3-200 (>=3.4.0), libprotobuf17 (>=3.0.0)")

include(CPack)
