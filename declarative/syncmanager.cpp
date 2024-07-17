/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2024 Damien Caliste.
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

#include "syncmanager.h"

#include <ProfileManager.h>
#include <SyncCommonDefs.h>
#include <ProfileEngineDefs.h>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

using namespace Buteo;

SyncManager::SyncManager(QObject *parent)
    : QObject(parent)
    , mSyncClient(SyncClientInterface::sharedInstance())
    , mFilterBy(new ProfileFilter(this))
{
    connect(mSyncClient.data(), &SyncClientInterface::isValidChanged,
            this, &SyncManager::serviceAvailableChanged);
    connect(mSyncClient.data(), &SyncClientInterface::syncStatus,
            this, &SyncManager::onSyncStatusChanged);
    connect(mSyncClient.data(), &SyncClientInterface::profileChanged,
            this, &SyncManager::onProfileChanged);

    connect(mFilterBy, &ProfileFilter::updated,
            this, &SyncManager::requestSyncProfiles);

    requestRunningSyncList();
}

SyncManager::~SyncManager()
{
}

void SyncManager::classBegin()
{
}

void SyncManager::componentComplete()
{
    mComponentCompleted = true;
    requestSyncProfiles();
}

bool SyncManager::serviceAvailable() const
{
    return mSyncClient->isValid();
}

bool SyncManager::synchronizing() const
{
    for (const ProfileEntry &entry : mProfiles) {
        if (mSyncingProfiles.contains(entry.id))
            return true;
    }
    return false;
}

void SyncManager::onSyncStatusChanged(QString aProfileId, int aStatus,
                                      QString aMessage, int aStatusDetails)
{
    Q_UNUSED(aMessage);
    Q_UNUSED(aStatusDetails);

    bool syncInProgress = synchronizing();
    if (static_cast<Sync::SyncStatus>(aStatus) < Sync::SYNC_ERROR)
        mSyncingProfiles.insert(aProfileId);
    else
        mSyncingProfiles.remove(aProfileId);

    if (syncInProgress != synchronizing())
        emit synchronizingChanged();
}

void SyncManager::requestRunningSyncList()
{
    connect(mSyncClient->requestRunningSyncList(this),
            &QDBusPendingCallWatcher::finished,
            [this] (QDBusPendingCallWatcher *call) {
                QDBusPendingReply<QStringList> reply = *call;
                if (reply.isError()) {
                    qWarning() << "cannot get running sync list:" << reply.error().message();
                } else {
                    bool syncInProgress = synchronizing();
                    mSyncingProfiles.clear();
                    for (const QString profileId : reply.value()) {
                        mSyncingProfiles.insert(profileId);
                    }
                    if (syncInProgress != synchronizing()) {
                        emit synchronizingChanged();
                    }
                }
                call->deleteLater();
            });
}

void SyncManager::onProfileChanged(QString aProfileName, int aChangeType,
                                   QString aProfileAsXml)
{
    Q_UNUSED(aProfileAsXml);
    ProfileManager::ProfileChangeType change = static_cast<ProfileManager::ProfileChangeType>(aChangeType);

    bool changed = false;
    if (change == ProfileManager::PROFILE_MODIFIED
        || change == ProfileManager::PROFILE_REMOVED) {
        mProfiles.erase(std::remove_if(mProfiles.begin(), mProfiles.end(),
                                       [aProfileName] (const ProfileEntry &entry)
                                       {return entry.id == aProfileName;}),
                        mProfiles.end());
        changed = true;
    }
    if (change == ProfileManager::PROFILE_MODIFIED
        || change == ProfileManager::PROFILE_ADDED) {
        Profile *profile = ProfileManager::profileFromXml(aProfileAsXml);
        if (profile && addProfile(*profile)) {
            std::sort(mProfiles.begin(), mProfiles.end());
            changed = true;
        }
        delete profile;
    }
    if (changed) {
        emit profilesChanged();
        emit synchronizingChanged();
    }
}

