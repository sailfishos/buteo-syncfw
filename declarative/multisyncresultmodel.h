/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2021 Damien Caliste.
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

#ifndef MULTISYNCRESULTMODEL_H
#define MULTISYNCRESULTMODEL_H

#include "syncresultmodelbase.h"
#include "profileentry.h"

class MultiSyncResultModel: public SyncResultModelBase
{
    Q_OBJECT
    Q_PROPERTY(QVariantList filterList READ filterList NOTIFY filterListChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(SortOptions sorting READ sorting WRITE setSorting NOTIFY sortingChanged)
public:
    enum SortOptions
    {
        ByDate,
        ByAccount
    };
    Q_ENUM(SortOptions)

    MultiSyncResultModel(QObject *parent = nullptr);
    ~MultiSyncResultModel();

    QVariantList filterList() const;
    QString filter() const;
    void setFilter(const QString &filter);

    SortOptions sorting() const;
    void setSorting(SortOptions option);

signals:
    void filterListChanged();
    void filterChanged();
    void sortingChanged();

private slots:
    void onProfileChanged(QString aProfileName, int aChangeType,
                          QString aProfileAsXml);

private:
    void addProfileToFilter(const SyncProfile &profile);
    void sortFilterList();
    virtual void sort();

    SortOptions mSortOption;
    QList<ProfileEntry> mFilterList;
};

#endif
