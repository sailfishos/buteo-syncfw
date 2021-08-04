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

#include "SyncAlarmInventory.h"
#include "SyncCommonDefs.h"

#include <QTimer>
#include <QObject>
#include <QSettings>
#include <QDebug>
#include <LogMacros.h>

const QString ALARM_CONNECTION_NAME("alarms");

const int TRIGGER_COUNT = 1;

SyncAlarmInventory::SyncAlarmInventory():
    iTimer(0),
    currentAlarm(0),
    triggerCount(TRIGGER_COUNT)
{
    // empty.explicitly call init
}

bool SyncAlarmInventory::init()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    static unsigned connectionNumber = 0;
    iConnectionName = ALARM_CONNECTION_NAME + QString::number(connectionNumber++);
    iDbHandle = QSqlDatabase::addDatabase("QSQLITE", iConnectionName);

    QString path(Sync::syncCacheDir());
    path.append(QDir::separator()).append("alarms.db.sqlite");
    path = QDir::toNativeSeparators(path);

    iDbHandle.setDatabaseName(path);

    if (!iDbHandle.open()) {
        qCCritical(lcButeoMsyncd) << "Failed to OPEN DB. SCHEDULING WILL NOT WORK";
        return false;
    } else {
        qCDebug(lcButeoMsyncd) << "DB Opened Successfully";
    }

    // Create the alarms table
    const QString createTableQuery("CREATE TABLE IF NOT EXISTS alarms(alarmid INTEGER PRIMARY KEY AUTOINCREMENT, synctime DATETIME)");
    QSqlQuery query(createTableQuery, iDbHandle);
    qCDebug(lcButeoMsyncd) << "SQL Query::" << query.lastQuery();
    if (!query.exec()) {
        qCWarning(lcButeoMsyncd) << "Failed to execute the createTableQuery";
        return false;
    }

    // Clear any old alarms that may have lingered
    removeAllAlarms();

    // Create the iTimer object
    iTimer = new QTimer(this);
    connect(iTimer, SIGNAL(timeout()), this, SLOT(timerTriggered()));
    currentAlarm = 0;
    return true;
}

SyncAlarmInventory::~SyncAlarmInventory()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    iDbHandle.close();
    iDbHandle = QSqlDatabase();
    QSqlDatabase::removeDatabase(iConnectionName);

    delete iTimer;
    iTimer = 0;
}

int SyncAlarmInventory::addAlarm(QDateTime alarmDate)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    // Check if alarmDate < QDateTime::currentDateTime()
    if (QDateTime::currentDateTime().secsTo(alarmDate) < 0) {
        qCWarning(lcButeoMsyncd) << "alarmDate < QDateTime::currentDateTime()";
        //Setting with current date time.
        alarmDate = QDateTime::currentDateTime();
    }

    // Store the alarm
    int alarmId = 0;
    if ((alarmId = addAlarmToDb(alarmDate)) == 0) {
        // Note: Even incase of an already existing profile, false is returned by the query
        // There is no way to detect a record insertion from an already existing alarm

        // If unable to add an alarm to db, set the iTimers
        // for already existing alarms
        qCWarning(lcButeoMsyncd) << "(alarmId = addAlarmToDb(alarmDate)) == 0";
    }

    // Select all the alarms from the db sorted by alarm time
    QSqlQuery selectQuery(iDbHandle);
    if (selectQuery.exec("SELECT alarmid,synctime FROM alarms ORDER BY synctime ASC")) {
        qCDebug(lcButeoMsyncd) << "SQL Query::" << selectQuery.lastQuery();
        if (selectQuery.first()) {
            int newAlarm = selectQuery.value(0).toInt();
            QDateTime alarmTime = selectQuery.value(1).toDateTime();

            // If the newAlarm != currentAlarm that is fetched from DB, stop the
            // previous iTimer
            if ((currentAlarm != 0) && (newAlarm != currentAlarm)) {
                if (!iTimer->isActive())
                    iTimer->stop();
            }

            // This is a new alarm. Set the iTimer for the alarm
            currentAlarm = newAlarm;
            QDateTime now = QDateTime::currentDateTime();
            int iTimerInterval = 0;
            if (now < alarmTime) {
                iTimerInterval = (now.secsTo(alarmTime) / TRIGGER_COUNT) * 1000;  // time interval in millisec
            }
            qCDebug(lcButeoMsyncd) << "currentAlarm" << currentAlarm << "alarmTime" << alarmTime << "iTimerInterval" << iTimerInterval;
            triggerCount = TRIGGER_COUNT;
            iTimer->setInterval(iTimerInterval);
            iTimer->start();
        }
    } else {
        qCWarning(lcButeoMsyncd) << "Select Query Execution Failed";
    }

    return alarmId;
}

