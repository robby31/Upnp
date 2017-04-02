#include "upnpcontrolpoint.h"

const QString UpnpControlPoint::UPNP_VERSION = "1.1";

const QString UpnpControlPoint::ALIVE = "ssdp:alive";

const QString UpnpControlPoint::BYEBYE = "ssdp:byebye";

const QString UpnpControlPoint::DISCOVER = "\"ssdp:discover\"";

const QHostAddress UpnpControlPoint::IPV4_UPNP_HOST = QHostAddress("239.255.255.250");

const int UpnpControlPoint::UPNP_PORT = 1900;

UpnpControlPoint::UpnpControlPoint(QObject *parent):
    QObject(parent),
    netManager(0),
    m_servername(QString("%1/%2 UPnP/%3 QMS/1.0").arg(QSysInfo::productType()).arg(QSysInfo::productVersion()).arg(UPNP_VERSION)),
    m_hostAddress(),
    udpSocketMulticast(this),
    udpSocketUnicast(this),
    m_bootid(0),
    m_configid(0),
    m_remoteRootDevice(0),
    m_localRootDevice(0)
{
    m_remoteRootDevice = new ListModel(new UpnpRootDevice, this);
    m_localRootDevice = new ListModel(new UpnpRootDevice, this);

    connect(this, &UpnpControlPoint::messageReceived, this, &UpnpControlPoint::_processSsdpMessageReceived);

    connect(&udpSocketMulticast, &QUdpSocket::readyRead, this, &UpnpControlPoint::_processPendingMulticastDatagrams);
    connect(&udpSocketUnicast, &QUdpSocket::readyRead, this, &UpnpControlPoint::_processPendingUnicastDatagrams);

    udpSocketMulticast.setSocketOption(QAbstractSocket::MulticastTtlOption, 4);
    if (!udpSocketMulticast.bind(QHostAddress::AnyIPv4, UPNP_PORT, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint))
        qCritical() << "Unable to bound multicast address.";

    if (!udpSocketMulticast.joinMulticastGroup(IPV4_UPNP_HOST))
        qCritical() << "Unable to join multicast UDP.";

    udpSocketUnicast.setSocketOption(QAbstractSocket::MulticastTtlOption, 4);

    initializeHostAdress();
}

UpnpControlPoint::~UpnpControlPoint()
{
    qDebug() << "Close UPNPControlPoint.";
    close();
}

void UpnpControlPoint::initializeHostAdress()
{
    QNetworkConfigurationManager mgr;
    foreach(QNetworkConfiguration ap, mgr.allConfigurations(QNetworkConfiguration::Active))
    {
        QNetworkSession session(ap);
        qDebug() << "SESSION" << session.interface().humanReadableName();

        foreach (QNetworkAddressEntry entry, session.interface().addressEntries())
        {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                m_hostAddress = entry.ip();
        }
    }

    if (m_hostAddress.isNull())
        qCritical() << "invalid address" << m_hostAddress;
}

void UpnpControlPoint::close()
{
    qDebug() << "Root devices" << m_remoteRootDevice->rowCount();

    for (int i=0;i<m_localRootDevice->rowCount();i++)
    {
        UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(m_localRootDevice->at(i));
        root->sendByeBye();
    }

    udpSocketUnicast.close();
    udpSocketMulticast.close();
}

QString UpnpControlPoint::serverName() const
{
    return m_servername;
}

QHostAddress UpnpControlPoint::host() const
{
    return m_hostAddress;
}

ListModel *UpnpControlPoint::localRootDevices() const
{
    return m_localRootDevice;
}

ListModel *UpnpControlPoint::remoteRootDevices() const
{
    return m_remoteRootDevice;
}

void UpnpControlPoint::_processPendingUnicastDatagrams()
{
    while (udpSocketUnicast.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocketUnicast.pendingDatagramSize());

        QHostAddress remoteAddr;
        quint16 remotePort;
        udpSocketUnicast.readDatagram(datagram.data(), datagram.size(), &remoteAddr, &remotePort);

        SsdpMessage message = SsdpMessage::fromByteArray(datagram);

        qDebug() << "Receiving UNICAST message from [" << remoteAddr.toString() << ":" << QString("%1").arg(remotePort) << "] " << message.startLine();
        emit messageReceived(remoteAddr, remotePort, message);
    }
}

void UpnpControlPoint::_processPendingMulticastDatagrams()
{
    while (udpSocketMulticast.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocketMulticast.pendingDatagramSize());

        QHostAddress remoteAddr;
        quint16 remotePort;
        udpSocketMulticast.readDatagram(datagram.data(), datagram.size(), &remoteAddr, &remotePort);

        SsdpMessage message = SsdpMessage::fromByteArray(datagram);

        qDebug() << "Receiving MULTICAST message from [" << remoteAddr.toString() << ":" << QString("%1").arg(remotePort) << "] " << message.startLine();
        emit messageReceived(remoteAddr, remotePort, message);
    }
}

