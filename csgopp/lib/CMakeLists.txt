add_library(csgopp STATIC
        demo.cpp demo.hpp)
target_include_directories(csgopp PUBLIC .)
target_compile_features(csgopp PRIVATE cxx_std_17)
target_link_libraries(csgopp csgopp.msg ${CONAN_LIBS})
