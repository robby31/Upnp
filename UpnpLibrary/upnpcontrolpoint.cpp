#include "upnpcontrolpoint.h"

const QString UpnpControlPoint::UPNP_VERSION = "1.1";

const QString UpnpControlPoint::ALIVE = "ssdp:alive";

const QString UpnpControlPoint::BYEBYE = "ssdp:byebye";

const QString UpnpControlPoint::DISCOVER = "\"ssdp:discover\"";

const QHostAddress UpnpControlPoint::IPV4_UPNP_HOST = QHostAddress("239.255.255.250");

const int UpnpControlPoint::UPNP_PORT = 1900;

UpnpControlPoint::UpnpControlPoint(QObject *parent):
    UpnpControlPoint(UPNP_PORT, parent)
{

}

UpnpControlPoint::UpnpControlPoint(qint16 eventPort, QObject *parent):
    QObject(parent),
    netManager(0),
    m_eventPort(eventPort),
    m_eventCheckSubscription(-1),
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

    initializeHostAdress();

    connect(this, &UpnpControlPoint::messageReceived, this, &UpnpControlPoint::_processSsdpMessageReceived);

    connect(&udpSocketMulticast, &QUdpSocket::readyRead, this, &UpnpControlPoint::_processPendingMulticastDatagrams);
    connect(&udpSocketUnicast, &QUdpSocket::readyRead, this, &UpnpControlPoint::_processPendingUnicastDatagrams);

    if (!udpSocketMulticast.bind(QHostAddress::AnyIPv4, UPNP_PORT, QAbstractSocket::ShareAddress))
        qCritical() << "Unable to bind multicast address.";

    if (!udpSocketMulticast.joinMulticastGroup(IPV4_UPNP_HOST))
        qCritical() << "Unable to join multicast UDP.";

    if (m_hostAddress.isNull())
    {
        qCritical() << "invalid address" << m_hostAddress;
    }
    else
    {
        // start event server
        if (!eventServer.listen(m_hostAddress, m_eventPort))
            qCritical() << "unable to start event server" << m_hostAddress.toString() << m_eventPort << eventServer.errorString();

        connect(&eventServer, &HttpServer::requestCompleted, this, &UpnpControlPoint::requestEventReceived);
    }

    m_eventCheckSubscription = startTimer(10000);
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
            {
                m_hostAddress = entry.ip();
                m_macAddress = session.interface().hardwareAddress();
            }
        }
    }
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
    udpSocketUnicast.setSocketOption(QUdpSocket::MulticastTtlOption, 2);

    if (udpSocketUnicast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
        qCritical() << "UPNPControlPoint: Unable to send multicast message.";
}

void UpnpControlPoint::_sendUnicastSsdpMessage(const QHostAddress &host, const int &port, SsdpMessage message)
{
    if (udpSocketUnicast.writeDatagram(message.toUtf8(), host, port) == -1)
        qCritical() << "UPNPControlPoint: Unable to send unicast message.";
}

void UpnpControlPoint::_sendAliveMessage(const QString &uuid, const QString &nt)
{
    UpnpObject *object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::T_RootDevice)
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
        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(root->bootId()));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(root->configId()));

        _sendMulticastSsdpMessage(message);
    }
}

