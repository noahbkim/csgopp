set(CMAKE_BUILD_TYPE Release)

enable_testing()
find_package(GTest REQUIRED)

add_executable(csgopp.tests
        common/id_tests.cpp common/bits_tests.cpp common/ring_tests.cpp
        common/reader_tests.cpp common/hash_tests.cpp)
target_link_libraries(csgopp.tests csgopp GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(csgopp.tests)
