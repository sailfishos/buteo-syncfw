include(${CMAKE_CURRENT_LIST_DIR}/../../tests_common.cmake)

get_filename_component(TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES ${TEST_NAME}.cpp)
set(HEADERS ${TEST_NAME}.h)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(${TEST_NAME} ${SOURCES} ${HEADERS})
else()
    add_executable(${TEST_NAME} ${SOURCES} ${HEADERS})
endif()

target_sources(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/msyncd/UnitTest.cpp)

target_include_directories(${TEST_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TEST_COMMON_INCLUDES}
    ${DBUS_INCLUDE_DIRS}
    ${SIGNON_INCLUDE_DIRS}
    ${ACCOUNTS_INCLUDE_DIRS}
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::DBus
        Qt6::Xml
        Qt6::Network
        Qt6::Sql
        ${BUTEOSYNCFW_LIB}
        msyncd-lib
        ${DBUS_LIBRARIES}
        ${SIGNON_LIBRARIES}
        ${ACCOUNTS_LIBRARIES}
    )
else()
    target_link_libraries(${TEST_NAME} PRIVATE
        Qt5::Test
        Qt5::Core
        Qt5::DBus
        Qt5::Xml
        Qt5::Network
        Qt5::Sql
        ${BUTEOSYNCFW_LIB}
        msyncd-lib
        ${DBUS_LIBRARIES}
        ${SIGNON_LIBRARIES}
        ${ACCOUNTS_LIBRARIES}
    )
endif()


file(RELATIVE_PATH TEST_REL_PATH ${CMAKE_SOURCE_DIR}/unittests/tests ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${TEST_NAME}
    RUNTIME DESTINATION ${INSTALL_TESTDIR}/${TEST_REL_PATH}
)

enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

