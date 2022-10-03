cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)

# set the project name
project(csgopp)

# include conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# external libraries
set(NB_SHARED OFF CACHE INTERNAL "")
add_subdirectory(libraries/nanobind)

# link compiled proto headers
add_subdirectory(csgopp/csgopp/)
add_subdirectory(csgopp/messages/)
add_subdirectory(csgopp/cli/)
add_subdirectory(csgopp/tests/)
add_subdirectory(csgopp/csgopy)
