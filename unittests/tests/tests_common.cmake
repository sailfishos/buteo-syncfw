if(NOT DEFINED TESTS_COMMON_CMAKE_INCLUDED)
    set(TESTS_COMMON_CMAKE_INCLUDED 1)

    if(QT_VERSION_MAJOR EQUAL 6)
        find_package(Qt6 REQUIRED COMPONENTS Test Core DBus Xml Network Sql)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Test Core DBus Xml Network Sql)
    endif()

    find_package(PkgConfig REQUIRED)

    set(ENV{PKG_CONFIG_PATH} "/usr/lib/x86_64-linux-gnu/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    
    pkg_check_modules(DBUS REQUIRED dbus-1)
    
    if(QT_VERSION_MAJOR EQUAL 6)
        pkg_check_modules(SIGNON libsignon-qt6 REQUIRED)
        pkg_check_modules(ACCOUNTS accounts-qt6 REQUIRED)
    else()
        pkg_check_modules(SIGNON libsignon-qt5 REQUIRED)
        pkg_check_modules(ACCOUNTS accounts-qt5 REQUIRED)
    endif()

    set(BUTEOSYNCFW_LIB buteosyncfw5)
    set(TEST_COMMON_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/libbuteosyncfw/clientfw
        ${CMAKE_SOURCE_DIR}/libbuteosyncfw/common
        ${CMAKE_SOURCE_DIR}/libbuteosyncfw/pluginmgr
        ${CMAKE_SOURCE_DIR}/libbuteosyncfw/profile
        ${CMAKE_SOURCE_DIR}/msyncd
    )

    add_definitions(-DSYNCFW_UNIT_TESTS)
    set(INSTALL_TESTDIR "/opt/tests/buteo-syncfw")

endif()
