find_package(GTest REQUIRED)
enable_testing()

add_executable(object.tests test_object.cpp)
target_compile_features(object.tests PRIVATE cxx_std_20)
target_link_libraries(object.tests object CONAN_PKG::gtest)

include(GoogleTest)
gtest_discover_tests(object.tests)
