add_library(object STATIC
        code.cpp
        code.h
        layout.h
        object.cpp
        object.h)

set_target_properties(object PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(object PRIVATE cxx_std_20)

target_include_directories(object PUBLIC ..)
target_link_libraries(object PUBLIC CONAN_PKG::abseil)
