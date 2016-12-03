############################################################################
# Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.1)
project(xproperty)

set(XPROPERTY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versionning
# ===========

file(STRINGS "${XPROPERTY_INCLUDE_DIR}/xproperty/xproperty_config.hpp" xproperty_version_defines
     REGEX "#define XPROPERTY_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xproperty_version_defines})
    if(ver MATCHES "#define XPROPERTY_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XPROPERTY_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION 
    ${XPROPERTY_VERSION_MAJOR}.${XPROPERTY_VERSION_MINOR}.${XPROPERTY_VERSION_PATCH})
message(STATUS "xproperty v${${PROJECT_NAME}_VERSION}")


# Build
# =====

set(XPROPERTY_HEADERS
    ${XPROPERTY_INCLUDE_DIR}/xproperty/xproperty.hpp
    ${XPROPERTY_INCLUDE_DIR}/xproperty/xobserved.hpp
    ${XPROPERTY_INCLUDE_DIR}/xproperty/xproperty_config.hpp
)

add_subdirectory(test)

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(FILES ${XPROPERTY_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xproperty)

# GNUInstallDirs "DATADIR" wrong here; CMake search path wants "share".
set(XPROPERTY_CMAKECONFIG_INSTALL_DIR "share/cmake/${PROJECT_NAME}" CACHE STRING "install path for xpropertyConfig.cmake")

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XPROPERTY_CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${XPROPERTY_CMAKECONFIG_INSTALL_DIR})

