#include "upnpcontrolpoint.h"

const QString UpnpControlPoint::ALIVE = "ssdp:alive";

const QHostAddress UpnpControlPoint::IPV4_UPNP_HOST = QHostAddress("239.255.255.250");

const int UpnpControlPoint::UPNP_PORT = 1900;

const QString UpnpControlPoint::BYEBYE = "ssdp:byebye";

UpnpControlPoint::UpnpControlPoint(QObject *parent):
    QObject(parent),
    netManager(0),
    m_uuid(),
    m_servername(QString("%1/%2 UPnP/1.1 QMS/1.0").arg(QSysInfo::productType()).arg(QSysInfo::productVersion())),
    m_serverurl(),
    m_host(),
    udpSocketMulticast(this),
    udpSocketUnicast(this),
    timerAlive(this),
    counterAlive(0),
    m_bootid(0),
    m_configid(0),
    m_rootDevice(0)
{
    m_rootDevice = new ListModel(new UpnpRootDevice, this);

    connect(&udpSocketMulticast, SIGNAL(readyRead()), this, SLOT(_processPendingMulticastDatagrams()));
    udpSocketMulticast.setSocketOption(QAbstractSocket::MulticastTtlOption, 2);
    udpSocketMulticast.bind(QHostAddress::AnyIPv4, UPNP_PORT, QAbstractSocket::ShareAddress);
    if (!udpSocketMulticast.joinMulticastGroup(IPV4_UPNP_HOST))
        qCritical() << "Unable to join multicast UDP.";

    connect(&udpSocketUnicast, SIGNAL(readyRead()), this, SLOT(_processPendingUnicastDatagrams()));
    udpSocketUnicast.setSocketOption(QAbstractSocket::MulticastTtlOption, 2);

    connect(&timerAlive, SIGNAL(timeout()), this, SLOT(_sendAlive()));
    connect(this, SIGNAL(startSignal()), this, SLOT(_start()));

    connect(this, SIGNAL(messageReceived(QHostAddress,int,SsdpMessage)), this, SLOT(_processSsdpMessageReceived(QHostAddress,int,SsdpMessage)));
}

UpnpControlPoint::~UpnpControlPoint()
{
    qDebug() << "Close UPNPHelper.";
    close();
}

void UpnpControlPoint::_start()
{
    qDebug() << "Starting UPNPHelper";

    // Start timer to broadcast UPnP ALIVE messages every 0.5 seconds 3 times
    timerAlive.start(500);

    qDebug() << "discover rootdevice";
    _sendDiscover("upnp:rootdevice");
}

void UpnpControlPoint::close()
{
    qDebug() << "Root devices" << m_rootDevice->rowCount();

    if (timerAlive.isActive())
    {
        timerAlive.stop();
        _sendByeBye();
    }

    udpSocketUnicast.close();
    udpSocketMulticast.close();
}

// Function called when a request is received
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

void UpnpControlPoint::_sendDiscoverAnswer(const QHostAddress &host, const int &port, const QString &st)
{
    if (m_serverurl.isEmpty())
    {
        qCritical() << "Unable to send discovery message, server url is empty.";
    }
    else
    {
        QDateTime sdf;

        SsdpMessage message(HTTP);
        message.addHeader("CACHE-CONTROL", "max-age=1800");
        message.addHeader("DATE", sdf.currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss")+ "GMT");
        message.addHeader("EXT", "");
        message.addHeader("LOCATION", QString("%1/description/fetch").arg(m_serverurl));
        message.addHeader("SERVER", m_servername);
        message.addHeader("ST", st);

        if (st == QString("uuid:%1").arg(m_uuid))
            message.addHeader("USN", QString("uuid:%1").arg(m_uuid));
        else
            message.addHeader("USN", QString("uuid:%1::%2").arg(m_uuid).arg(st));

        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(m_bootid));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(m_configid));

        if (udpSocketUnicast.writeDatagram(message.toUtf8(), host, port) == -1)
            qCritical() << "UPNPHELPER: Unable to send message.";
    }
}

