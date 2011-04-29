/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include "StoragePluginTest.h"
#include <StoragePlugin.h>
#include <StorageItem.h>

#include "SyncFwTestLoader.h"

#include "PluginManager.h"

using namespace Buteo;

class TestStorageItem : public StorageItem {
	public:
    bool write( qint64 aOffset, const QByteArray& aData )
    {
    	return true;
    }

    /*! \brief Read (part of) the item data
     *
     * @param aOffset The offset in bytes from where the data is read
     * @param aLength The number of bytes to read
     * @param aData Data buffer where to place data
     * @return True on success, otherwise false
     */
    bool read( qint64 aOffset, qint64 aLength, QByteArray& aData ) const
    {
    	return true;
    }

    /*! \brief Sets the length of the item data
    *
    * @param aLen Length to set for item data
    * @return True on success, otherwise false
    */
    bool resize( qint64 aLen )
    {
    	return true;
    }

    /*! \brief Get the size of the item data
     *
     * @return The data size in bytes
     */
    qint64 getSize() const
    {
    	qint64 x = 12;
    	return x;
    }
};

void StoragePluginTest::testCreateDestroy()
{
    QDir dir = QDir::current();
    QString path = dir.absolutePath();
    if (dir.cd("../dummyplugins/dummystorage/"))
    {
        path = dir.absolutePath();
    } // no else

    PluginManager pluginManager( path );

    StoragePlugin* storage1 = pluginManager.createStorage( "hdummy" );
    QVERIFY( storage1 );

    QString aProperty("test");
    storage1->getProperty(aProperty);

    QMap<QString, QString> aProperties;
    storage1->getProperties(aProperties);

    TestStorageItem si;
    QString id("id");
    si.setId(id);
    QString parentid("parentid");
    si.setParentId(parentid);
    QString type("type");
    si.setType(type);
    QString version("v1");
    si.setVersion(version);

    si.getId();
    si.getParentId();
    si.getType();
    si.getVersion();
    si.getSize();

    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    StoragePlugin* storage2 = pluginManager.createStorage( "hdummy" );

    QVERIFY( storage2 );
    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyStorage( storage1 );

    QVERIFY( pluginManager.iLoadedDlls.count() == 1 );

    pluginManager.destroyStorage(storage2 );

    pluginManager.iLoadedDlls.count();
}

void StoragePluginTest::testCreateStorageChangeNotifier()
{
    QDir dir = QDir::current();
    QString path = dir.absolutePath();
    if (dir.cd("/usr/lib/sync"))
    {
        path = dir.absolutePath();
    } // no else

    PluginManager pluginManager( path );
    QString storage("hcontacts");
    StorageChangeNotifierPlugin* x = pluginManager.createStorageChangeNotifier(storage);
    pluginManager.destroyStorageChangeNotifier(x);
}





TESTLOADER_ADD_TEST(StoragePluginTest);
