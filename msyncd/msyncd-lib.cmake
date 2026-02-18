set(TARGET msyncd-lib)

set(LIB_HEADERS
    ServerActivator.h
    synchronizer.h
    SyncDBusInterface.h
    SyncBackupProxy.h
    SyncDBusAdaptor.h
    SyncBackupAdaptor.h
    ClientThread.h
    ServerThread.h
    StorageBooker.h
    SyncQueue.h
    SyncScheduler.h
    SyncBackup.h
    AccountsHelper.h
    SyncSession.h
    PluginRunner.h
    ClientPluginRunner.h
    ServerPluginRunner.h
    SyncSigHandler.h
    StorageChangeNotifier.h
    SyncOnChange.h
    SyncOnChangeScheduler.h
    UnitTest.h
)

set(LIB_SOURCES
    ServerActivator.cpp
    synchronizer.cpp
    SyncDBusAdaptor.cpp
    SyncBackupAdaptor.cpp
    ClientThread.cpp
    ServerThread.cpp
    StorageBooker.cpp
    SyncQueue.cpp
    SyncScheduler.cpp
    SyncBackup.cpp
    AccountsHelper.cpp
    SyncSession.cpp
    PluginRunner.cpp
    ClientPluginRunner.cpp
    ServerPluginRunner.cpp
    SyncSigHandler.cpp
    StorageChangeNotifier.cpp
    SyncOnChange.cpp
    SyncOnChangeScheduler.cpp
    UnitTest.cpp
)

if(USE_KEEPALIVE)
    list(APPEND LIB_HEADERS BackgroundSync.h)
    list(APPEND LIB_SOURCES BackgroundSync.cpp)
    add_definitions(-DUSE_KEEPALIVE)
elseif(USE_IPHB)
    list(APPEND LIB_HEADERS IPHeartBeat.h SyncAlarmInventory.h)
    list(APPEND LIB_SOURCES IPHeartBeat.cpp SyncAlarmInventory.cpp)
    add_definitions(-DUSE_IPHB)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_library(${TARGET} STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${CMAKE_CURRENT_LIST_FILE})
else()
    add_library(${TARGET} STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${CMAKE_CURRENT_LIST_FILE})
endif()

target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/pluginmgr
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/profile
)

set_property(TARGET ${TARGET} PROPERTY AUTOMOC_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/pluginmgr
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/profile
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-2.0)

if(QT_VERSION_MAJOR EQUAL 6)
    pkg_check_modules(ACCOUNTS REQUIRED accounts-qt6)
    pkg_check_modules(SIGNON REQUIRED libsignon-qt6)
else()
    pkg_check_modules(ACCOUNTS REQUIRED accounts-qt5)
    pkg_check_modules(SIGNON REQUIRED libsignon-qt5)
endif()

pkg_check_modules(MCE QUIET mce-qt5)
if(MCE_FOUND)
    add_definitions(-DHAS_MCE)
else()
    message(STATUS "MCE support disabled (optional dependency)")
endif()

pkg_check_modules(BOOSTER QUIET qt5-boostable)
if(BOOSTER_FOUND)
    add_definitions(-DHAS_BOOSTER)
else()
    message(STATUS "Booster optimization not available (optional dependency)")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${TARGET} PUBLIC
        buteosyncfw5
        Qt6::Xml
        Qt6::DBus
        Qt6::Sql
        Qt6::Network
        Qt6::Core
        ${GIO_LIBRARIES}
        ${ACCOUNTS_LIBRARIES}
        ${SIGNON_LIBRARIES}
        ${MCE_LIBRARIES}
        ${BOOSTER_LIBRARIES}
    )
else()
    target_link_libraries(${TARGET} PUBLIC
        buteosyncfw5
        Qt5::Xml
        Qt5::DBus
        Qt5::Sql
        Qt5::Network
        Qt5::Core
        ${GIO_LIBRARIES}
        ${ACCOUNTS_LIBRARIES}
        ${SIGNON_LIBRARIES}
        ${MCE_LIBRARIES}
        ${BOOSTER_LIBRARIES}
    )
endif()
        
target_include_directories(${TARGET} PUBLIC
    ${GIO_INCLUDE_DIRS}
    ${ACCOUNTS_INCLUDE_DIRS}
    ${SIGNON_INCLUDE_DIRS}
    ${MCE_INCLUDE_DIRS}
    ${BOOSTER_INCLUDE_DIRS}
)

target_compile_options(${TARGET} PUBLIC
    ${GIO_CFLAGS_OTHER}
    ${ACCOUNTS_CFLAGS_OTHER}
    ${SIGNON_CFLAGS_OTHER}
    ${MCE_CFLAGS_OTHER}
    ${BOOSTER_CFLAGS_OTHER}
)

set_target_properties(${TARGET} PROPERTIES
    AUTOMOC_MOC_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR};-I${CMAKE_CURRENT_SOURCE_DIR}/..;-I${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/pluginmgr;-I${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/common;-I${CMAKE_CURRENT_SOURCE_DIR}/../libbuteosyncfw/profile")
