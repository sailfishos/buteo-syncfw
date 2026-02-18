cmake_minimum_required(VERSION 3.16)

project(buteosyncfw5 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED)
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Sql Xml DBus Network)
elseif(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS Sql Xml DBus Network)
endif()

option(USB_MODED "Enable USB-moded support" OFF)

if(QT_VERSION_MAJOR EQUAL 5)
    set(DEFAULT_PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/lib/buteo-plugins-qt5")
else()
    set(DEFAULT_PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/lib/buteo-plugins-qt${QT_VERSION_MAJOR}")
endif()

set(PUBLIC_HEADERS
    common/Logger.h
    common/LogMacros.h
    common/SyncCommonDefs.h
    common/TransportTracker.h
    common/NetworkManager.h
    clientfw/SyncClientInterface.h
    pluginmgr/ClientPlugin.h
    pluginmgr/DeletedItemsIdStorage.h
    pluginmgr/PluginCbInterface.h
    pluginmgr/PluginManager.h
    pluginmgr/ServerPlugin.h
    pluginmgr/StorageChangeNotifierPlugin.h
    pluginmgr/StorageChangeNotifierPluginLoader.h
    pluginmgr/StorageItem.h
    pluginmgr/StoragePlugin.h
    pluginmgr/StoragePluginLoader.h
    pluginmgr/SyncPluginBase.h
    pluginmgr/SyncPluginLoader.h
    profile/BtHelper.h
    profile/Profile.h
    profile/ProfileEngineDefs.h
    profile/ProfileFactory.h
    profile/ProfileField.h
    profile/ProfileManager.h
    profile/StorageProfile.h
    profile/SyncLog.h
    profile/SyncProfile.h
    profile/SyncResults.h
    profile/SyncSchedule.h
    profile/TargetResults.h
    pluginmgr/OOPClientPlugin.h
    pluginmgr/OOPServerPlugin.h
    pluginmgr/ButeoPluginIface.h
)

set(PRIVATE_HEADERS
    clientfw/SyncClientInterfacePrivate.h
    clientfw/SyncDaemonProxy.h
    profile/Profile_p.h
    profile/SyncSchedule_p.h
)

set(SOURCES
    common/Logger.cpp
    common/TransportTracker.cpp
    common/NetworkManager.cpp
    clientfw/SyncClientInterface.cpp
    clientfw/SyncClientInterfacePrivate.cpp
    clientfw/SyncDaemonProxy.cpp
    pluginmgr/ClientPlugin.cpp
    pluginmgr/DeletedItemsIdStorage.cpp
    pluginmgr/PluginManager.cpp
    pluginmgr/ServerPlugin.cpp
    pluginmgr/StorageItem.cpp
    pluginmgr/StoragePlugin.cpp
    pluginmgr/SyncPluginBase.cpp
    pluginmgr/SyncPluginLoader.cpp
    profile/BtHelper.cpp
    profile/Profile.cpp
    profile/ProfileFactory.cpp
    profile/ProfileField.cpp
    profile/ProfileManager.cpp
    profile/StorageProfile.cpp
    profile/SyncLog.cpp
    profile/SyncProfile.cpp
    profile/SyncResults.cpp
    profile/SyncSchedule.cpp
    profile/TargetResults.cpp
    pluginmgr/OOPClientPlugin.cpp
    pluginmgr/OOPServerPlugin.cpp
    pluginmgr/ButeoPluginIface.cpp
)

if(USB_MODED)
    list(APPEND PUBLIC_HEADERS common/USBModedProxy.h)
    list(APPEND SOURCES common/USBModedProxy.cpp)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_library(buteosyncfw5 SHARED
        ${PUBLIC_HEADERS}
        ${PRIVATE_HEADERS}
        ${SOURCES}
    )
else()
    add_library(buteosyncfw5 SHARED
        ${PUBLIC_HEADERS}
        ${PRIVATE_HEADERS}
        ${SOURCES}
    )
endif()

set_target_properties(buteosyncfw5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(buteosyncfw5
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/clientfw>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/common>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pluginmgr>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/profile>
        $<INSTALL_INTERFACE:include/buteosyncfw5>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(buteosyncfw5
        PUBLIC
            Qt6::Sql
            Qt6::Xml
            Qt6::DBus
            Qt6::Network
            Qt6::Core
    )
else()
    target_link_libraries(buteosyncfw5
        PUBLIC
            Qt5::Sql
            Qt5::Xml
            Qt5::DBus
            Qt5::Network
            Qt5::Core
    )
endif()

set_target_properties(buteosyncfw5 PROPERTIES
    AUTOMOC_MOC_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR};-I${CMAKE_CURRENT_SOURCE_DIR}/clientfw;-I${CMAKE_CURRENT_SOURCE_DIR}/common;-I${CMAKE_CURRENT_SOURCE_DIR}/pluginmgr;-I${CMAKE_CURRENT_SOURCE_DIR}/profile")

target_compile_definitions(buteosyncfw5
    PRIVATE
        DEFAULT_PLUGIN_PATH="${DEFAULT_PLUGIN_PATH}"
        $<$<BOOL:${USB_MODED}>:__USBMODED__>)

set_source_files_properties(profile/Profile.cpp PROPERTIES
    COMPILE_OPTIONS "-Wno-clazy-range-loop-detach"
    SKIP_LINTING ON)

include(GNUInstallDirs)

install(TARGETS buteosyncfw5
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/buteosyncfw5)
