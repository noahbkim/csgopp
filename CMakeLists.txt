cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)

# Set the project name
project(csgopp)

# Include conan
include(./conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# External libraries
set(NB_SHARED OFF CACHE INTERNAL "")
add_subdirectory(libraries/nanobind)

# Main subdirectories
add_subdirectory(objective/)
add_subdirectory(csgopp/)
add_subdirectory(csgopy)
add_subdirectory(csgod/)
#add_subdirectory(csgopp/docs)
