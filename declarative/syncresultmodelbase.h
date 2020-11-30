/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2020-2021 Damien Caliste.
 *
 * Contact: Damien Caliste <dcaliste@free.fr>
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

#ifndef SYNCRESULTMODELBASE_H
#define SYNCRESULTMODELBASE_H

#include <QList>
#include <QSharedPointer>
#include <QAbstractListModel>

#include <ProfileManager.h>
#include <SyncResults.h>
#include <clientfw/SyncClientInterface.h>

using namespace Buteo;

class SyncResultModelBase: public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        ProfileNameRole = Qt::UserRole + 1,
        ProfileDisplayNameRole,
        ClientNameRole,
        AccountIdRole,
        SyncResultsRole,
    };

    SyncResultModelBase(QObject *parent = nullptr);
    ~SyncResultModelBase();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QHash<int, QByteArray> roleNames() const;

protected:
    void addProfileResults(QSharedPointer<SyncProfile> &profile);
    virtual void sort();

    SyncClientInterface mSyncClient;
    ProfileManager mManager;
    struct SyncResultEntry {
        QSharedPointer<SyncProfile> profile;
        SyncResults results;
    };
    QList<SyncResultEntry> mResults;
    QString mProfileName;

private slots:
    void onProfileChanged(QString aProfileName, int aChangeType,
                          QString aProfileAsXml);
};

#endif