void SyncManager::requestSyncProfiles()
{
    if (!mComponentCompleted)
        return;

    QDBusPendingCallWatcher *request;
    if (mFilterBy->isValid()) {
        request = mSyncClient->requestSyncProfilesByKey(mFilterBy->key,
                                                        mFilterBy->value,
                                                        this);
    } else if (!mFilterByAccount.isEmpty()) {
        request = mSyncClient->requestSyncProfilesByKey(KEY_ACCOUNT_ID,
                                                        mFilterByAccount,
                                                        this);
    } else if (mFilterHidden) {
        request = mSyncClient->requestAllVisibleSyncProfiles(this);
    } else {
        request = mSyncClient->requestProfilesByType(Profile::TYPE_SYNC, this);
    }
    connect(request, &QDBusPendingCallWatcher::finished,
            [this] (QDBusPendingCallWatcher *call) {
                QDBusPendingReply<QStringList> reply = *call;
                if (reply.isError()) {
                    qWarning() << "cannot list profiles:" << reply.error().message();
                } else {
                    setProfilesFromXml(reply.value());
                }
                call->deleteLater();
            });
}

QVariantList SyncManager::profiles() const
{
    QVariantList list;
    for (const ProfileEntry &entry : mProfiles) {
        list << QVariant::fromValue(entry);
    }
    return list;
}

void SyncManager::setProfilesFromXml(const QStringList &profiles)
{
    bool changed = !mProfiles.isEmpty();
    mProfiles.clear();
    for (const QString profileAsXml : profiles) {
        Profile *profile = ProfileManager::profileFromXml(profileAsXml);
        if (profile && addProfile(*profile)) {
            changed = true;
        }
        delete profile;
    }
    if (changed) {
        std::sort(mProfiles.begin(), mProfiles.end());
        emit profilesChanged();
        emit synchronizingChanged();
    }
}

bool SyncManager::addProfile(const Profile &profile)
{
    if (profile.type() != Profile::TYPE_SYNC) {
        return false;
    }
    if (mFilterDisabled && !profile.isEnabled()) {
        return false;
    }
    if (mFilterHidden && profile.isHidden()) {
        return false;
    }
    if (!mFilterByAccount.isEmpty() && profile.key(KEY_ACCOUNT_ID) != mFilterByAccount) {
        return false;
    }
    const Profile *client = static_cast<const SyncProfile*>(&profile)->clientProfile();
    mProfiles << ProfileEntry{profile.name(),
            profile.displayname(), client ? client->name() : QString()};
    return true;
}

bool SyncManager::filterDisabled() const
{
    return mFilterDisabled;
}

bool SyncManager::filterHidden() const
{
    return mFilterHidden;
}

QString SyncManager::filterByAccount() const
{
    return mFilterByAccount;
}

ProfileFilter* SyncManager::filterBy() const
{
    return mFilterBy;
}

void SyncManager::setFilterDisabled(bool value)
{
    if (value == mFilterDisabled)
        return;

    mFilterDisabled = value;
    emit filterDisabledChanged();

    requestSyncProfiles();
}

void SyncManager::setFilterHidden(bool value)
{
    if (value == mFilterHidden)
        return;

    mFilterHidden = value;
    emit filterHiddenChanged();

    requestSyncProfiles();
}

void SyncManager::setFilterByAccount(const QString &accountId)
{
    if (accountId == mFilterByAccount)
        return;

    mFilterByAccount = accountId;
    emit filterByAccountChanged();

    requestSyncProfiles();
}

void SyncManager::synchronize()
{
    for (const ProfileEntry &entry : mProfiles) {
        connect(mSyncClient->requestSync(entry.id, this),
                &QDBusPendingCallWatcher::finished,
                [this, entry] (QDBusPendingCallWatcher *call) {
                    QDBusPendingReply<bool> reply = *call;
                    if (reply.isError() || !reply.value()) {
                        qWarning() << "cannot start sync for" << entry.id
                                   << ":" << (reply.isError() ? reply.error().message() : "no such profile");
                        mSyncingProfiles.remove(entry.id);
                        emit synchronizingChanged();
                    }
                    call->deleteLater();
                });
        mSyncingProfiles.insert(entry.id);
    }
    if (!mProfiles.isEmpty())
        emit synchronizingChanged();
}

void SyncManager::abort()
{
    for (const ProfileEntry &entry : mProfiles) {
        mSyncClient->abortSync(entry.id);
        mSyncingProfiles.remove(entry.id);
    }
    if (!mProfiles.isEmpty())
        emit synchronizingChanged();
}