void UpnpControlPoint::_sendByeByeMessage(const QString &uuid, const QString &nt)
{
    UpnpObject *object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::T_RootDevice)
    {
        qCritical() << "BYEBYE message shall be sent from RootDevice" << object->type() << object;
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
        if (nt.isEmpty())
            message.addHeader("NT", usn);
        else
            message.addHeader("NT", nt);
        message.addHeader("NTS", BYEBYE);
        message.addHeader("USN", usn);
        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(root->bootId()));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(root->configId()));

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

    _sendMulticastSsdpMessage(message);
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
                    removeSidEventFromUuid(device->uuid());
                }
            }
            else
            {
                UpnpObject *object = getUpnpObjectFromUSN(message.getHeader("USN"));
                if (object != 0)
                {
                    object->setAvailable(false);
                    qWarning() << "object BYEBYE" << object;
                }
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
                if (!message.getHeader("MX").isNull())
                    qWarning() << "SEARCH MULTICAST" << host << port << message.getHeader("MX") << "seconds.";

                _searchForST(host, port, st);
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
            device = new UpnpRootDevice(netManager, m_macAddress, uuid, m_remoteRootDevice);
            connect(device, &UpnpRootDevice::availableChanged, this, &UpnpControlPoint::_rootDeviceAvailableChanged);
            connect(device, &UpnpRootDevice::statusChanged, this, &UpnpControlPoint::_rootDeviceStatusChanged);
            connect(device, &UpnpRootDevice::subscribeEventingSignal, this, &UpnpControlPoint::subscribeEventing);
            device->setServerName(serverName());
            device->update(message);
            device->setUrl(message.getHeader("LOCATION"));
            device->requestDescription();
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
        qCritical() << message.toStringList();
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

UpnpRootDevice *UpnpControlPoint::addLocalRootDevice(UpnpRootDeviceDescription *description, int port, QString url)
{
    UpnpRootDevice *device = new UpnpRootDevice(netManager, m_macAddress, QString(), m_localRootDevice);
    connect(device, &UpnpRootDevice::aliveMessage, this, &UpnpControlPoint::_sendAliveMessage);
    connect(device, &UpnpRootDevice::byebyeMessage, this, &UpnpControlPoint::_sendByeByeMessage);
    connect(device, &UpnpRootDevice::searchResponse, this, &UpnpControlPoint::_sendSearchResponse);

    description->setDeviceAttribute("UDN", QString("uuid:%1").arg(device->id()));
    device->setDescription(description);
    device->setServerName(serverName());
    device->setAdvertise(true);

    QUrl tmp(QString("http://%1:%2").arg(m_hostAddress.toString()).arg(port));
    device->setUrl(tmp.resolved(url));

    m_localRootDevice->appendRow(device);

    return device;
}

bool UpnpControlPoint::addLocalRootDevice(UpnpRootDevice *device)
{
    if (device)
    {
        device->setParent(m_localRootDevice);

        connect(device, &UpnpRootDevice::aliveMessage, this, &UpnpControlPoint::_sendAliveMessage);
        connect(device, &UpnpRootDevice::byebyeMessage, this, &UpnpControlPoint::_sendByeByeMessage);
        connect(device, &UpnpRootDevice::searchResponse, this, &UpnpControlPoint::_sendSearchResponse);

        device->setServerName(serverName());
        device->setAdvertise(true);

        m_localRootDevice->appendRow(device);

        return true;
    }
    else
    {
        return false;
    }
}

void UpnpControlPoint::_searchForST(const QHostAddress &host, const int &port, const QString &st)
{
    for (int i=0;i<m_localRootDevice->rowCount();++i)
    {
        UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(m_localRootDevice->at(i));
        root->searchForST(host, port, st);
    }
}

void UpnpControlPoint::_sendSearchResponse(const QHostAddress &host, const int &port, const QString &st, const QString &usn)
{
    UpnpObject *object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::T_RootDevice)
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
        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(root->bootId()));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(root->configId()));

        _sendUnicastSsdpMessage(host, port, message);
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
}

void UpnpControlPoint::subscribeEventing(QNetworkRequest request, const QString &uuid, const QString &serviceId)
{
    request.setRawHeader("Connection", "close");
    request.setRawHeader("HOST", QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());
    request.setRawHeader("CALLBACK", QString("<http://%1:%2/event/%3/%4>").arg(host().toString()).arg(m_eventPort).arg(uuid).arg(serviceId).toUtf8());

    QNetworkReply *reply = netManager->sendCustomRequest(request, "SUBSCRIBE");
    connect(reply, &QNetworkReply::finished, this, &UpnpControlPoint::subscribeEventingFinished);
    reply->setProperty("deviceUuid", uuid);
    reply->setProperty("serviceId", serviceId);
}

