cmake_minimum_required (VERSION 2.6)
project (microtcp)

set (microtcp_version_major 1)
set (microtcp_version_minor 1.0)


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

set(MICROTCP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/utils CACHE INTERNAL "" FORCE)

add_subdirectory(lib)
add_subdirectory(test)
#add_subdirectory(utils) 
