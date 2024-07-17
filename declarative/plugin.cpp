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

#include <QtGlobal>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "profile/SyncResults.h"
#include "profile/SyncSchedule.h"
#include "syncmanager.h"
#include "syncresultmodel.h"
#include "multisyncresultmodel.h"
#include "syncprofilewatcher.h"

class Q_DECL_EXPORT ButeoProfilesPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Buteo.Profiles")

public:
    ButeoProfilesPlugin(){}

    virtual ~ButeoProfilesPlugin() {}

    void initializeEngine(QQmlEngine *engine, const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("Buteo.Profiles"));
        Q_UNUSED(engine)
        Q_UNUSED(uri)
    }

    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("Buteo.Profiles"));

        qmlRegisterType<SyncManager>(uri, 0, 1, "SyncManager");
        qmlRegisterUncreatableType<ProfileFilter>(uri, 0, 1, "ProfileFilter",
                                                  "ProfileFilter is used as a group property of SyncManager");
        qmlRegisterType<SyncProfileWatcher>(uri, 0, 1, "SyncProfileWatcher");
        qRegisterMetaType<Buteo::SyncSchedule>("SyncSchedule");
        qmlRegisterUncreatableType<Buteo::SyncSchedule>(uri, 0, 1, "SyncSchedule",
                                                        "SyncSchedule is retrieved from a SyncProfileWatcher");

        qmlRegisterType<SyncResultModel>(uri, 0, 1, "SyncResultModel");
        qmlRegisterType<MultiSyncResultModel>(uri, 0, 1, "MultiSyncResultModel");
        qRegisterMetaType<Buteo::SyncResults>();
        qmlRegisterUncreatableType<Buteo::SyncResults>(uri, 0, 1, "SyncResults",
                                                       "SyncResults are retrieved from a SyncResultModel or a MultiSyncResultModel");
        qRegisterMetaType<Buteo::TargetResults>();
        qmlRegisterUncreatableType<Buteo::TargetResults>(uri, 0, 1, "TargetResults",
                                                         "TargetResults are retrieved from a SyncResults");
        qRegisterMetaType<Buteo::ItemCounts>();
        qmlRegisterUncreatableType<Buteo::ItemCounts>(uri, 0, 1, "ItemCounts",
                                                      "ItemCounts are retrieved from a TargetResults");
    }
};

#include "plugin.moc"
