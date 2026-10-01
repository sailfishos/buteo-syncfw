/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2014 Jolla Ltd.
 *
 * Contact: Valerio Valerio <valerio.valerio@jolla.com>
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

#include "LogMacros.h"
#include "BackgroundSync.h"

#include <QStringList>

#include <keepalive/backgroundactivity.h>

// 24 hours
const int MAX_FREQUENCY = 1440;

// For explicit-time schedules we arm a one-shot wakeup close to the exact
// configured time instead of using a coarse, system-aligned frequency slot.
// A small range is still used so that iphb can coalesce the wakeup with nearby
// heartbeats for power efficiency, while staying within the acceptance window
// checked by SyncSchedule::isSyncScheduled().
const int EXACT_WAKEUP_RANGE_SLACK = 60; // seconds

BackgroundSync::BackgroundSync(QObject *aParent)
    : QObject(aParent)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
}

BackgroundSync::~BackgroundSync()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    removeAll();
}

void BackgroundSync::removeAll()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QStringList profNames;

    QMapIterator<QString, BActivityStruct> iter(iScheduledSyncs);
    while (iter.hasNext()) {
        iter.next();
        profNames.append(iter.key());
    }

    for (int i = 0; i < profNames.size(); i++) {
        remove(profNames[i]);
    }

    removeAllSwitches();
}

bool BackgroundSync::remove(const QString &aProfName)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    removeSwitch(aProfName);

    if (iScheduledSyncs.contains(aProfName) == false)
        return false;

    BActivityStruct &tmp = iScheduledSyncs[aProfName];

    tmp.backgroundActivity->stop();
    delete tmp.backgroundActivity;

    iScheduledSyncs.remove(aProfName);
    return true;
}

bool BackgroundSync::set(const QString &aProfName, int seconds, bool aExactTime)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (aProfName.isEmpty())
        return false;

    // Explicit-time schedules and intervals too long to be expressed as a
    // frequency slot are armed with a one-shot wakeup range near the target
    // time instead of a coarse, system-aligned frequency slot.
    const bool useWakeupRange = aExactTime || (seconds / 60 > MAX_FREQUENCY);

    if (iScheduledSyncs.contains(aProfName) == true) {
        // A wakeup range activity can't be updated via setWakeupFrequency(),
        // so drop the existing activity and recreate it below.
        if (useWakeupRange) {
            remove(aProfName);
        } else {
            BActivityStruct &newAct = iScheduledSyncs[aProfName];
            BackgroundActivity::Frequency frequency = frequencyFromSeconds(seconds);

            if (newAct.frequency != frequency) {
                newAct.backgroundActivity->stop();
                newAct.frequency = frequency;
                newAct.backgroundActivity->setWakeupFrequency(newAct.frequency);
                newAct.backgroundActivity->wait();
                qCDebug(lcButeoMsyncd) << "BackgroundSync::set() Rescheduling for" << aProfName
                                       << "with frequency" << (seconds / 60) << "minutes, waiting.";
                return true;
            } else {
                newAct.backgroundActivity->wait();
                qCDebug(lcButeoMsyncd) << "BackgroundSync::set() Frequency unchanged for" << aProfName << ", waiting.";
                return true; //returning 'true' - no immediate sync request to be sent.
            }
        }
    }

    BActivityStruct &newAct = iScheduledSyncs[aProfName];
    newAct.backgroundActivity = new BackgroundActivity(this);
    newAct.id = newAct.backgroundActivity->id();
    connect(newAct.backgroundActivity, &BackgroundActivity::running,
            this, &BackgroundSync::onBackgroundSyncStarted);

    if (useWakeupRange) {
        newAct.frequency = BackgroundActivity::Range; // 0
        const int minDelay = qMax(0, seconds);
        newAct.backgroundActivity->setWakeupRange(minDelay, minDelay + EXACT_WAKEUP_RANGE_SLACK);
        newAct.backgroundActivity->wait();
        qCDebug(lcButeoMsyncd) << "BackgroundSync::set() profile name =" << aProfName
                               << (aExactTime ? "with explicit-time" : "without a valid frequency")
                               << "wakeup range, waiting" << minDelay << "to"
                               << (minDelay + EXACT_WAKEUP_RANGE_SLACK) << "seconds.";
    } else {
        newAct.frequency = frequencyFromSeconds(seconds);
        newAct.backgroundActivity->setWakeupFrequency(newAct.frequency);
        newAct.backgroundActivity->wait();
        qCDebug(lcButeoMsyncd) << "BackgroundSync::set() profile name =" << aProfName
                               << "with frequency " << (seconds / 60) << "minutes, waiting.";
    }
    return true;
}

void BackgroundSync::onBackgroundSyncStarted()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    BackgroundActivity *tempAct = static_cast<BackgroundActivity *>(sender());

    QString profName = getProfNameFromId(tempAct->id());

    if (!profName.isEmpty()) {
        qCDebug(lcButeoMsyncd) << "BackgroundSync started, for profile = " << profName;
        emit backgroundSyncRunning(profName);
    } else {
        qCWarning(lcButeoMsyncd) << "BackgroundSync: Error: profile for background activity not found!  Stopping background activity.";
        tempAct->stop(); // but don't delete tempAct to avoid possible crash in later profile cleanup.
    }
}

void BackgroundSync::onBackgroundSyncCompleted(QString aProfName)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    qCDebug(lcButeoMsyncd) << "BackgroundSync completed, removing activity, profile name = " << aProfName;
    remove(aProfName);
}