void UpnpControlPoint::_sendAlive(const QString &notification_type)
{
    if (m_serverurl.isEmpty())
    {
        qCritical() << "Unable to send discovery message, server url is empty.";
    }
    else
    {
        SsdpMessage message(NOTIFY);

        message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
        message.addHeader("CACHE-CONTROL", "max-age=1800");
        message.addHeader("LOCATION", m_serverurl+"/description/fetch");
        message.addHeader("NT", notification_type);
        message.addHeader("NTS", ALIVE);
        message.addHeader("SERVER", m_servername);

        if (notification_type == QString("uuid:%1").arg(m_uuid))
            message.addHeader("USN", QString("uuid:%1").arg(m_uuid));
        else
            message.addHeader("USN", QString("uuid:%1::%2").arg(m_uuid).arg(notification_type));

        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(m_bootid));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(m_configid));

        if (udpSocketMulticast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
            qCritical() << "UPNPHELPER: Unable to send message.";
    }
}

void UpnpControlPoint::_sendAlive()
{
    qDebug() << "Sending Alive...";

//    _sendAlive("upnp:rootdevice");
//    _sendAlive(QString("uuid:%1").arg(m_uuid));
//    _sendAlive("urn:schemas-upnp-org:device:MediaServer:1");

//    _sendAlive("urn:schemas-upnp-org:service:ContentDirectory:1");
//    _sendAlive("urn:schemas-upnp-org:service:ConnectionManager:1");

    if (counterAlive>1)
    {
        // after 3 Alive sent, sending every 10 minutes
        timerAlive.start(600000);

        // stop counting
        counterAlive = -1;
    }
    else if (counterAlive>=0)
    {
        // increment if counter > 0
        ++counterAlive;
    }
}

void UpnpControlPoint::_sendByeBye(const QString &notification_type)
{
    SsdpMessage message(NOTIFY);

    message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
    message.addHeader("NT", notification_type);
    message.addHeader("NTS", BYEBYE);

    if (notification_type == QString("uuid:%1").arg(m_uuid))
        message.addHeader("USN", QString("uuid:%1").arg(m_uuid));
    else
        message.addHeader("USN", QString("uuid:%1::%2").arg(m_uuid).arg(notification_type));

    message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(m_bootid));
    message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(m_configid));

    if (udpSocketMulticast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
        qCritical() << "UPNPHELPER: Unable to send message.";
}

void UpnpControlPoint::_sendByeBye()
{
    qDebug() << "Sending BYEBYE...";

//    _sendByeBye("upnp:rootdevice");
//    _sendByeBye("urn:schemas-upnp-org:device:MediaServer:1");
//    _sendByeBye("urn:schemas-upnp-org:service:ContentDirectory:1");
//    _sendByeBye("urn:schemas-upnp-org:service:ConnectionManager:1");
}

void UpnpControlPoint::_sendDiscover(const QString &search_target)
{
    SsdpMessage message(SEARCH);

    message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
    message.addHeader("MAN", "\"ssdp:discover\"");
    message.addHeader("MX", "1");
    message.addHeader("ST", search_target);
    message.addHeader("USER-AGENT", m_servername);

    if (udpSocketUnicast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
        qCritical() << "UPNPHELPER: Unable to send message.";
}

QNetworkReply *UpnpControlPoint::_sendAction(const QHostAddress &host, const int &port, const QString &url, const QString &service, const QString &action)
{

    if (netManager)
    {
        QNetworkRequest request;

        request.setUrl(QUrl(QString("http://%1:%2%3").arg(host.toString()).arg(port).arg(url)));
        request.setRawHeader(QByteArray("HOST"), m_host.toUtf8());
        request.setRawHeader(QByteArray("CONTENT-TYPE"), "text/xml; charset=\"utf-8\"");
        request.setRawHeader(QByteArray("USER-AGENT"), m_servername.toUtf8());
        request.setRawHeader(QByteArray("SOAPACTION"), QString("%1#%2").arg(service).arg(action).toUtf8());

        QDomDocument xml;
        xml.appendChild(xml.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));

        QDomElement envelope = xml.createElementNS("http://schemas.xmlsoap.org/soap/envelope/", "s:Envelope");
        envelope.setAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
        xml.appendChild(envelope);

        QDomElement body = xml.createElement("s:Body");
        envelope.appendChild(body);

        QDomElement xmlAction = xml.createElementNS(service, "u:"+action);
        body.appendChild(xmlAction);

        return netManager->post(request, xml.toByteArray());
    }
    else
    {
        return 0;
    }
}

