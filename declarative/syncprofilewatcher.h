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

#ifndef SYNCPROFILEWATCHER_H
#define SYNCPROFILEWATCHER_H

#include <QObject>
#include <QVariant>

#include "profile/ProfileManager.h"
#include "clientfw/SyncClientInterface.h"
#include "common/SyncCommonDefs.h"

using namespace Buteo;

class Q_DECL_EXPORT SyncProfileWatcher: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(QVariantList log READ log NOTIFY logChanged)
    Q_PROPERTY(SyncSchedule schedule READ schedule NOTIFY scheduleChanged)
    Q_PROPERTY(QVariantMap keys READ keys NOTIFY keysChanged)
    Q_PROPERTY(Sync::SyncStatus syncStatus READ syncStatus NOTIFY syncStatusChanged)

public:
    SyncProfileWatcher(QObject *parent = nullptr);
    ~SyncProfileWatcher();

    QString name() const;
    void setName(const QString &aName);

    QString displayName() const;

    bool enabled() const;

    QVariantList log();

    SyncSchedule schedule() const;

    QVariantMap keys() const;

    Sync::SyncStatus syncStatus() const;

    Q_INVOKABLE void startSync() const;
    Q_INVOKABLE void abortSync() const;

signals:
    void nameChanged();
    void displayNameChanged();
    void enabledChanged();
    void logChanged();
    void scheduleChanged();
    void keysChanged();
    void syncStatusChanged();

private slots:
    void onProfileChanged(QString aProfileName, int aChangeType,
                          QString aProfileAsXml);
    void onSyncStatus(QString aProfileId, int aStatus,
                      QString aMessage, int aStatusDetails);

private:
    void setKeys();

protected:
    ProfileManager mManager;
    QSharedPointer<SyncClientInterface> mSyncClient;
    SyncProfile *mSyncProfile;
    QVariantMap mKeys;
    Sync::SyncStatus mSyncStatus;
};

#endif
