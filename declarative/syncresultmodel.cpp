/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2020-2021 Damien Caliste.
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

#include "syncresultmodel.h"

#include <QDebug>

using namespace Buteo;

SyncResultModel::SyncResultModel(QObject *parent)
    : SyncResultModelBase(parent)
{
}

SyncResultModel::~SyncResultModel()
{
}

QString SyncResultModel::profile() const
{
    return mProfileName;
}

void SyncResultModel::setProfile(const QString &profile)
{
    if (profile == mProfileName) {
        return;
    }

    mProfileName = profile;
    emit profileChanged();

    beginResetModel();
    mResults.clear();
    if (!profile.isEmpty()) {
        QSharedPointer<SyncProfile> syncProfile(mManager.syncProfile(mProfileName));
        addProfileResults(syncProfile);
        sort();
    }
    endResetModel();
}
