include(${CMAKE_CURRENT_LIST_DIR}/../testapplication.cmake)
target_sources(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/msyncd/UnitTest.cpp)
target_link_libraries(${TEST_NAME} PRIVATE msyncd-lib)
