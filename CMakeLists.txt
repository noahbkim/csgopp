cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)

# Set the project name
project(csgopp)

# Include conan
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# External libraries
set(NB_SHARED OFF CACHE INTERNAL "")
add_subdirectory(libraries/nanobind)

# Main subdirectories
add_subdirectory(csgopp/object)
add_subdirectory(csgopp/messages/)
add_subdirectory(csgopp/csgopp/)
add_subdirectory(csgopp/csgopy)
add_subdirectory(csgopp/cli/)
add_subdirectory(csgopp/docs)

# Test subdirectories
add_subdirectory(tests/csgopp/)
add_subdirectory(tests/object/)

