add_library(objective STATIC
        source/objective/type/array.cpp
        source/objective/type/object.cpp
        source/objective/type/value.cpp
        source/objective/code.cpp
        source/objective/view.cpp
        )

find_package(absl REQUIRED)
target_link_libraries(objective PRIVATE abseil::abseil)

set_target_properties(objective PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(objective PRIVATE cxx_std_20)
target_include_directories(objective PUBLIC include)

add_subdirectory(tests)
