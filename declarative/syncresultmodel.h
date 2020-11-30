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

#ifndef SYNCRESULTMODEL_H
#define SYNCRESULTMODEL_H

#include <syncresultmodelbase.h>

class SyncResultModel: public SyncResultModelBase
{
    Q_OBJECT
    Q_PROPERTY(QString profile READ profile WRITE setProfile NOTIFY profileChanged)

public:
    SyncResultModel(QObject *parent = nullptr);
    ~SyncResultModel();

    QString profile() const;
    void setProfile(const QString &profile);

signals:
    void profileChanged();
};

#endif
