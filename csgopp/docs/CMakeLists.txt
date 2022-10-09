find_package(Doxygen REQUIRED)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY)

# https://stackoverflow.com/questions/68193337/how-to-run-the-doxygen-binary-installed-by-its-conan-package-inside-a-cmake-scri
add_custom_target(csgopp.docs
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "generating documentation with doxygen"
        VERBATIM)