bool SyncAlarmInventory::removeAlarm(int alarmId)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (alarmId <= 0)
        return false;
    deleteAlarmFromDb(alarmId);
    return true;
}

void SyncAlarmInventory::removeAllAlarms()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QSqlQuery deleteAllQuery(QString("DELETE FROM alarms"), iDbHandle);
    qCDebug(lcButeoMsyncd) << "SQL Query::" << deleteAllQuery.lastQuery();
    if (!deleteAllQuery.exec()) {
        qCWarning(lcButeoMsyncd) << "Failed query to delete all alarms";
    }
}

void SyncAlarmInventory::timerTriggered()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    // Decrement the alarm counter
    triggerCount--;

    // Alarm expired. Trigger the alarm and delete it from DB and set the alarm for the next one
    if (triggerCount == 0) {
        qCDebug(lcButeoMsyncd) << "Triggering the alarm " << currentAlarm;
        emit triggerAlarm(currentAlarm);

        // Delete the alarm from DB
        if (!deleteAlarmFromDb(currentAlarm)) {
        }
        iTimer->stop();
        currentAlarm = 0;

        // Set the new alarm iTimer
        // Select all the alarms from the db sorted by alarm time
        QSqlQuery selectQuery(iDbHandle);
        if (selectQuery.exec("SELECT alarmid,synctime FROM alarms ORDER BY synctime ASC")) {
            qCDebug(lcButeoMsyncd) << "SQL Query::" << selectQuery.lastQuery();
            if (selectQuery.first()) {
                currentAlarm = selectQuery.value(0).toInt();
                QDateTime alarmTime = selectQuery.value(1).toDateTime();

                // Set the iTimer for the alarm
                QDateTime now = QDateTime::currentDateTime();
                int iTimerInterval = 0;
                if (now < alarmTime) {
                    iTimerInterval = (now.secsTo(alarmTime) / TRIGGER_COUNT) * 1000;  // time interval in millisec
                }
                triggerCount = TRIGGER_COUNT;
                qCDebug(lcButeoMsyncd) << "Starting timer with interval::" << iTimerInterval;
                iTimer->setInterval(iTimerInterval);
                iTimer->start();
            }
        }
    }
}

bool SyncAlarmInventory::deleteAlarmFromDb(int alarmId)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QSqlQuery removeQuery (iDbHandle);
    removeQuery.prepare("DELETE FROM alarms WHERE alarmid=:alarmid");
    removeQuery.bindValue(":alarmid", alarmId);

    qCDebug(lcButeoMsyncd) << "SQL Query::" << removeQuery.lastQuery();
    if (!removeQuery.exec())
        return false;
    else
        return true;
}

int SyncAlarmInventory::addAlarmToDb(QDateTime timeStamp)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    QSqlQuery insertQuery(iDbHandle);
    insertQuery.prepare("INSERT INTO alarms(synctime) VALUES(:synctime)");
    insertQuery.bindValue(":synctime", timeStamp);

    qCDebug(lcButeoMsyncd) << "SQL Query::" << insertQuery.lastQuery();
    if (insertQuery.exec())
        return insertQuery.lastInsertId().toInt();
    else
        return 0;
}

