/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2014-2016 Jolla Ltd.
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
#if defined(USE_KEEPALIVE)
#include "BackgroundSync.h"
#elif defined(USE_IPHB)
#include "IPHeartBeat.h"
#endif
#include "SyncScheduler.h"
#include "SyncProfile.h"
#include "SyncCommonDefs.h"
#include "LogMacros.h"
#include <QtDBus/QtDBus>


using namespace Buteo;

SyncScheduler::SyncScheduler(QObject *aParent)
    :   QObject(aParent)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

#if defined(USE_KEEPALIVE)
    iBackgroundActivity = new BackgroundSync(this);

    connect(iBackgroundActivity, SIGNAL(onBackgroundSyncRunning(QString)), this, SLOT(doIPHeartbeatActions(QString)));
    connect(iBackgroundActivity, SIGNAL(onBackgroundSwitchRunning(QString)), this,
            SLOT(rescheduleBackgroundActivity(QString)));
#elif defined(USE_IPHB)
    iIPHeartBeatMan = new IPHeartBeat(this);

    connect(iIPHeartBeatMan, SIGNAL(onHeartBeat(QString)), this, SLOT(doIPHeartbeatActions(QString)));

    // Create the alarm inventory object
    iAlarmInventory = new SyncAlarmInventory();
    connect(iAlarmInventory, SIGNAL(triggerAlarm(int)),
            this, SLOT(doAlarmActions(int)));
    if (!iAlarmInventory->init()) {
        qCWarning(lcButeoMsyncd) << "AlarmInventory Init Failed";
    }

#endif
}

SyncScheduler::~SyncScheduler()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

#if defined(USE_KEEPALIVE)
    iBackgroundActivity->removeAll();
#elif defined(USE_IPHB)
    removeAllAlarms();
    delete iAlarmInventory;
    iAlarmInventory = 0;
#endif
}

void SyncScheduler::addProfileForSyncRetry(const SyncProfile *aProfile, QDateTime aNextSyncTime)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (aProfile && aProfile->isEnabled()) {
#if defined(USE_KEEPALIVE)
        setNextAlarm(aProfile, aNextSyncTime);
#elif defined(USE_IPHB)
        //remove alarm
        removeProfile(aProfile->name());

        int alarmId = setNextAlarm(aProfile, aNextSyncTime);
        if (alarmId > 0) {
            iSyncScheduleProfiles.insert(aProfile->name(), alarmId);
            qCDebug(lcButeoMsyncd) << "syncretries : retry scheduled for profile" << aProfile->name();
        }
#endif
    }
}

bool SyncScheduler::addProfile(const SyncProfile *aProfile)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (!aProfile)
        return false;

// In case Keepalive is used no need to remove
// existent profile will be updated
#if defined(USE_KEEPALIVE)
    if (aProfile->isEnabled() &&
            aProfile->syncType() == SyncProfile::SYNC_SCHEDULED) {
        setNextAlarm(aProfile);
        return true;
    } else {
        removeProfile(aProfile->name());
        return false;
    }

#elif defined(USE_IPHB)
    bool profileAdded = false;

    // Remove possible old alarm first.
    removeProfile(aProfile->name());

    if (aProfile->isEnabled() &&
            aProfile->syncType() == SyncProfile::SYNC_SCHEDULED) {
        int alarmId = setNextAlarm(aProfile);
        if (alarmId > 0) {
            iSyncScheduleProfiles.insert(aProfile->name(), alarmId);
            profileAdded = true;
            qCDebug(lcButeoMsyncd) << "Sync scheduled: profile =" << aProfile->name() <<
                      "time =" << aProfile->nextSyncTime();
        }
    }

    return profileAdded;
#else
    return false;
#endif
}

void SyncScheduler::removeProfile(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
#if defined(USE_KEEPALIVE)
    if (iBackgroundActivity->remove(aProfileName)) {
        qCDebug(lcButeoMsyncd) << "Scheduled sync removed: profile =" << aProfileName;
    }
#elif defined(USE_IPHB)
    if (iSyncScheduleProfiles.contains(aProfileName)) {
        int alarmEventID = iSyncScheduleProfiles.value(aProfileName);
        removeAlarmEvent(alarmEventID);
        iSyncScheduleProfiles.remove(aProfileName);
        qCDebug(lcButeoMsyncd) << "Scheduled sync removed: profile =" << aProfileName;
    }
#endif
}

void SyncScheduler::doIPHeartbeatActions(QString aProfileName)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    iActiveBackgroundSyncProfiles.insert(aProfileName);
    emit syncNow(aProfileName);
}

