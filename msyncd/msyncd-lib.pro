TEMPLATE = lib
CONFIG += staticlib
TARGET = msyncd
QT += xml \
    dbus \
    sql \
    network
QT -= gui

CONFIG += \
    link_pkgconfig \
    create_prl

DEPENDPATH += .
INCLUDEPATH += . \
    ../ \
    ../libbuteosyncfw/pluginmgr \
    ../libbuteosyncfw/common \
    ../libbuteosyncfw/profile


PKGCONFIG += dbus-1 gio-2.0 libsignon-qt5 accounts-qt5
packagesExist(mce-qt5) {
    PKGCONFIG += mce-qt5
    DEFINES += HAS_MCE
} else {
    message("mce-qt5 not found, MCE support disabled")
}
LIBS += -lbuteosyncfw5
packagesExist(qt5-boostable) {
    DEFINES += HAS_BOOSTER
    PKGCONFIG += qt5-boostable
} else {
    warning("qt5-boostable not available; startup times will be slower")
}

QMAKE_LIBDIR_QT += ../libsyncprofile/

LIBS += -L../libbuteosyncfw

# Input
HEADERS += ServerActivator.h \
    synchronizer.h \
    SyncDBusInterface.h \
    SyncBackupProxy.h \
    SyncDBusAdaptor.h \
    SyncBackupAdaptor.h \
    ClientThread.h \
    ServerThread.h \
    StorageBooker.h \
    SyncQueue.h \
    SyncScheduler.h \
    SyncBackup.h \
    AccountsHelper.h \
    SyncSession.h \
    PluginRunner.h \
    ClientPluginRunner.h \
    ServerPluginRunner.h \
    SyncSigHandler.h \
    StorageChangeNotifier.h \
    SyncOnChange.h \
    SyncOnChangeScheduler.h

SOURCES += ServerActivator.cpp \
    synchronizer.cpp \
    SyncDBusAdaptor.cpp \
    SyncBackupAdaptor.cpp \
    ClientThread.cpp \
    ServerThread.cpp \
    StorageBooker.cpp \
    SyncQueue.cpp \
    SyncScheduler.cpp \
    SyncBackup.cpp \
    AccountsHelper.cpp \
    SyncSession.cpp \
    PluginRunner.cpp \
    ClientPluginRunner.cpp \
    ServerPluginRunner.cpp \
    SyncSigHandler.cpp \
    StorageChangeNotifier.cpp \
    SyncOnChange.cpp \
    SyncOnChangeScheduler.cpp

contains(DEFINES, USE_KEEPALIVE) {
    PKGCONFIG += keepalive

    HEADERS += \
        BackgroundSync.h

    SOURCES += \
        BackgroundSync.cpp

} else:contains(DEFINES, USE_IPHB) {
    PKGCONFIG += libiphb

    HEADERS += \
        IPHeartBeat.h \
        SyncAlarmInventory.h

    SOURCES += \
        IPHeartBeat.cpp \
        SyncAlarmInventory.cpp
}

# #####################################################################
# make coverage (debug)
# #####################################################################
coverage.CONFIG += recursive
QMAKE_EXTRA_TARGETS += coverage
CONFIG(debug,debug|release) {
    QMAKE_EXTRA_TARGETS += cov_cxxflags \
        cov_lflags
    cov_cxxflags.target = coverage
    cov_cxxflags.depends = CXXFLAGS \
        += \
        -fprofile-arcs \
        -ftest-coverage
    cov_lflags.target = coverage
    cov_lflags.depends = LFLAGS \
        += \
        -fprofile-arcs \
        -ftest-coverage

    # QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    # QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
    # -ftest-coverage
    coverage.commands = @echo \
        "Built with coverage support..."
    build_pass|!debug_and_release:coverage.depends = all
    QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda \
        $(OBJECTS_DIR)/*.gcno \
        $(OBJECTS_DIR)/*.gcov
}
