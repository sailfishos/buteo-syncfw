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

#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include <QSet>
#include <QQmlParserStatus>

#include "clientfw/SyncClientInterface.h"

#include "profileentry.h"

using namespace Buteo;

class ProfileFilter: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString key MEMBER key NOTIFY updated)
    Q_PROPERTY(QString value MEMBER value NOTIFY updated)
public:
    ProfileFilter(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    bool operator==(const struct ProfileFilter &other) const
    {
        return key == other.key && value == other.value;
    }
    bool isValid() const
    {
        return !key.isEmpty() && !value.isEmpty();
    }
    QString key;
    QString value;
signals:
    void updated();
};

class Q_DECL_EXPORT SyncManager: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool serviceAvailable READ serviceAvailable NOTIFY serviceAvailableChanged)
    Q_PROPERTY(bool synchronizing READ synchronizing NOTIFY synchronizingChanged)

    Q_PROPERTY(bool filterDisabled READ filterDisabled WRITE setFilterDisabled NOTIFY filterDisabledChanged)
    Q_PROPERTY(bool filterHidden READ filterHidden WRITE setFilterHidden NOTIFY filterHiddenChanged)
    Q_PROPERTY(QString filterByAccount READ filterByAccount WRITE setFilterByAccount NOTIFY filterByAccountChanged)
    Q_PROPERTY(ProfileFilter* filterBy READ filterBy CONSTANT)
    Q_PROPERTY(QVariantList profiles READ profiles NOTIFY profilesChanged)

public:
    SyncManager(QObject *parent = nullptr);
    ~SyncManager();

    void classBegin() override;
    void componentComplete() override;

    bool serviceAvailable() const;
    bool synchronizing() const;

    bool filterDisabled() const;
    bool filterHidden() const;
    QString filterByAccount() const;
    ProfileFilter* filterBy() const;
    QVariantList profiles() const;

    void setFilterDisabled(bool value);
    void setFilterHidden(bool value);
    void setFilterByAccount(const QString &accountId);

    Q_INVOKABLE void synchronize();
    Q_INVOKABLE void abort();

signals:
    void serviceAvailableChanged();
    void synchronizingChanged();
    void filterDisabledChanged();
    void filterHiddenChanged();
    void filterByAccountChanged();
    void profilesChanged();

private:
    void requestSyncProfiles();
    void onProfileChanged(QString aProfileName, int aChangeType,
                          QString aProfileAsXml);
    void requestRunningSyncList();
    void onSyncStatusChanged(QString aProfileId, int aStatus,
                             QString aMessage, int aStatusDetails);
    bool addProfile(const Profile &profile);
    void setProfilesFromXml(const QStringList &profiles);

    QSharedPointer<SyncClientInterface> mSyncClient;
    QSet<QString> mSyncingProfiles;

    bool mComponentCompleted = false;
    bool mFilterDisabled = true;
    bool mFilterHidden = false;
    QString mFilterByAccount;
    ProfileFilter *mFilterBy;
    QList<ProfileEntry> mProfiles;
};

#endif
