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
#include "DeletedItemsIdStorage.h"
#include "LogMacros.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

using namespace Buteo;

DeletedItemsIdStorage::DeletedItemsIdStorage()
{
    FUNCTION_CALL_TRACE;
}

DeletedItemsIdStorage::~DeletedItemsIdStorage()
{
    FUNCTION_CALL_TRACE;
}

bool DeletedItemsIdStorage::init(const QString &aDbFile)
{
    FUNCTION_CALL_TRACE;

    static unsigned connectionNumber = 0;
    const QString connectionName = "deleteditems";

    if (!iDb.isOpen()) {
        iConnectionName = connectionName + QString::number(connectionNumber++);
        iDb = QSqlDatabase::addDatabase("QSQLITE", iConnectionName);
        iDb.setDatabaseName(aDbFile);
        iDb.open();
    }


    if (!iDb.isOpen()) {
        qCCritical(lcButeoCore) << "Could open deleted items database file:" << aDbFile ;
        return false;
    }

    if (!ensureItemSnapshotExists() || !ensureDeletedItemsExists()) {
        return false;
    }

    return true;
}

bool DeletedItemsIdStorage::uninit()
{
    FUNCTION_CALL_TRACE;

    if (iDb.isOpen()) {
        iDb.close();
        iDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(iConnectionName);
    }

    return true;
}

bool DeletedItemsIdStorage::getSnapshot(QList<QString> &aItems, QList<QDateTime> &aCreationTimes) const
{
    FUNCTION_CALL_TRACE;

    const QString queryString("SELECT * FROM snapshot");
    QSqlQuery query(iDb);
    query.prepare(queryString);

    if (!query.exec()) {
        qCWarning(lcButeoCore) << "Could not retrieve item snapshot: " << query.lastError() ;
        return false;
    }

    while (query.next()) {
        aItems.append(query.value(0).toString());
        QDateTime t = query.value(1).toDateTime();
        t.setTimeSpec(Qt::UTC);
        aCreationTimes.append(t.toLocalTime());
    }

    return true;
}

bool DeletedItemsIdStorage::setSnapshot(const QList<QString> &aItems,
                                        const QList<QDateTime> &aCreationTimes)
{
    FUNCTION_CALL_TRACE;

    // Clear existing snapshot
    const QString deleteQueryString("DELETE FROM snapshot");
    QSqlQuery deleteQuery(iDb);
    deleteQuery.prepare(deleteQueryString);

    if (!deleteQuery.exec()) {
        qCWarning(lcButeoCore) << "Could not clear item snapshot: " << deleteQuery.lastError() ;
        return false;
    }

    if (!aItems.isEmpty()) {
        const QString insertQueryString("INSERT INTO snapshot VALUES (:itemid, :creationtime)");
        bool supportsTransaction = iDb.transaction();
        if (!supportsTransaction) {
            qCDebug(lcButeoCore) << "SQL Db doesn't support transactions";
        }

        QSqlQuery insertQuery(iDb);
        insertQuery.prepare(insertQueryString);

        QVariantList itemIds;
        QVariantList creationTimes;

        for (int i = 0; i < aItems.count(); ++i) {
            itemIds << aItems[i];
            creationTimes << aCreationTimes[i].toUTC();
        }

        insertQuery.addBindValue(itemIds);
        insertQuery.addBindValue(creationTimes);

        // Insert new snapshot
        if (insertQuery.execBatch()) {
            qCDebug(lcButeoCore) << itemIds.count() << "items set to snapshot" ;
        } else {
            qCWarning(lcButeoCore) << "Could not set items snapshot" ;
            qCWarning(lcButeoCore) << "Reason:" << insertQuery.lastError() ;
        }

        if (supportsTransaction) {
            if (!iDb.commit()) {
                qCWarning(lcButeoCore) << "Error while commiting : " << iDb.lastError();
            }
        }
    }

    return true;
}

