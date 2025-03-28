TEMPLATE = lib
TARGET = buteosyncfw5
DEPENDPATH += . clientfw common pluginmgr profile
INCLUDEPATH += . clientfw common pluginmgr profile

VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0

QT += sql xml dbus network
QT -= gui

CONFIG += dll \
    create_pc \
    create_prl

#DEFINES += BUTEO_ENABLE_DEBUG

DEFINES += DEFAULT_PLUGIN_PATH=\"\\\"$$[QT_INSTALL_LIBS]/buteo-plugins-qt5\\\"\"

# there might be something still here which shouldn't really be publicly offered
PUBLIC_HEADERS += \
           common/Logger.h \
           common/LogMacros.h \
           common/SyncCommonDefs.h \
           common/TransportTracker.h \
           common/NetworkManager.h \
           clientfw/SyncClientInterface.h \
           pluginmgr/ClientPlugin.h \
           pluginmgr/DeletedItemsIdStorage.h \
           pluginmgr/PluginCbInterface.h \
           pluginmgr/PluginManager.h \
           pluginmgr/ServerPlugin.h \
           pluginmgr/StorageChangeNotifierPlugin.h \
           pluginmgr/StorageChangeNotifierPluginLoader.h \
           pluginmgr/StorageItem.h \
           pluginmgr/StoragePlugin.h \
           pluginmgr/StoragePluginLoader.h \
           pluginmgr/SyncPluginBase.h \
           pluginmgr/SyncPluginLoader.h \
           profile/BtHelper.h \
           profile/Profile.h \
           profile/ProfileEngineDefs.h \
           profile/ProfileFactory.h \
           profile/ProfileField.h \
           profile/ProfileManager.h \
           profile/StorageProfile.h \
           profile/SyncLog.h \
           profile/SyncProfile.h \
           profile/SyncResults.h \
           profile/SyncSchedule.h \
           profile/TargetResults.h \
           pluginmgr/OOPClientPlugin.h \
           pluginmgr/OOPServerPlugin.h \
           pluginmgr/ButeoPluginIface.h

HEADERS += $$PUBLIC_HEADERS \
           clientfw/SyncClientInterfacePrivate.h \
           clientfw/SyncDaemonProxy.h \
           profile/Profile_p.h \
           profile/SyncSchedule_p.h \


SOURCES += common/Logger.cpp \
           common/TransportTracker.cpp \
           common/NetworkManager.cpp \
           clientfw/SyncClientInterface.cpp \
           clientfw/SyncClientInterfacePrivate.cpp \
           clientfw/SyncDaemonProxy.cpp \
           pluginmgr/ClientPlugin.cpp \
           pluginmgr/DeletedItemsIdStorage.cpp \
           pluginmgr/PluginManager.cpp \
           pluginmgr/ServerPlugin.cpp \
           pluginmgr/StorageItem.cpp \
           pluginmgr/StoragePlugin.cpp \
           pluginmgr/SyncPluginBase.cpp \
           pluginmgr/SyncPluginLoader.cpp \
           profile/BtHelper.cpp \
           profile/Profile.cpp \
           profile/ProfileFactory.cpp \
           profile/ProfileField.cpp \
           profile/ProfileManager.cpp \
           profile/StorageProfile.cpp \
           profile/SyncLog.cpp \
           profile/SyncProfile.cpp \
           profile/SyncResults.cpp \
           profile/SyncSchedule.cpp \
           profile/TargetResults.cpp \
           pluginmgr/OOPClientPlugin.cpp \
           pluginmgr/OOPServerPlugin.cpp \
           pluginmgr/ButeoPluginIface.cpp

usb-moded {
  message("Building with usb-moded")
  DEFINES += __USBMODED__
  HEADERS += common/USBModedProxy.h
  SOURCES += common/USBModedProxy.cpp
}

# clean
QMAKE_CLEAN += $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)

QMAKE_CLEAN += $(OBJECTS_DIR)/moc_*
QMAKE_CLEAN += lib$${TARGET}.prl pkgconfig/*

# install
target.path = $$[QT_INSTALL_LIBS]
headers.path = /usr/include/buteosyncfw5/

headers.files = $$PUBLIC_HEADERS

INSTALLS += target headers

QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR  = $$target.path
QMAKE_PKGCONFIG_INCDIR  = $$headers.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
pkgconfig.files = $${TARGET}.pc