void UpnpControlPoint::setNetworkManager(QNetworkAccessManager *nam)
{
    if (thread() != nam->thread())
        qWarning() << "NetworkManager and UpnpControlPoint are in different thread.";

    netManager = nam;
}

UpnpObject *UpnpControlPoint::getUpnpObjectFromNt(const QString &nt)
{
    if (nt.startsWith("uuid:"))
    {
        QString uuid = nt.right(nt.size()-5);

        UpnpDevice *device = getDeviceFromUuid(uuid);
        if (device != 0)
            return device;
        else
            qCritical() << "device/uuid not found" << uuid;
    }
    else if (nt.contains(":device:"))
    {
        UpnpDevice *device = getDeviceFromType(nt);
        if (device != 0)
            return device;
        else
            qCritical() << "device not found" << nt;
    }
    else if (nt.contains(":service:"))
    {
        UpnpService *service = getServiceFromType(nt);
        if (service != 0)
            return service;
        else
            qCritical() << "service not found" << nt;
    }
    else
    {
        qCritical() << "unable to find upnp object" << nt;
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

        if (nts == "ssdp:alive")
        {
            if (nt == "upnp:rootdevice")
            {
                addRootDevice(host, message);
            }
            else
            {
                UpnpObject *object = getUpnpObjectFromNt(nt);
                if (object != 0)
                    object->update(message);
                else
                    qCritical() << "unable to find" << host << nt;
            }
        }
        else if (nts == "ssdp:byebye")
        {
            if (nt == "upnp:rootdevice")
            {
                UpnpRootDevice *device = getRootDeviceFromUuid(message.getUuidFromUsn());

                if (device != 0)
                    device->setAvailable(false);
                else
                    qCritical() << "root device not found" << message.getUuidFromUsn();
            }
            else
            {
                UpnpObject *object = getUpnpObjectFromNt(nt);
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

        if (st == "upnp:rootdevice")
        {
            addRootDevice(host, message);
        }
        else
        {
            qCritical() << "unknown st" << st;
        }
    }
}

void UpnpControlPoint::addRootDevice(QHostAddress host, SsdpMessage message)
{
    // read uuid
    QString uuid = message.getUuidFromUsn();

    if (!uuid.isEmpty())
    {
        UpnpRootDevice *device = getRootDeviceFromUuid(uuid);

        if (device == 0)
        {
            device = new UpnpRootDevice(QHostAddress(host.toIPv4Address()), uuid, m_rootDevice);
            device->setNetworkManager(netManager);
            device->update(message);
            device->requestDescription(message.getHeader("LOCATION"));
            m_rootDevice->appendRow(device);
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
        UpnpRootDevice *device = qobject_cast<UpnpRootDevice*>(m_rootDevice->find(uuid));
        return device;
    }

    return 0;
}

UpnpDevice *UpnpControlPoint::getDeviceFromUuid(const QString &uuid)
{
    for (int i=0;i<m_rootDevice->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpRootDevice*>(m_rootDevice->at(i))->getDeviceFromUuid(uuid);
        if (device != 0)
            return device;
    }

    return 0;
}

UpnpDevice *UpnpControlPoint::getDeviceFromType(const QString &type)
{
    for (int i=0;i<m_rootDevice->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpRootDevice*>(m_rootDevice->at(i))->getDeviceFromType(type);
        if (device != 0)
            return device;
    }

    return 0;
}

UpnpService *UpnpControlPoint::getServiceFromType(const QString &type)
{
    for (int i=0;i<m_rootDevice->rowCount();++i)
    {
        UpnpService *service = qobject_cast<UpnpRootDevice*>(m_rootDevice->at(i))->getServiceFromType(type);
        if (service != 0)
            return service;
    }

    return 0;
}