void UpnpControlPoint::subscribeEventingFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if (reply)
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qCritical() << "ERROR subscribe eventing" << reply->request().url().toString() << reply->error() << reply->errorString();
            qCritical() << reply->readAll();

            QString sid = reply->request().rawHeader("SID").trimmed();
            if (!sid.isEmpty() && m_sidEvent.contains(sid))
            {
                // cancel event
                qWarning() << "remove sid from event subscribed" << sid << reply->request().url().toString();
                m_sidEvent.remove(sid);
            }
        }
        else
        {
            QString sid = reply->rawHeader("SID").trimmed();

            if (!m_sidEvent.contains(sid))
            {
                qWarning() << "new event subscribe" << sid;
                m_sidEvent[sid].deviceUuid = reply->property("deviceUuid").toString();
                m_sidEvent[sid].serviceId = reply->property("serviceId").toString();

                QDateTime dateTimeOut;
                QString timeout = reply->rawHeader("TIMEOUT").trimmed();
                QRegularExpression pattern("^Second-(\\d+)$");
                QRegularExpressionMatch match = pattern.match(timeout);
                if (match.hasMatch())
                {
                    dateTimeOut = QDateTime::currentDateTime().addSecs(match.captured(1).toInt());
                    if (dateTimeOut.isValid())
                        m_sidEvent[sid].timeOut = dateTimeOut.toString();
                    else
                        m_sidEvent[sid].timeOut = reply->rawHeader("TIMEOUT").trimmed();
                }
                else
                {
                    qCritical() << "invalid format for TIMEOUT" << timeout;
                    m_sidEvent[sid].timeOut = reply->rawHeader("TIMEOUT").trimmed();
                }

                qDebug() << "new event subscribed" << sid << reply->request().url();
            }
            else
            {
                if (m_sidEvent[sid].deviceUuid != reply->property("deviceUuid").toString())
                {
                    qCritical() << "sid already known, invalid deviceUuid" << m_sidEvent[sid].deviceUuid << reply->property("deviceUuid").toString();
                }
                else if (m_sidEvent[sid].serviceId != reply->property("serviceId").toString())
                {
                    qCritical() << "sid already known, invalid serviceId" << m_sidEvent[sid].serviceId << reply->property("serviceId").toString();
                }
                else
                {
                    // update subscription timeout
                    QDateTime dateTimeOut;
                    QString timeout = reply->rawHeader("TIMEOUT").trimmed();
                    QRegularExpression pattern("^Second-(\\d+)$");
                    QRegularExpressionMatch match = pattern.match(timeout);
                    if (match.hasMatch())
                    {
                        dateTimeOut = QDateTime::currentDateTime().addSecs(match.captured(1).toInt());
                        if (dateTimeOut.isValid())
                            m_sidEvent[sid].timeOut = dateTimeOut.toString();
                        else
                            m_sidEvent[sid].timeOut = reply->rawHeader("TIMEOUT").trimmed();
                    }
                    else
                    {
                        qCritical() << "invalid format for TIMEOUT" << timeout;
                        m_sidEvent[sid].timeOut = reply->rawHeader("TIMEOUT").trimmed();
                    }
                }
            }
        }

        reply->deleteLater();
    }
    else
    {
        qCritical() << "invalid reply" << reply << sender();
    }
}

