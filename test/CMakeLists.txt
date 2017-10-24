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

include_directories(${MICROTCP_INCLUDE_DIRS})

add_executable(bandwidth_test bandwidth_test.c)
add_executable(traffic_generator_client traffic_generator_client.c)
add_executable(traffic_generator traffic_generator.cpp)
add_executable(test_microtcp_server test_microtcp_server.c)
add_executable(test_microtcp_client test_microtcp_client.c)

target_link_libraries(bandwidth_test microtcp)
target_link_libraries(test_microtcp_server microtcp)
target_link_libraries(test_microtcp_client microtcp)
target_link_libraries(traffic_generator microtcp)
target_link_libraries(traffic_generator_client microtcp)

install(TARGETS bandwidth_test DESTINATION bin)