void UpnpControlPoint::_sendMulticastSsdpMessage(SsdpMessage message)
{
    if (udpSocketMulticast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
        qCritical() << "UPNPControlPoint: Unable to send message.";
}

void UpnpControlPoint::_sendAliveMessage(const QString &uuid, const QString &nt)
{
    UpnpObject *object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::RootDevice)
    {
        qCritical() << "ALIVE message shall be sent from RootDevice" << object->type() << object;
    }
    else
    {
        UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(object);

        QString usn;
        if (nt.isEmpty())
            usn = QString("uuid:%1").arg(uuid);
        else
            usn = QString("uuid:%1::%2").arg(uuid).arg(nt);

        SsdpMessage message(NOTIFY);
        message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
        message.addHeader("CACHE-CONTROL", "max-age=1800");
        message.addHeader("LOCATION", root->url().toString());
        if (nt.isEmpty())
            message.addHeader("NT", usn);
        else
            message.addHeader("NT", nt);
        message.addHeader("NTS", ALIVE);
        message.addHeader("SERVER", m_servername);
        message.addHeader("USN", usn);

//        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(m_bootid));
//        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(m_configid));

        _sendMulticastSsdpMessage(message);
    }
}

void UpnpControlPoint::_sendByeByeMessage(const QString &uuid, const QString &nt)
{
    UpnpObject *object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::RootDevice)
    {
        qCritical() << "BYEBYE message shall be sent from RootDevice" << object->type() << object;
    }
    else
    {
        QString usn;
        if (nt.isEmpty())
            usn = QString("uuid:%1").arg(uuid);
        else
            usn = QString("uuid:%1::%2").arg(uuid).arg(nt);

        SsdpMessage message(NOTIFY);

        message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
        if (nt.isEmpty())
            message.addHeader("NT", usn);
        else
            message.addHeader("NT", nt);
        message.addHeader("NTS", BYEBYE);
        message.addHeader("USN", usn);

//        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(m_bootid));
//        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(m_configid));

        _sendMulticastSsdpMessage(message);
    }
}


void UpnpControlPoint::sendDiscover(const QString &search_target)
{
    qDebug() << "discover" << search_target;

    SsdpMessage message(SEARCH);

    message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
    message.addHeader("MAN", DISCOVER);
    message.addHeader("MX", "1");
    message.addHeader("ST", search_target);
    message.addHeader("USER-AGENT", m_servername);

    if (udpSocketUnicast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
        qCritical() << "UPNPControlPoint: Unable to send discovery message.";
}

void UpnpControlPoint::setNetworkManager(QNetworkAccessManager *nam)
{
    if (thread() != nam->thread())
        qWarning() << "NetworkManager and UpnpControlPoint are in different thread.";

    netManager = nam;
}

UpnpObject *UpnpControlPoint::getUpnpObjectFromUSN(const QString &usn)
{
    for (int i=0;i<m_remoteRootDevice->rowCount();++i)
    {
        UpnpObject *object = qobject_cast<UpnpRootDevice*>(m_remoteRootDevice->at(i))->getUpnpObjectFromUSN(usn);
        if (object != 0)
            return object;
    }

    return 0;
}

void UpnpControlPoint::_processSsdpMessageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message)
{
    Q_UNUSED(port)

    if (message.format() == NOTIFY)
    {
        QString nts = message.getHeader("NTS");
        QString nt = message.getHeader("NT");

        if (nts == ALIVE)
        {
            if (nt == UpnpRootDevice::UPNP_ROOTDEVICE)
            {
                addRootDevice(message);
            }
            else
            {
                QString uuid = message.getUuidFromUsn();
                UpnpObject *device = getUpnpObjectFromUSN(QString("uuid:%1").arg(uuid));
                if (device)
                {
                    if (device->status() == UpnpObject::Ready)
                    {
                        UpnpObject *object = getUpnpObjectFromUSN(message.getHeader("USN"));
                        if (object != 0)
                            object->update(message);
                        else
                            qCritical() << "unable to find" << host << nt;
                    }
                }
                else
                {
                    qCritical() << "unable to find device" << host << uuid;
                }
            }
        }
        else if (nts == BYEBYE)
        {
            if (nt == UpnpRootDevice::UPNP_ROOTDEVICE)
            {
                UpnpRootDevice *device = getRootDeviceFromUuid(message.getUuidFromUsn());

                if (device != 0)
                {
                    device->setAvailable(false);
                }
                else
                {
                    qCritical() << "root device not found" << message.getUuidFromUsn();
                    qCritical() << message.toStringList();
                }
            }
            else
            {
                UpnpObject *object = getUpnpObjectFromUSN(message.getHeader("USN"));
                if (object != 0)
                    object->setAvailable(false);
                else
                    qCritical() << "device or servce not found" << nt;
            }
        }
        else
        {
            qCritical() << "Invalid NTS value" << nts << nt;
        }
    }
    else if (message.format() == HTTP)
    {
        QString st = message.getHeader("ST");

        if (st == UpnpRootDevice::UPNP_ROOTDEVICE)
        {
            addRootDevice(message);
        }
        else if (st.isEmpty())
        {
            qCritical() << "invalid st" << st;
            qCritical() << message.toStringList();
        }
    }
    else if (message.format() == SEARCH)
    {
        QString man = message.getHeader("MAN");

        if (man == DISCOVER)
        {
            QString st = message.getHeader("ST");
            if (!st.isEmpty())
            {
                _searchForST(st);
            }
            else
            {
                qCritical() << "invalid ST value";
            }

        }
        else
        {
            qCritical() << "invalid MAN value" << man;
        }
    }
    else
    {
        qCritical() << "invalid message" << message.toStringList();
    }
}

