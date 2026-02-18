cmake_minimum_required(VERSION 3.16)

project(buteo-tests LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/tests_common.cmake)

add_subdirectory(msyncdtests)
add_subdirectory(pluginmanagertests)
add_subdirectory(syncfwclienttests)
add_subdirectory(syncprofiletests)

install(FILES runstarget.sh
    DESTINATION ${INSTALL_TESTDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

install(FILES tests.xml
    DESTINATION ${INSTALL_TESTDIR}
)
