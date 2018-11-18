#
# microtcp, a lightweight implementation of TCP for teaching,
# and academic purposes.
#
# Copyright (C) 2015-2017  Manolis Surligas <surligas@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required (VERSION 2.6)
project (microtcp C CXX)

# Enable C++11 support
if (CMAKE_VERSION VERSION_LESS "3.1")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else ()
	set (CMAKE_CXX_STANDARD 11)
endif()

# Enable all warnings during compilation, hoping that this will be 
# helpful for the students :)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")

set (microtcp_version_major 1)
set (microtcp_version_minor 2.0)


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
