/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "daemon.h"
#include "networkpackage.h"
#include "notificationpackagereceiver.h"
#include "pausemusicpackagereceiver.h"
#include "avahiannouncer.h"
#include "fakeannouncer.h"

#include <QtNetwork/QUdpSocket>
#include <QFile>

#include <KDE/KIcon>

#include <sstream>
#include <iomanip>
#include <iostream>

K_PLUGIN_FACTORY(AndroidShineFactory, registerPlugin<Daemon>();)
K_EXPORT_PLUGIN(AndroidShineFactory("androidshine", "androidshine"))

void Daemon::linkTo(DeviceLink* dl) {

    linkedDevices.append(dl);

    Q_FOREACH (PackageReceiver* pr, packageReceivers) {
        QObject::connect(dl,SIGNAL(receivedPackage(const NetworkPackage&)),
                            pr,SLOT(receivePackage(const NetworkPackage&)));
    }

}

Daemon::Daemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{

    qDebug() << "GO GO GO!";

    //TODO: Do not hardcode the load of the package receivers
    packageReceivers.push_back(new NotificationPackageReceiver());
    packageReceivers.push_back(new PauseMusicPackageReceiver());

    //TODO: Do not hardcode the load of the device locators
    announcers.insert(new AvahiAnnouncer());
    announcers.insert(new FakeAnnouncer());

    //Listen to incomming connections
    Q_FOREACH (Announcer* a, announcers) {
        QObject::connect(a,SIGNAL(deviceConnection(DeviceLink*)),
                            this,SLOT(deviceConnection(DeviceLink*)));
        a->setDiscoverable(true);
    }

    //TODO: Read paired devices from config
    //pairedDevices.push_back(new Device("MyAndroid","MyAndroid"));

}

QString Daemon::listVisibleDevices()
{

    std::stringstream ret;

    ret << std::setw(20) << "ID";
    ret << std::setw(20) << "Name";
    ret << std::endl;

    Q_FOREACH (Device* d, visibleDevices) {
        ret << std::setw(20) << d->id().toStdString();
        ret << std::setw(20) << d->name().toStdString();
        ret << std::endl;
    }

    return QString::fromStdString(ret.str());

}

bool Daemon::pairDevice(QString id)
{
    //TODO
    return true;
}

QString Daemon::listLinkedDevices()
{
    QString ret;

    Q_FOREACH (DeviceLink* dl, linkedDevices) {
        ret += dl->device()->name() + "(" + dl->device()->id() + ")";
    }

    return ret;

}


void Daemon::deviceConnection(DeviceLink* dl)
{

    QString id = dl->device()->id();
    bool paired = false;
    Q_FOREACH (Device* d, pairedDevices) {
        if (id == d->id()) {
            paired = true;
            break;
        }
    }

    visibleDevices.append(dl->device());

    if (paired) {
        qDebug() << "Known device connected" + dl->device()->name();
        linkTo(dl);
    }
    else {
        qDebug() << "Unknown device connected" + dl->device()->name();
        //TODO: Not connect immediately
        linkTo(dl);
    }

}

Daemon::~Daemon()
{
    qDebug() << "SAYONARA BABY";
}

