cmake_minimum_required(VERSION 3.16)

project(buteo-syncfw-doc)

find_package(Doxygen REQUIRED)

if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is required to build documentation")
endif()

file(GLOB_RECURSE DOC_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.dox")

add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/html/
        DESTINATION /usr/share/doc/buteo-syncfw-doc/
        OPTIONAL)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/buteo-syncfw.tag)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/buteo-syncfw.tag
            DESTINATION /usr/share/doc/buteo-syncfw-doc/)
endif()

if(NOT TARGET clean-doc)
    add_custom_target(clean-doc
        COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/html/*
        COMMAND rm -f ${CMAKE_CURRENT_SOURCE_DIR}/buteo-syncfw.tag
        COMMENT "Cleaning generated documentation"
        VERBATIM)
endif()
