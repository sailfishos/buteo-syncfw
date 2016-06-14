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

#include "TransportTracker.h"
#if __USBMODED__
#include "USBModedProxy.h"
#endif
#include "NetworkManager.h"
#include "LogMacros.h"

// BluezQt
#include <manager.h>
#include <adapter.h>
#include <initmanagerjob.h>

#include <QMutexLocker>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusArgument>

using namespace Buteo;

TransportTracker::TransportTracker(QObject *aParent) :
    QObject(aParent),
    iUSBProxy(0),
    iInternet(0),
    iSystemBus(0),
    iBluetoothManager(0)
{
    FUNCTION_CALL_TRACE;

    iTransportStates[Sync::CONNECTIVITY_USB] = false;
    iTransportStates[Sync::CONNECTIVITY_BT] = false;
    iTransportStates[Sync::CONNECTIVITY_INTERNET] = false;

#if __USBMODED__
    // USB
    iUSBProxy = new USBModedProxy(this);
    if (!iUSBProxy->isValid())
    {
        LOG_CRITICAL("Failed to connect to USB moded D-Bus interface");
        delete iUSBProxy;
        iUSBProxy = NULL;
    }
    else
    {
        QObject::connect(iUSBProxy, SIGNAL(usbConnection(bool)), this,
            SLOT(onUsbStateChanged(bool)));
        iTransportStates[Sync::CONNECTIVITY_USB] =
            iUSBProxy->isUSBConnected();
    }
#endif

    // BT
    // Set the bluetooth state
    iTransportStates[Sync::CONNECTIVITY_BT] = false;
    iBluetoothManager = new BluezQt::Manager(this);
    connect(iBluetoothManager, &BluezQt::Manager::usableAdapterChanged,
            this, &TransportTracker::bluetoothUsableAdapterChanged);

    // Internet
    // @todo: enable when internet state is reported correctly.
    iInternet = new NetworkManager(this);
    if (iInternet != 0)
    {
        iTransportStates[Sync::CONNECTIVITY_INTERNET] =
            iInternet->isOnline();
        connect(iInternet,
                SIGNAL(statusChanged(bool, Sync::InternetConnectionType)),
                SLOT(onInternetStateChanged(bool, Sync::InternetConnectionType)) /*, Qt::QueuedConnection*/);
    }
    else
    {
        LOG_WARNING("Failed to listen for Internet state changes");
    }
}

TransportTracker::~TransportTracker()
{
    FUNCTION_CALL_TRACE;
    delete iSystemBus;
}

void TransportTracker::initialize()
{
    FUNCTION_CALL_TRACE;

    BluezQt::InitManagerJob *job = iBluetoothManager->init();
    if (!job->exec()) { // blocks synchronously
        LOG_WARNING("Unable to initialize Bluetooth manager: " << job->errorText());
    }
}

void TransportTracker::bluetoothUsableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    if (iBluetoothAdapter) {
        iBluetoothAdapter->disconnect(this);
    }
    iBluetoothAdapter = adapter;
    if (iBluetoothAdapter) {
        connect(iBluetoothAdapter.data(), &BluezQt::Adapter::poweredChanged,
                this, &TransportTracker::bluetoothPoweredChanged);
    }

    iTransportStates[Sync::CONNECTIVITY_BT] = (adapter && adapter->isPowered());
}

void TransportTracker::bluetoothPoweredChanged(bool powered)
{
    iTransportStates[Sync::CONNECTIVITY_BT] = powered;
}

bool TransportTracker::isConnectivityAvailable(Sync::ConnectivityType aType) const
{
    FUNCTION_CALL_TRACE;

    QMutexLocker locker(&iMutex);

    return iTransportStates[aType];
}

void TransportTracker::onUsbStateChanged(bool aConnected)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("USB state changed:" << aConnected);
    updateState(Sync::CONNECTIVITY_USB, aConnected);
}

void TransportTracker::onBtStateChanged(QString aKey, QDBusVariant aValue)
{
    FUNCTION_CALL_TRACE;

    if (aKey == "Powered")
    {
        bool btPowered = aValue.variant().toBool();
        LOG_DEBUG("BT power state " << btPowered);
        updateState(Sync::CONNECTIVITY_BT, btPowered);
    }
}

void TransportTracker::onInternetStateChanged(bool aConnected, Sync::InternetConnectionType aType)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Internet state changed:" << aConnected);
    updateState(Sync::CONNECTIVITY_INTERNET, aConnected);
    emit networkStateChanged(aConnected, aType);
}

void TransportTracker::updateState(Sync::ConnectivityType aType,
                                   bool aState)
{
    FUNCTION_CALL_TRACE;


    bool oldState = false;
    {
        QMutexLocker locker(&iMutex);
        oldState = iTransportStates[aType];
        iTransportStates[aType] = aState;
    }
    if(oldState != aState)
    {
        if (aType != Sync::CONNECTIVITY_INTERNET)
        {
            emit connectivityStateChanged(aType, aState);
        }
    }
}
