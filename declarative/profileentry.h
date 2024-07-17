/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2024 Damien Caliste.
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

#ifndef PROFILEENTRY_H
#define PROFILEENTRY_H

#include <QObject>

struct ProfileEntry {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString label MEMBER label)
    Q_PROPERTY(QString clientName MEMBER clientName)
public:
    QString id;
    QString label;
    QString clientName;
    bool operator<(const struct ProfileEntry &other)
    {
        return label < other.label;
    }
};

#endif
