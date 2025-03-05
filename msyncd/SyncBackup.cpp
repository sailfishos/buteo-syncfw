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

#include "SyncBackup.h"
#include "SyncBackupAdaptor.h"
#include "LogMacros.h"
#include "UnitTest.h"

#include <QtDBus/QtDBus>

using namespace Buteo;

static const char *DBUS_BACKUP_OBJECT = "/backup";

SyncBackup::SyncBackup()
    : iBackupRestore(false)
    , iReply(0)
    , iAdaptor(0)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    iAdaptor = new SyncBackupAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();

    if (dbus.registerObject(DBUS_BACKUP_OBJECT, this)) {
        qCDebug(lcButeoMsyncd) << "Registered sync backup to D-Bus";
    } else {
        qCCritical(lcButeoMsyncd) << "Failed to register sync backup to D-Bus";
        Q_ASSERT(false);
    }
}

SyncBackup::~SyncBackup()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    iBackupRestore = false;
    //Unregister from D-Bus.
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject(DBUS_BACKUP_OBJECT);
    delete iAdaptor;
    iAdaptor = 0;
    qCDebug(lcButeoMsyncd) << "Unregistered backup from D-Bus";
}

void SyncBackup::sendDelayReply (const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (SYNCFW_UNIT_TESTS_RUNTIME)
        return;

    // coverity[unreachable]  //Suppressing false positives with code annotations
    message.setDelayedReply(true);
    if (!iReply)
        iReply = new QDBusMessage;
    *iReply = message.createReply();
}

void SyncBackup::sendReply(uchar aResult)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);

    if (SYNCFW_UNIT_TESTS_RUNTIME)
        return ;

    // coverity[unreachable]  //Suppressing false positives with code annotations
    if (iReply) {
        qCDebug(lcButeoMsyncd) << "Send Reply";
        QList<QVariant>  arguments;
        QVariant vt = QVariant::fromValue((uchar)aResult);
        arguments.append(vt);
        iReply->setArguments(arguments);
        QDBusConnection::sessionBus().send(*iReply);
        delete iReply;
        iReply = 0;
    }
}

uchar SyncBackup::backupStarts(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    iBackupRestore = true;
    sendDelayReply(message);
    emit startBackup();
    return 0;
}

uchar SyncBackup::backupFinished(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    Q_UNUSED (message);
    iBackupRestore = false;
    sendDelayReply(message);
    emit backupDone();
    return 0;
}

uchar SyncBackup::restoreStarts(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    iBackupRestore = true;
    sendDelayReply(message);
    emit startRestore();
    return 0;
}

uchar SyncBackup::restoreFinished(const QDBusMessage &message)
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    Q_UNUSED (message);
    iBackupRestore = false;
    sendDelayReply(message);
    emit restoreDone();
    return 0;
}

bool SyncBackup::getBackUpRestoreState()
{
    FUNCTION_CALL_TRACE(lcButeoTrace);
    return iBackupRestore;
}
