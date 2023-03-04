find_package(GTest REQUIRED)
enable_testing()

add_executable(csgopp.tests
        common/id_tests.cpp common/bits_tests.cpp common/ring_tests.cpp
        common/reader_tests.cpp client/data_table_tests.cpp)
target_link_libraries(csgopp.tests csgopp CONAN_PKG::gtest)

include(GoogleTest)
gtest_discover_tests(csgopp.tests)
