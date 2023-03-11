add_library(object STATIC
        source/object/type.cpp
        source/object/lens.cpp
        source/object/view.cpp
        )

set_target_properties(object PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(object PRIVATE cxx_std_20)

target_include_directories(object PUBLIC include)
target_link_libraries(object PUBLIC CONAN_PKG::abseil)

add_subdirectory(tests)
