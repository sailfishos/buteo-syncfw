project(oopp-runner)

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
elseif(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS DBus)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/pluginmgr
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/clientfw
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/profile
)

set(HEADERS
    ButeoPluginIfaceAdaptor.h
    PluginCbImpl.h
    PluginServiceObj.h
)

set(SOURCES
    ButeoPluginIfaceAdaptor.cpp
    PluginCbImpl.cpp
    PluginServiceObj.cpp
    main.cpp
)

add_executable(buteo-oopp-runner ${SOURCES} ${HEADERS})

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(buteo-oopp-runner
        Qt6::DBus
        buteosyncfw5
    )
else()
    target_link_libraries(buteo-oopp-runner
        Qt5::DBus
        buteosyncfw5
    )
endif()

install(TARGETS buteo-oopp-runner DESTINATION /usr/libexec/)