/*
 * This file is part of buteo-syncfw package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2013 - 2021 Jolla Ltd.
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

#include <QScopedPointer>

#include "Logger.h"

using namespace Buteo;

LogTimer::LogTimer(const QString &categoryName, const QString &func)
    : m_categoryName(categoryName.toUtf8())
    , m_func(func)
    , m_category(m_categoryName.constData())
{
    qCDebug(m_category) << m_func << ":Entry";
    m_timer.start();
}

LogTimer::~LogTimer()
{
    qCDebug(m_category) << m_func << ":Exit, execution time:" << m_timer.elapsed() << "ms";
}

Q_LOGGING_CATEGORY(lcButeoCore, "buteo.core", QtWarningMsg)
Q_LOGGING_CATEGORY(lcButeoMsyncd, "buteo.msyncd", QtWarningMsg)
Q_LOGGING_CATEGORY(lcButeoPlugin, "buteo.plugin", QtWarningMsg)
Q_LOGGING_CATEGORY(lcButeoTrace, "buteo.trace", QtWarningMsg)