void DeletedItemsIdStorage::addDeletedItem(const QString &aItem, const QDateTime &aCreationTime,
                                           const QDateTime &aDeleteTime)
{
    FUNCTION_CALL_TRACE;

    const QString queryString("INSERT INTO deleteditems VALUES(:itemid, :creationtime, :deletetime)");

    QSqlQuery query(iDb);

    query.prepare(queryString);
    query.bindValue(":itemid", aItem);
    query.bindValue(":creationtime", aCreationTime.toUTC());
    query.bindValue(":deletetime", aDeleteTime.toUTC());

    if (query.exec()) {
        qCDebug(lcButeoCore) << "Added item" << aItem << "as deleted at time" << aDeleteTime << ", creation time:" << aCreationTime ;
    } else {
        qCWarning(lcButeoCore) << "Could not add item as deleted:" << aItem ;
        qCWarning(lcButeoCore) << "Reason:" << query.lastError() ;
    }

}

void DeletedItemsIdStorage::addDeletedItems(const QList<QString> &aItems, const QList<QDateTime> &aCreationTimes,
                                            const QList<QDateTime> &aDeleteTimes)
{
    FUNCTION_CALL_TRACE;

    const QString queryString("INSERT INTO deleteditems VALUES(:itemid, :creationtime, :deletetime)");

    QSqlQuery query(iDb);

    bool supportsTransaction = iDb.transaction();
    if (!supportsTransaction) {
        qCDebug(lcButeoCore) << "SQL Db doesn't support transactions";
    }

    query.prepare(queryString);

    QVariantList items;
    QVariantList creationTimes;
    QVariantList deleteTimes;

    for (int i = 0; i < aItems.count(); ++i) {
        items << aItems[i];
        creationTimes << aCreationTimes[i].toUTC();
        deleteTimes << aDeleteTimes[i].toUTC();
    }

    query.addBindValue(items);
    query.addBindValue(creationTimes);
    query.addBindValue(deleteTimes);

    if (query.execBatch()) {
        qCDebug(lcButeoCore) << "Added" << items.count()  << "items as deleted" ;
    } else {
        qCWarning(lcButeoCore) << "Could not add items as deleted" ;
        qCWarning(lcButeoCore) << "Reason:" << query.lastError() ;
    }

    if (supportsTransaction) {
        if (!iDb.commit()) {
            qCWarning(lcButeoCore) << "Error while commiting : " << iDb.lastError();
        }
    }
}

bool DeletedItemsIdStorage::getDeletedItems(QList<QString> &aItems, const QDateTime &aTime)
{
    FUNCTION_CALL_TRACE;

    const QString queryString("SELECT itemid FROM deleteditems WHERE creationtime < :creationtime AND deletetime > :deletetime");
    qCDebug(lcButeoCore) << queryString;
    QSqlQuery query(iDb);

    query.prepare(queryString );
    query.bindValue(":creationtime", aTime.toUTC());
    query.bindValue(":deletetime", aTime.toUTC());

    if (!query.exec()) {
        qCWarning(lcButeoCore) << "Could not retrieve deleted items:" << query.lastError();
        return false;
    }

    while (query.next()) {
        aItems.append( query.value(0).toString() );
    }

    qCDebug(lcButeoCore) << "Found" << aItems.count() << "deleted items" ;

    return true;
}

bool DeletedItemsIdStorage::ensureItemSnapshotExists()
{
    FUNCTION_CALL_TRACE;

    const QString queryString("CREATE TABLE IF NOT EXISTS snapshot(itemid varchar(512) primary key, creationtime timestamp)");
    QSqlQuery query(iDb);
    query.prepare(queryString);

    if (!query.exec()) {
        qCWarning(lcButeoCore) << "Query failed: " << query.lastError();
        return false;
    } else {
        qCDebug(lcButeoCore) << "Ensured database table: snapshot" ;
        return true;
    }
}

bool DeletedItemsIdStorage::ensureDeletedItemsExists()
{
    FUNCTION_CALL_TRACE;

    const QString queryString("CREATE TABLE IF NOT EXISTS deleteditems(itemid varchar(512) primary key, creationtime timestamp, deletetime timestamp)");
    QSqlQuery query(iDb);
    query.prepare(queryString);

    if (!query.exec()) {
        qCWarning(lcButeoCore) << "Query failed: " << query.lastError();
        return false;
    } else {
        qCDebug(lcButeoCore) << "Ensured database table: deleteditems" ;
        return true;
    }
}
