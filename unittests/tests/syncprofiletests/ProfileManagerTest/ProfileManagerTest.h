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
#ifndef PROFILEMANAGERTEST_H
#define PROFILEMANAGERTEST_H

#include <QtTest/QtTest>

namespace Buteo {

class ProfileManagerTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testGetProfile();
    void testGetSyncProfile();
    void testGetByData();
    void testGetBySingleCriteria();
    void testGetByMultipleCriteria();
    void testGetByStorage();
    void testLog();
    void testSave();
    void testHiddenProfiles();
    void testRemovingProfiles();
    void testOverrideKey();
    void testBackup();
};

}

#endif // PROFILEMANAGERTEST_H