void UpnpControlPoint::addRootDevice(SsdpMessage message)
{
    // read uuid
    QString uuid = message.getUuidFromUsn();

    if (!uuid.isEmpty())
    {
        UpnpRootDevice *device = getRootDeviceFromUuid(uuid);

        if (device == 0)
        {
            device = new UpnpRootDevice(netManager, uuid, m_remoteRootDevice);
            connect(device, SIGNAL(availableChanged()), this, SLOT(_rootDeviceAvailableChanged()));
            connect(device, SIGNAL(statusChanged()), this, SLOT(_rootDeviceStatusChanged()));
            device->setServerName(serverName());
            device->update(message);
            device->setUrl(message.getHeader("LOCATION"));
            m_remoteRootDevice->appendRow(device);
        }
        else
        {
            device->update(message);
        }
    }
    else
    {
        qCritical() << "invalid uuid in message" << message.getHeader("USN");
    }
}

UpnpRootDevice *UpnpControlPoint::getRootDeviceFromUuid(const QString &uuid)
{
    if (!uuid.isEmpty())
    {
        UpnpRootDevice *device = qobject_cast<UpnpRootDevice*>(m_remoteRootDevice->find(uuid));
        return device;
    }

    return 0;
}

UpnpRootDevice *UpnpControlPoint::addLocalRootDevice(int port, QString uuid, QString url)
{
    UpnpRootDevice *device = new UpnpRootDevice(netManager, uuid, m_localRootDevice);
    connect(device, SIGNAL(aliveMessage(QString,QString)), this, SLOT(_sendAliveMessage(QString,QString)));
    connect(device, SIGNAL(byebyeMessage(QString,QString)), this, SLOT(_sendByeByeMessage(QString,QString)));
    connect(device, SIGNAL(searchResponse(QString,QString)), this, SLOT(_sendSearchResponse(QString,QString)));
    device->setServerName(serverName());
    device->setAdvertise(true);

    QUrl tmp(QString("http://%1:%2").arg(m_hostAddress.toString()).arg(port));
    device->setUrl(tmp.resolved(url));

    m_localRootDevice->appendRow(device);

    return device;
}

void UpnpControlPoint::advertiseLocalRootDevice()
{
    for (int i=0;i<m_localRootDevice->rowCount();++i)
    {
        UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(m_localRootDevice->at(i));
        root->sendAlive();
    }
}

void UpnpControlPoint::_searchForST(const QString &st)
{
    for (int i=0;i<m_localRootDevice->rowCount();++i)
    {
        UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(m_localRootDevice->at(i));
        root->searchForST(st);
    }
}

void UpnpControlPoint::_sendSearchResponse(const QString &st, const QString &usn)
{
    UpnpObject *object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::RootDevice)
    {
        qCritical() << "SEARCH response message shall be sent from RootDevice" << object->type() << object;
    }
    else
    {
        UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(object);

        QDateTime sdf;

        SsdpMessage message(HTTP);
        message.addHeader("CACHE-CONTROL", "max-age=1800");
        message.addHeader("DATE", sdf.currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
        message.addHeader("EXT", "");
        message.addHeader("LOCATION", root->url().toString());
        message.addHeader("SERVER", m_servername);
        message.addHeader("ST", st);
        message.addHeader("USN", usn);

//        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(m_bootid));
//        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(m_configid));

        _sendMulticastSsdpMessage(message);
    }
}

void UpnpControlPoint::_rootDeviceAvailableChanged()
{
    UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(sender());

    if (root->available() == false)
    {
        QModelIndex index = m_remoteRootDevice->indexFromItem(root);
        if (index.isValid())
            m_remoteRootDevice->removeRow(index.row());
    }

}

void UpnpControlPoint::_rootDeviceStatusChanged()
{
    UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(sender());

    if (root->status() == UpnpObject::Ready)
        emit newRootDevice(root);

    advertiseLocalRootDevice();
}
