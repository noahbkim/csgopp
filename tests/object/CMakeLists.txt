find_package(GTest REQUIRED)
enable_testing()

add_executable(object.tests object_tests.cpp)
target_link_libraries(object.tests object CONAN_PKG::gtest)

include(GoogleTest)
gtest_discover_tests(object.tests)
