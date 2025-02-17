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
#ifndef ACCOUNTSHELPER_H
#define ACCOUNTSHELPER_H

#include <QObject>

#include <Accounts/manager.h>
#include <Accounts/account.h>

namespace Buteo {

class Profile;
class AccountsHelperTest;
class ProfileManager;
class SyncProfile;

/*! \brief Helper Class towards Accounts::Manager and various SSO related
 *  operations.
 */
class AccountsHelper : public QObject
{
    Q_OBJECT

public:
    /*! \brief Constructor
     *
     * \param aProfileManager - reference to Profile Manager Object
     * \param aParent - Parent object
     */
    AccountsHelper(ProfileManager &aProfileManager, QObject *aParent = NULL);

    /*! \brief Destructor
     *
     */
    virtual ~AccountsHelper();

    /*! \brief Returns sync profiles that correspond to a given account ID
     *
     * \param id - The account ID.
     * \return A list of sync profiles. The caller should delete the profiles
     * after use.
     */
    QList<SyncProfile *> getProfilesByAccountId(Accounts::AccountId id);

public Q_SLOTS:
    /*! \brief This method is used to create profiles for a specified
     * account
     * \param id Accounts Id
     */
    void createProfileForAccount(Accounts::AccountId id);

    /*! \brief slot for Accounts::Manager accountRemoved signal
     *
     * \param id of the accounts
     */
    void slotAccountRemoved(Accounts::AccountId id);

    void slotSchedulerSettingsChanged(const char *aKey);

Q_SIGNALS:
    void enableSOC(const QString &aProfileName);
    void scheduleUpdated(const QString &aProfileName);
    void removeProfile(QString profileId);
    void removeScheduledSync(const QString &profileId);

private Q_SLOTS:
    void registerAccountListeners();

private:
    void syncEnableWithAccount(Accounts::Account *account);
    bool addProfileForAccount(Accounts::Account *account,
                              const QString &serviceName,
                              bool serviceEnabled,
                              const QString &label = QString());

    void setSyncSchedule(SyncProfile *syncProfile, Accounts::AccountId id, bool aCreateNew = false);

    void registerAccountListener(Accounts::AccountId id);

    Accounts::Manager *iAccountManager;

    ProfileManager &iProfileManager;

    QList<Accounts::Account *> iAccountList;
    QMap <Accounts::Watch *, Accounts::AccountId> iAcctWatchMap;

#ifdef SYNCFW_UNIT_TESTS
    friend class AccountsHelperTest;
#endif
};

}

#endif // ACCOUNTSHELPER_H
