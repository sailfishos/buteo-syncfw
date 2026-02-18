if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core DBus)
elseif(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS Core DBus)
endif()

add_library(hdummy-server SHARED
    DummyServer.cpp
    DummyServer.h
)

set_target_properties(hdummy-server PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_include_directories(hdummy-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libbuteosyncfw/common
    ${CMAKE_SOURCE_DIR}/libbuteosyncfw/pluginmgr
    ${CMAKE_SOURCE_DIR}/libbuteosyncfw/profile
)

target_link_directories(hdummy-server PRIVATE
    $<TARGET_FILE_DIR:buteosyncfw5>
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(hdummy-server PRIVATE
        Qt6::Core
        Qt6::DBus
        buteosyncfw5
    )
elseif(QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(hdummy-server PRIVATE
        Qt5::Core
        Qt5::DBus
        buteosyncfw5
    )
endif()

target_compile_definitions(hdummy-server PRIVATE
    QT_PLUGIN
)

set_target_properties(hdummy-server PROPERTIES
    PREFIX ""
    SUFFIX ".so")

install(TARGETS hdummy-server
    LIBRARY DESTINATION /opt/tests/buteo-syncfw)
