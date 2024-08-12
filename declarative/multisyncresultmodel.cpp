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

#include "multisyncresultmodel.h"

#include <QDebug>

using namespace Buteo;

MultiSyncResultModel::MultiSyncResultModel(QObject *parent)
    : SyncResultModelBase(parent)
    , mSortOption(MultiSyncResultModel::ByDate)
{
    const QList<SyncProfile*> profiles = mManager.allSyncProfiles();
    for (SyncProfile *profile : profiles) {
        QSharedPointer<SyncProfile> ptr(profile);
        addProfileResults(ptr);
        addProfileToFilter(*ptr);
    }
    sort();
    sortFilterList();
    connect(mSyncClient.data(), &SyncClientInterface::profileChanged,
            this, &MultiSyncResultModel::onProfileChanged);
}

MultiSyncResultModel::~MultiSyncResultModel()
{
}

void MultiSyncResultModel::addProfileToFilter(const SyncProfile &profile)
{
    if (!profile.isEnabled()) {
        return;
    }
    const Profile *client = profile.clientProfile();
    mFilterList << ProfileEntry{profile.name(),
            profile.displayname(), client ? client->name() : QString()};
}

QVariantList MultiSyncResultModel::filterList() const
{
    QVariantList list;
    for (const ProfileEntry &entry : mFilterList) {
        list << QVariant::fromValue(entry);
    }
    return list;
}

void MultiSyncResultModel::sortFilterList()
{
    std::sort(mFilterList.begin(), mFilterList.end());
}

QString MultiSyncResultModel::filter() const
{
    return mProfileName;
}

void MultiSyncResultModel::setFilter(const QString &filter)
{
    if (filter == mProfileName) {
        return;
    }

    mProfileName = filter;
    emit filterChanged();

    beginResetModel();
    mResults.clear();
    const QList<SyncProfile*> profiles = mManager.allSyncProfiles();
    for (SyncProfile *profile : profiles) {
        QSharedPointer<SyncProfile> ptr(profile);
        addProfileResults(ptr);
    }
    sort();
    endResetModel();
}

void MultiSyncResultModel::sort()
{
    switch (mSortOption) {
    case MultiSyncResultModel::ByDate:
        SyncResultModelBase::sort();
        break;
    case MultiSyncResultModel::ByAccount:
        std::sort(mResults.begin(), mResults.end(),
                  [](const SyncResultEntry &a, const SyncResultEntry &b) {
                      return a.profile->key("accountid") < b.profile->key("accountid");
                  });
        break;
    }
}

MultiSyncResultModel::SortOptions MultiSyncResultModel::sorting() const
{
    return mSortOption;
}

void MultiSyncResultModel::setSorting(MultiSyncResultModel::SortOptions option)
{
    if (option == mSortOption) {
        return;
    }
    mSortOption = option;
    emit sortingChanged();

    beginResetModel();
    sort();
    endResetModel();
}

void MultiSyncResultModel::onProfileChanged(QString aProfileName, int aChangeType , QString aProfileAsXml)
{
    Q_UNUSED(aProfileAsXml);
    Q_UNUSED(aChangeType);

    if (!mProfileName.isEmpty() && aProfileName != mProfileName) {
        return;
    }

    mFilterList.erase(std::remove_if(mFilterList.begin(), mFilterList.end(),
                                     [aProfileName] (const ProfileEntry &entry)
                                     {return entry.id == aProfileName;}),
                      mFilterList.end());
    SyncProfile* profile = mManager.syncProfile(aProfileName);
    if (profile) {
        addProfileToFilter(*profile);
        sortFilterList();
    }
    delete profile;
    emit filterListChanged();
}