QString BackgroundSync::getProfNameFromId(const QString activityId) const
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QMapIterator<QString, BActivityStruct> iter(iScheduledSyncs);

    while (iter.hasNext()) {
        iter.next();
        const BActivityStruct &tmp = iter.value();
        if (tmp.id == activityId) {
            return iter.key();
            break;
        }
    }

    return QString();
}

BackgroundActivity::Frequency BackgroundSync::frequencyFromSeconds(int seconds)
{
    // Don't allow frequencies smaller than 5 mins.
    // In rare cases is possible that seconds is 0
    int minutes = seconds / 60;

    if (minutes <= 5)
        return BackgroundActivity::FiveMinutes;
    else if (minutes <= 10)
        return BackgroundActivity::TenMinutes;
    else if (minutes <= 15)
        return BackgroundActivity::FifteenMinutes;
    else if (minutes <= 30)
        return BackgroundActivity::ThirtyMinutes;
    else if (minutes <= 60)
        return BackgroundActivity::OneHour;
    else if (minutes <= 2 * 60)
        return BackgroundActivity::TwoHours;
    else if (minutes <= 4 * 60)
        return BackgroundActivity::FourHours;
    else if (minutes <= 8 * 60)
        return BackgroundActivity::EightHours;
    else if (minutes <= 10 * 60)
        return BackgroundActivity::TenHours;
    else if (minutes <= 12 * 60)
        return BackgroundActivity::TwelveHours;
    else
        return BackgroundActivity::TwentyFourHours;
}

// sync switches

void BackgroundSync::removeAllSwitches()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QStringList profNames;
    QMapIterator<QString, BActivitySwitchStruct> iter(iScheduledSwitch);
    while (iter.hasNext()) {
        iter.next();
        profNames.append(iter.key());
    }

    for (int i = 0; i < profNames.size(); i++) {
        removeSwitch(profNames[i]);
    }
}

bool BackgroundSync::removeSwitch(const QString &aProfName)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (iScheduledSwitch.contains(aProfName) == false)
        return false;

    BActivitySwitchStruct &tmp = iScheduledSwitch[aProfName];

    tmp.backgroundActivity->stop();
    delete tmp.backgroundActivity;

    iScheduledSwitch.remove(aProfName);
    return true;
}

bool BackgroundSync::setSwitch(const QString &aProfName, const QDateTime &aSwitchTime)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (aProfName.isEmpty())
        return false;

    int switchSecs = QDateTime::currentDateTime().secsTo(aSwitchTime);
    if (iScheduledSwitch.contains(aProfName) == true) {
        BActivitySwitchStruct &newSwitch = iScheduledSwitch[aProfName];
        if (newSwitch.nextSwitch != aSwitchTime) {
            // If activity's state was already Waiting, the state doesn't change, nothing happens and
            // the existing background activity keeps running until the previously set time expires, so we have to stop it.
            newSwitch.backgroundActivity->stop();
            newSwitch.nextSwitch = aSwitchTime;
            newSwitch.backgroundActivity->wait(switchSecs);
            qCDebug(lcButeoMsyncd) << "BackgroundSync::setSwitch() Rescheduling switch for" << aProfName
                                   << "at" << aSwitchTime.toString() << "("
                                   << switchSecs << "secs ) waiting.";
        } else {
            newSwitch.backgroundActivity->wait(switchSecs);
            qCDebug(lcButeoMsyncd) << "BackgroundSync::setSwitch() Profile" << aProfName
                                   << "already with the same switch timer, at"
                                   << aSwitchTime.toString() << "(" << switchSecs << "secs ) waiting.";
        }
    } else {
        BActivitySwitchStruct &newSwitch = iScheduledSwitch[aProfName];
        newSwitch.backgroundActivity = new BackgroundActivity(this);
        newSwitch.id = newSwitch.backgroundActivity->id();
        connect(newSwitch.backgroundActivity, &BackgroundActivity::running,
                this, &BackgroundSync::onBackgroundSwitchStarted);
        newSwitch.nextSwitch = aSwitchTime;
        newSwitch.backgroundActivity->wait(switchSecs);
        qCDebug(lcButeoMsyncd) << "BackgroundSync::setSwitch() Set switch for profile name =" << aProfName
                               << "at" << aSwitchTime.toString() << "(" << switchSecs << "secs ) waiting.";
    }
    return true;
}

void BackgroundSync::onBackgroundSwitchStarted()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    BackgroundActivity *tempAct = static_cast<BackgroundActivity *>(sender());

    QString profName = getProfNameFromSwitchId(tempAct->id());

    if (!profName.isEmpty()) {
        qCDebug(lcButeoMsyncd) << "BackgroundSync: switch timer started, for profile = " << profName;
        emit backgroundSwitchRunning(profName);
    } else {
        qCWarning(lcButeoMsyncd) << "BackgroundSync: Error: profile for switch timer not found!  Stopping background activity.";
        tempAct->stop(); // but don't delete tempAct to avoid possible crash in later profile cleanup.
    }
}

QString BackgroundSync::getProfNameFromSwitchId(const QString activityId) const
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QMapIterator<QString, BActivitySwitchStruct> iter(iScheduledSwitch);

    while (iter.hasNext()) {
        iter.next();
        const BActivitySwitchStruct &tmp = iter.value();
        if (tmp.id == activityId) {
            return iter.key();
            break;
        }
    }

    return QString();
}
