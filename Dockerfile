FROM debian:jessie
MAINTAINER Surligas Manolis <surligas@csd.uoc.gr>
MAINTAINER Antonios Chariton <daknob@daknob.net>

# Update Image
RUN apt-get update
RUN apt-get -y -q upgrade

# Install Developer Tools
RUN apt-get -y -q install binutils g++

# Install CMake
RUN apt-get -y -q install cmake

# Move source code
RUN mkdir /software
COPY CMakeLists.txt /software/CMakeLists.txt
COPY cmake_uninstall.cmake.in /software/cmake_uninstall.cmake.in
COPY lib /software/lib
COPY test /software/test
COPY utils /software/utils

# Compile Program
ENV CXX gcc
RUN cd /software && mkdir build && cd build && cmake .. && make

# Set which program to run
ENV RUNBIN test_microtcp_client

# Execute Program
CMD ["sh", "-c", "/software/build/test/${RUNBIN}"]
