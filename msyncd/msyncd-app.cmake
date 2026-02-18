set(TARGET msyncd-app)

set(APP_SOURCES
    main.cpp
    UnitTest.cpp
)

set(APP_HEADERS
    UnitTest.h
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(${TARGET} ${APP_SOURCES} ${APP_HEADERS} ${CMAKE_CURRENT_LIST_FILE})
else()
    add_executable(${TARGET} ${APP_SOURCES} ${APP_HEADERS} ${CMAKE_CURRENT_LIST_FILE})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${TARGET} PRIVATE
        msyncd-lib
        Qt6::Core
        Qt6::Xml
        Qt6::DBus
        Qt6::Sql
        Qt6::Network
    )
else()
    target_link_libraries(${TARGET} PRIVATE
        msyncd-lib
        Qt5::Core
        Qt5::Xml
        Qt5::DBus
        Qt5::Sql
        Qt5::Network
    )
endif()

include(GNUInstallDirs)

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES bin/msyncd.service DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user/)
install(FILES com.meego.msyncd DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/syncwidget/)
install(FILES gschemas/com.meego.msyncd.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)