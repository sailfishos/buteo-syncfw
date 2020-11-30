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

#include "syncresultmodelbase.h"

#include <QDebug>

using namespace Buteo;

SyncResultModelBase::SyncResultModelBase(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&mSyncClient, &SyncClientInterface::profileChanged,
            this, &SyncResultModelBase::onProfileChanged);
}

SyncResultModelBase::~SyncResultModelBase()
{
}

void SyncResultModelBase::onProfileChanged(QString aProfileName, int aChangeType, QString aProfileAsXml)
{
    Q_UNUSED(aProfileAsXml);
    Q_UNUSED(aChangeType);

    if (!mProfileName.isEmpty() && aProfileName != mProfileName) {
        return;
    }
    QSharedPointer<SyncProfile> profile(mManager.syncProfile(aProfileName));

    beginResetModel();

    mResults.erase(std::remove_if(mResults.begin(), mResults.end(),
                                  [aProfileName] (const SyncResultEntry &entry)
                                  {return entry.profile->name() == aProfileName;}),
                   mResults.end());
    if (profile) {
        addProfileResults(profile);
        sort();
    }

    endResetModel();
}

void SyncResultModelBase::addProfileResults(QSharedPointer<SyncProfile> &profile)
{
    if (!profile || !profile->isEnabled()) {
        return;
    }
    if (!mProfileName.isEmpty() && profile->name() != mProfileName) {
        return;
    }

    const QList<const SyncResults*> allResults = profile->log()->allResults();
    for (const SyncResults *results : allResults) {
        mResults.prepend(SyncResultEntry{profile, *results});
    }
}

void SyncResultModelBase::sort()
{
    std::sort(mResults.begin(), mResults.end(),
              [](const SyncResultEntry &a, const SyncResultEntry &b) {
                  return a.results.syncTime() > b.results.syncTime();
              });
}

int SyncResultModelBase::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return mResults.size();
}

QVariant SyncResultModelBase::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mResults.count())
        return QVariant();

    switch (role) {
    case SyncResultModelBase::ProfileNameRole:
        return QVariant::fromValue(mResults[index.row()].profile->name());
    case SyncResultModelBase::ProfileDisplayNameRole:
        return QVariant::fromValue(mResults[index.row()].profile->displayname());
    case SyncResultModelBase::ClientNameRole: {
        const Profile *client = mResults[index.row()].profile->clientProfile();
        return client ? QVariant::fromValue(client->name()) : QVariant();
    }
    case SyncResultModelBase::AccountIdRole:
        return QVariant::fromValue(mResults[index.row()].profile->key("accountid"));
    case SyncResultModelBase::SyncResultsRole:
        return QVariant::fromValue(mResults[index.row()].results);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> SyncResultModelBase::roleNames() const
{
    static QHash<int, QByteArray> names;

    if (names.size() == 0) {
        names.insert(SyncResultModelBase::ProfileNameRole, "profileName");
        names.insert(SyncResultModelBase::ProfileDisplayNameRole, "profileDisplayName");
        names.insert(SyncResultModelBase::ClientNameRole, "clientName");
        names.insert(SyncResultModelBase::AccountIdRole, "accountId");
        names.insert(SyncResultModelBase::SyncResultsRole, "syncResults");
    }
    return names;
}
