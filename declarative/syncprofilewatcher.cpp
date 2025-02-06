/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2019-2021 Damien Caliste.
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

#include "syncprofilewatcher.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

using namespace Buteo;

SyncProfileWatcher::SyncProfileWatcher(QObject *parent)
    : QObject(parent)
    , mSyncClient(SyncClientInterface::sharedInstance())
    , mSyncProfile(nullptr)
    , mSyncStatus(Done)
{
    connect(&mManager, &ProfileManager::signalProfileChanged,
            this, &SyncProfileWatcher::onProfileChanged);
    connect(mSyncClient.data(), &SyncClientInterface::profileChanged,
            this, &SyncProfileWatcher::onProfileChanged);
    connect(mSyncClient.data(), &SyncClientInterface::syncStatus,
            this, &SyncProfileWatcher::onSyncStatus);
}

SyncProfileWatcher::~SyncProfileWatcher()
{
    delete mSyncProfile;
}

void SyncProfileWatcher::setName(const QString &aName)
{
    if (aName == name())
        return;

    delete mSyncProfile;
    mSyncProfile = mManager.syncProfile(aName);
    setKeys();
    Status status = Done;
    if (mSyncProfile && mSyncProfile->lastResults()) {
        switch (mSyncProfile->lastResults()->majorCode()) {
        case (SyncResults::SYNC_RESULT_INVALID): // Fallthrough
        case (SyncResults::SYNC_RESULT_SUCCESS):
            status = Done;
            break;
        case (SyncResults::SYNC_RESULT_FAILED):
            status = Error;
            break;
        case (SyncResults::SYNC_RESULT_CANCELLED):
            status = Cancelled;
            break;
        }
    }
    if (mSyncStatus != status) {
        mSyncStatus = status;
        emit syncStatusChanged();
    }
    // These properties directly depend on mSyncProfile only.
    emit nameChanged();
    emit displayNameChanged();
    emit enabledChanged();
    emit scheduleChanged();
    emit logChanged();
}

void SyncProfileWatcher::setKeys()
{
    mKeys.clear();
    if (mSyncProfile) {
        const QMap<QString, QString> keys = mSyncProfile->allKeys();
        for (QMap<QString, QString>::ConstIterator it = keys.constBegin(); it != keys.constEnd(); ++it) {
            mKeys.insert(it.key(), it.value());
        }
        Buteo::Profile *client = mSyncProfile->clientProfile();
        if (client) {
            const QMap<QString, QString> keys = client->allKeys();
            for (QMap<QString, QString>::ConstIterator it = keys.constBegin(); it != keys.constEnd(); ++it) {
                mKeys.insert(client->name() + "/" + it.key(), it.value());
            }
        }
    }
    emit keysChanged();
}

QString SyncProfileWatcher::name() const
{
    return mSyncProfile ? mSyncProfile->name() : QString();
}

QString SyncProfileWatcher::displayName() const
{
    return mSyncProfile ? mSyncProfile->displayname() : QString();
}

bool SyncProfileWatcher::enabled() const
{
    return mSyncProfile ? mSyncProfile->isEnabled() : false;
}

QVariantList SyncProfileWatcher::log()
{
    QVariantList out;
    if (mSyncProfile && mSyncProfile->log()) {
        const QList<const SyncResults*> allResults(mSyncProfile->log()->allResults());
        for (const SyncResults *result : allResults) {
            out << QVariant::fromValue(*result);
        }
    }
    return out;
}

SyncSchedule SyncProfileWatcher::schedule() const
{
    return mSyncProfile ? mSyncProfile->syncSchedule() : SyncSchedule();
}

QVariantMap SyncProfileWatcher::keys() const
{
    return mKeys;
}

SyncProfileWatcher::Status SyncProfileWatcher::syncStatus() const
{
    return mSyncStatus;
}

bool SyncProfileWatcher::synchronizing() const
{
    return mSyncStatus < Error;
}

void SyncProfileWatcher::startSync()
{
    if (mSyncProfile) {
        const QString profileId = mSyncProfile->name();
        connect(mSyncClient->requestSync(profileId, this),
                &QDBusPendingCallWatcher::finished,
                [this, profileId] (QDBusPendingCallWatcher *call) {
                    QDBusPendingReply<bool> reply = *call;
                    if (reply.isError() || !reply.value()) {
                        qWarning() << "cannot start sync for" << profileId
                                   << ":" << (reply.isError() ? reply.error().message() : "no such profile");
                        if (mSyncProfile && mSyncProfile->name() == profileId) {
                            mSyncStatus = Error;
                            emit syncStatusChanged();
                        }
                    }
                    call->deleteLater();
                });
        mSyncStatus = Queued;
        emit syncStatusChanged();
    }
}

void SyncProfileWatcher::abortSync() const
{
    if (mSyncProfile) {
        mSyncClient->abortSync(mSyncProfile->name());
    }
}

void SyncProfileWatcher::onProfileChanged(QString aProfileName, int aChangeType , QString aProfileAsXml)
{
    Q_UNUSED(aProfileAsXml);

    if (aProfileName.isEmpty() || aProfileName != name())
        return;

    // Need to reload the profile because it may have been modified by
    // an external ProfileManager and we've been signaled by DBus.
    SyncProfile *previous = mSyncProfile;
    mSyncProfile = mManager.syncProfile(aProfileName);
    if (aChangeType == ProfileManager::PROFILE_MODIFIED) {
        if (!mSyncProfile || previous->displayname() != mSyncProfile->displayname()) {
            emit displayNameChanged();
        }
        if (!mSyncProfile || previous->isEnabled() != mSyncProfile->isEnabled()) {
            emit enabledChanged();
        }
        if (!mSyncProfile || !(previous->syncSchedule() == mSyncProfile->syncSchedule())) {
            emit scheduleChanged();
        }
        setKeys();
    } else if (aChangeType == ProfileManager::PROFILE_LOGS_MODIFIED) {
        emit logChanged();
    }

    delete previous;
}

void SyncProfileWatcher::onSyncStatus(QString aProfileId, int aStatus,
                                      QString aMessage, int aStatusDetails)
{
    Q_UNUSED(aMessage);
    Q_UNUSED(aStatusDetails);

    if (aProfileId.isEmpty() || aProfileId != name())
        return;

    if (mSyncStatus != Status(aStatus)) {
        mSyncStatus = Status(aStatus);
        emit syncStatusChanged();
    }
}
