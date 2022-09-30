cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)

# set the project name
project(csgopp)

# include conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# link compiled proto headers
add_subdirectory(csgopp/lib/)
add_subdirectory(csgopp/msg/)
add_subdirectory(csgopp/cli/)
add_subdirectory(csgopp/test/)
