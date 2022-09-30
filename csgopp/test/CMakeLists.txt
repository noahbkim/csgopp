set(CMAKE_BUILD_TYPE Release)

enable_testing()
find_package(GTest REQUIRED)

add_executable(csgopp.tests
        demo_tests.cpp)
target_link_libraries(csgopp.tests csgopp GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(csgopp.tests)