void UpnpControlPoint::requestEventReceived(HttpRequest *request)
{
    if (request)
    {        
        QString nt = request->header("NT");
        QString nts = request->header("NTS");
        QString sid = request->header("SID");
        QString seq = request->header("SEQ");

        qDebug() << "event received" << request->peerAddress() << sid << seq;

        if (nt == "upnp:event")
        {
            if (nts == "upnp:propchange")
            {
                qDebug() << "event property change" << request->requestData();
                EventResponse event(request->requestData());
                if (!event.isValid())
                {
                    qCritical() << "invalid eventing request received" << event.toString();
                }
                else
                {
                    if (m_sidEvent.contains(sid))
                    {
                        QString deviceUuid = m_sidEvent[sid].deviceUuid;
                        QString serviceId = m_sidEvent[sid].serviceId;

                        UpnpService *service = getService(deviceUuid, serviceId);
                        if (service)
                            service->updateStateVariables(event.variables());
                        else
                            qCritical() << "unable to find service" << serviceId << deviceUuid << request->peerAddress() << sid << seq;
                    }
                    else
                    {
                        qCritical() << "available sid" << m_sidEvent.keys();
                        qCritical() << "invalid sid for eventing." << request->peerAddress().toString() << sid;
                    }
                }
            }
            else
            {
                qCritical() << "invalid NTS for eventing" << nts;
            }
        }
        else
        {
            qCritical() << "invalid NT for eventing" << nt;
        }

        request->deleteLater();
    }
    else
    {
        qCritical() << "invalid event request" << request;
    }
}


void UpnpControlPoint::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_eventCheckSubscription)
    {
        // check eventing subscription
        foreach (const QString &sid, m_sidEvent.keys())
        {
            QDateTime timeout = QDateTime::fromString(m_sidEvent[sid].timeOut);
            if (timeout.isValid())
            {
                if (QDateTime::currentDateTime().secsTo(timeout) < 0)
                {
                    qCritical() << QDateTime::currentDateTime() << "event" << sid << "is obsolete" << timeout;

                    // cancel event
                    qWarning() << "remove sid from event subscribed" << sid;
                    m_sidEvent.remove(sid);
                }
                else if (QDateTime::currentDateTime().secsTo(timeout) < 100)
                {
                    QString deviceUuid = m_sidEvent[sid].deviceUuid;
                    QString serviceId = m_sidEvent[sid].serviceId;

                    qDebug() << QDateTime::currentDateTime() << "renew event" << sid << deviceUuid << serviceId << timeout;

                    UpnpService *service = getService(deviceUuid, serviceId);
                    if (service)
                    {
                        // renew subscription
                        QNetworkRequest request(service->eventSubUrl());
                        request.setRawHeader("Connection", "close");
                        request.setRawHeader("HOST", QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());
                        request.setRawHeader("SID", sid.toUtf8());
                        request.setRawHeader("TIMEOUT", "Second-300");

                        QNetworkReply *reply = netManager->sendCustomRequest(request, "SUBSCRIBE");
                        connect(reply, &QNetworkReply::finished, this, &UpnpControlPoint::subscribeEventingFinished);
                        reply->setProperty("deviceUuid", deviceUuid);
                        reply->setProperty("serviceId", serviceId);
                    }
                    else
                    {
                        qCritical() << "unable to find service for eventing renewing" << deviceUuid << serviceId;

                        // cancel event
                        qWarning() << "remove sid from event subscribed" << sid;
                        m_sidEvent.remove(sid);
                    }
                }
            }
            else
            {
                qCritical() << "invalid timeout" << timeout << "for event sid" << sid;

                // cancel event
                qWarning() << "remove sid from event subscribed" << sid;
                m_sidEvent.remove(sid);
            }
        }
    }
}

UpnpService *UpnpControlPoint::getService(const QString &deviceUuid, const QString &serviceId)
{
    UpnpDevice *device = qobject_cast<UpnpDevice*>(getUpnpObjectFromUSN(QString("uuid:%1").arg(deviceUuid)));
    if (device)
        return device->getService(serviceId);
    else
        return Q_NULLPTR;
}

void UpnpControlPoint::removeSidEventFromUuid(const QString &deviceUuid)
{
    qDebug() << "removeSidEventFromUuid" << deviceUuid;
    foreach (const QString &sid, m_sidEvent.keys())
    {
        QString uuid = m_sidEvent[sid].deviceUuid;
        if (uuid == deviceUuid)
        {
            m_sidEvent.remove(sid);
            qDebug() << "remove sid" << sid;
        }
    }
}

QString UpnpControlPoint::macAddress() const
{
    return m_macAddress;
}
