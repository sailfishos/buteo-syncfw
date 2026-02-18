cmake_minimum_required(VERSION 3.16)

project(buteoprofiles LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core DBus Qml Sql Xml Network Gui)
    
    qt_add_qml_module(buteoprofiles
        URI Buteo.Profiles
        VERSION 0.1
        NO_LINT
        SOURCES
            plugin.cpp
            syncmanager.cpp
            syncmanager.h
            profileentry.h
            syncresultmodelbase.cpp
            syncresultmodelbase.h
            syncresultmodel.cpp
            syncresultmodel.h
            multisyncresultmodel.cpp
            multisyncresultmodel.h
            syncprofilewatcher.cpp
            syncprofilewatcher.h
    )

    target_link_libraries(buteoprofiles
        PRIVATE
            Qt6::Core
            Qt6::Qml
            Qt6::DBus
            Qt6::Sql
            Qt6::Xml
            Qt6::Network
            Qt6::Gui
        PRIVATE
            buteosyncfw5
    )

    target_include_directories(buteoprofiles PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/common
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/pluginmgr
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/clientfw
    )

else()
    find_package(Qt5 REQUIRED COMPONENTS Core DBus Qml Sql Xml Network Gui)
    set(QT Qt5)

    add_library(buteoprofiles SHARED
        plugin.cpp
        syncmanager.cpp
        syncmanager.h
        profileentry.h
        syncresultmodelbase.cpp
        syncresultmodelbase.h
        syncresultmodel.cpp
        syncresultmodel.h
        multisyncresultmodel.cpp
        multisyncresultmodel.h
        syncprofilewatcher.cpp
        syncprofilewatcher.h
    )

    target_link_libraries(buteoprofiles
        PRIVATE
            Qt5::Core
            Qt5::DBus
            Qt5::Qml
            Qt5::Sql
            Qt5::Xml
            Qt5::Network
            Qt5::Gui
            buteosyncfw5
    )

    target_include_directories(buteoprofiles PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/common
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/profile
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/pluginmgr
        ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/clientfw
    )

    set_target_properties(buteoprofiles PROPERTIES
        PREFIX "")

    install(TARGETS buteoprofiles
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt5/qml/Buteo/Profiles)

    install(FILES qmldir
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt5/qml/Buteo/Profiles)

endif()

set_target_properties(buteoprofiles PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
