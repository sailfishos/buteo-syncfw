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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QtDebug>
#include <QDateTime>

#include "Logger.h"
#include "synchronizer.h"
#include "SyncSigHandler.h"
#include "SyncCommonDefs.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Buteo::Synchronizer *synchronizer = new Buteo::Synchronizer(&app);

    if (!synchronizer->initialize()) {
        delete synchronizer;
        synchronizer = 0;
        return -1;
    }

    QString msyncConfigSyncDir = Sync::syncConfigDir() + QDir::separator() + "sync";

    // Make sure we have the msyncd/sync directory
    QDir syncDir;
    syncDir.mkpath(msyncConfigSyncDir);

    Buteo::configureLegacyLogging();

    // Note:- Since we can't call Qt functions from Unix signal handlers.
    // This class provide handling unix signal.
    SyncSigHandler *sigHandler = new SyncSigHandler();

    qCDebug(lcButeoMsyncd) << "Entering event loop";
    int returnValue = app.exec();
    qCDebug(lcButeoMsyncd) << "Exiting event loop";

    synchronizer->close();
    delete synchronizer;
    synchronizer = 0;

    delete sigHandler;
    sigHandler = 0;

    qDebug() << "Exiting program";

    return returnValue;
}