void SyncScheduler::syncStatusChanged(const QString &aProfileName, int aStatus,
                                      const QString &aMessage, int aMoreDetails)
{
    if (iActiveBackgroundSyncProfiles.contains(aProfileName) && aStatus >= Sync::SYNC_ERROR) {
        // the background sync cycle is finished.
        // tell the scheduler that it can stop preventing device suspend.
        qCDebug(lcButeoMsyncd) << "Background sync" << aProfileName << "finished with status:" << aStatus <<
                  "and extra:" << aMessage << "," << aMoreDetails;
        iActiveBackgroundSyncProfiles.remove(aProfileName);
#if defined(USE_KEEPALIVE)
        iBackgroundActivity->onBackgroundSyncCompleted(aProfileName);

        // and schedule the next background sync if necessary.
        SyncProfile *profile = iProfileManager.syncProfile(aProfileName);
        if (profile) {
            setNextAlarm(profile);
            delete profile;
        }
#endif
    }
}

#if defined(USE_KEEPALIVE)
void SyncScheduler::rescheduleBackgroundActivity(const QString &aProfileName)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    SyncProfile *profile = iProfileManager.syncProfile(aProfileName);
    if (profile) {
        if (profile->syncExternallyEnabled() || profile->syncExternallyDuringRush()) {
            emit externalSyncChanged(profile->name(), false);
        }
        setNextAlarm(profile);
        delete profile;
    } else {
        qCWarning(lcButeoMsyncd) << "Invalid profile, can't reschedule switch timer for " << aProfileName;
    }
}
#endif

int SyncScheduler::setNextAlarm(const SyncProfile *aProfile, QDateTime aNextSyncTime)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    int alarmEventID = -1;

    if (aProfile == 0) {
        return alarmEventID;
    }

    QDateTime nextSyncTime;
    if (!aNextSyncTime.isValid()) {
        nextSyncTime = aProfile->nextSyncTime(aProfile->lastSyncTime());
    } else {
        nextSyncTime = aNextSyncTime;
    }

    if (nextSyncTime.isValid()) {
        // The existing event object can be used by just updating the alarm time
        // and enqueuing it again.

#if defined(USE_KEEPALIVE)
        alarmEventID = 1;
        iBackgroundActivity->set(aProfile->name(), QDateTime::currentDateTime().secsTo(nextSyncTime) + 1);

        if (aProfile->rushEnabled()) {
            QDateTime nextSyncSwitch = aProfile->nextRushSwitchTime(QDateTime::currentDateTime());
            if (nextSyncSwitch.isValid()) {
                iBackgroundActivity->setSwitch(aProfile->name(), nextSyncSwitch);
            } else {
                iBackgroundActivity->removeSwitch(aProfile->name());
                qCDebug(lcButeoMsyncd) << "Removing switch timer for"
                          << aProfile->name() << " invalid switch timer";
            }
        } else {
            iBackgroundActivity->removeSwitch(aProfile->name());
        }
#elif defined(USE_IPHB)
        iAlarmInventory->addAlarm(nextSyncTime);
#endif
        if (alarmEventID == 0) {
            qCWarning(lcButeoMsyncd) << "Failed to add alarm for scheduled sync of profile"
                        << aProfile->name();
        }
    } else {
#if defined(USE_KEEPALIVE)
        // no valid next scheduled sync time for background sync.
        // stop the background activity to allow device suspend.
        iBackgroundActivity->remove(aProfile->name());
        if (aProfile->rushEnabled()) {
            QDateTime nextSyncSwitch = aProfile->nextRushSwitchTime(QDateTime::currentDateTime());
            if (nextSyncSwitch.isValid()) {
                iBackgroundActivity->setSwitch(aProfile->name(), nextSyncSwitch);
            } else {
                iBackgroundActivity->removeSwitch(aProfile->name());
            }
        } else {
            iBackgroundActivity->removeSwitch(aProfile->name());
        }
#endif
        qCWarning(lcButeoMsyncd) << "Next sync time is not valid, sync not scheduled for profile"
                    << aProfile->name();
    }

    return alarmEventID;
}

#if defined(USE_IPHB)
void SyncScheduler::doAlarmActions(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    const QString syncProfileName
        = iSyncScheduleProfiles.key(aAlarmEventID);

    if (!syncProfileName.isEmpty()) {
        iSyncScheduleProfiles.remove(syncProfileName);
        // Use global slots (min time == max time) for scheduling heart beats.
        if (iIPHeartBeatMan->setHeartBeat(syncProfileName, IPHB_GS_WAIT_2_5_MINS, IPHB_GS_WAIT_2_5_MINS)) {
            //Do nothing, sync will be triggered on getting heart beat
        } else {
            emit syncNow(syncProfileName);
        }
    } // no else, in error cases simply ignore

}

void SyncScheduler::removeAlarmEvent(int aAlarmEventID)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    bool err = iAlarmInventory->removeAlarm(aAlarmEventID);

    if (err < false) {
        qCWarning(lcButeoMsyncd) << "No alarm found for ID " << aAlarmEventID;
    } else {
        qCDebug(lcButeoMsyncd) << "Removed alarm, ID =" << aAlarmEventID;
    }
}

void SyncScheduler::removeAllAlarms()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    iAlarmInventory->removeAllAlarms();
}
#endif
