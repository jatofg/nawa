cmake_minimum_required(VERSION 3.13)
project(nawa_docs)

include(../version.cmake)

set(NAWA_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

# configured files needed for docs generation
include(../configure.cmake)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(docs
            "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Generating Doxygen documentation" VERBATIM)
endif ()