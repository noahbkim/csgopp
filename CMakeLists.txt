cmake_minimum_required(VERSION 3.23)

# set the project name
project(csgopp)

# include conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# add the executable
add_executable(csgopp csgopp/src/main.cpp)
target_link_libraries(csgopp ${CONAN_LIBS})
