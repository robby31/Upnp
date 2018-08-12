#include "upnpcontrolpoint.h"

const QString UpnpControlPoint::UPNP_VERSION = "1.1";

const QHostAddress UpnpControlPoint::IPV4_UPNP_HOST = QHostAddress("239.255.255.250");

const quint16 UpnpControlPoint::UPNP_PORT = 1900;

UpnpControlPoint::UpnpControlPoint(QObject *parent):
    UpnpControlPoint(UPNP_PORT, parent)
{

}

UpnpControlPoint::UpnpControlPoint(quint16 eventPort, QObject *parent):
    QObject(parent),
    m_eventPort(eventPort),
    m_servername(QString("%1/%2 UPnP/%3 CTP/1.0").arg(QSysInfo::productType(), QSysInfo::productVersion(), UPNP_VERSION)),
    m_bootid(0),
    m_configid(0)
{
    m_remoteRootDevice = new DevicesModel(new UpnpRootDevice, this);
    connect(this, &UpnpControlPoint::messageReceived, m_remoteRootDevice, &DevicesModel::ssdpMessageReceived);
    connect(m_remoteRootDevice, &DevicesModel::removeSidEventFromUuid, this, &UpnpControlPoint::removeSidEventFromUuid);
    connect(m_remoteRootDevice, &DevicesModel::newRootDevice, this, &UpnpControlPoint::newRootDevice);
    connect(m_remoteRootDevice, &DevicesModel::subscribeEventing, this, &UpnpControlPoint::subscribeEventing);

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
        eventServer.setServerName(m_servername);

        // start event server
        if (!eventServer.listen(m_hostAddress, m_eventPort))
            qCritical() << "unable to start event server" << m_hostAddress.toString() << m_eventPort << eventServer.errorString();

        connect(&eventServer, &HttpServer::requestCompleted, this, &UpnpControlPoint::requestEventReceived);
    }

    m_eventCheckSubscription = startTimer(60000);
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

                if (m_remoteRootDevice)
                    m_remoteRootDevice->setMacAddress(m_macAddress);
            }
        }
    }
}

void UpnpControlPoint::close()
{
    if (m_remoteRootDevice)
        qDebug() << "Root devices" << m_remoteRootDevice->rowCount();

    for (int i=0;i<m_localRootDevice->rowCount();i++)
    {
        auto root = qobject_cast<UpnpRootDevice*>(m_localRootDevice->at(i));
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

DevicesModel *UpnpControlPoint::remoteRootDevices() const
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

void UpnpControlPoint::_sendMulticastSsdpMessage(const SsdpMessage &message)
{
    udpSocketUnicast.setSocketOption(QUdpSocket::MulticastTtlOption, 2);

    if (udpSocketUnicast.writeDatagram(message.toUtf8(), IPV4_UPNP_HOST, UPNP_PORT) == -1)
        qCritical() << "UPNPControlPoint: Unable to send multicast message.";
}

void UpnpControlPoint::_sendUnicastSsdpMessage(const QHostAddress &host, const quint16 &port, const SsdpMessage &message)
{
    if (udpSocketUnicast.writeDatagram(message.toUtf8(), host, port) == -1)
        qCritical() << "UPNPControlPoint: Unable to send unicast message.";
}

void UpnpControlPoint::_sendAliveMessage(const QString &uuid, const QString &nt)
{
    auto object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::T_RootDevice)
    {
        qCritical() << "ALIVE message shall be sent from RootDevice" << object->type() << object;
    }
    else
    {
        auto root = qobject_cast<UpnpRootDevice*>(object);

        QString usn;
        if (nt.isEmpty())
            usn = QString("uuid:%1").arg(uuid);
        else
            usn = QString("uuid:%1::%2").arg(uuid, nt);

        SsdpMessage message(NOTIFY);
        message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
        message.addHeader("CACHE-CONTROL", "max-age=1800");
        message.addHeader("LOCATION", root->url().toString());
        if (nt.isEmpty())
            message.addHeader("NT", usn);
        else
            message.addHeader("NT", nt);
        message.addHeader("NTS", SsdpMessage::ALIVE);
        message.addHeader("SERVER", root->serverName());
        message.addHeader("USN", usn);
        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(root->bootId()));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(root->configId()));

        _sendMulticastSsdpMessage(message);
    }
}

void UpnpControlPoint::_sendByeByeMessage(const QString &uuid, const QString &nt)
{
    auto object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::T_RootDevice)
    {
        qCritical() << "BYEBYE message shall be sent from RootDevice" << object->type() << object;
    }
    else
    {
        auto root = qobject_cast<UpnpRootDevice*>(object);

        QString usn;
        if (nt.isEmpty())
            usn = QString("uuid:%1").arg(uuid);
        else
            usn = QString("uuid:%1::%2").arg(uuid, nt);

        SsdpMessage message(NOTIFY);

        message.addHeader("HOST", QString("%1:%2").arg(IPV4_UPNP_HOST.toString()).arg(UPNP_PORT));
        if (nt.isEmpty())
            message.addHeader("NT", usn);
        else
            message.addHeader("NT", nt);
        message.addHeader("NTS", SsdpMessage::BYEBYE);
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
    message.addHeader("MAN", SsdpMessage::DISCOVER);
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

    if (m_remoteRootDevice)
        m_remoteRootDevice->setNetworkManager(nam);
}

void UpnpControlPoint::_processSsdpMessageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message)
{
    Q_UNUSED(port)

    if (message.format() == SEARCH)
    {
        QString man = message.getHeader("MAN");

        if (man == SsdpMessage::DISCOVER)
        {
            QString st = message.getHeader("ST");
            if (!st.isEmpty())
            {
                int delay = -1;
                if (!message.getHeader("MX").isNull())
                {
                    delay = QRandomGenerator::global()->bounded(message.getHeader("MX").toInt()*1000);
                    qDebug() << "SEARCH MULTICAST" << host << port << st << message.getHeader("MX") << "seconds." << delay;
                }

                if (delay > 0)
                {
                    qDebug() << "delay answer" << delay << "seconds";
                    int timerEvent = startTimer(delay);
                    if (timerEvent > 0)
                    {
                        m_searchAnswer[timerEvent].host = host;
                        m_searchAnswer[timerEvent].port = port;
                        m_searchAnswer[timerEvent].st = st;
                    }
                    else
                    {
                        qCritical() << "unable to start timer";
                        _searchForST(host, port, st);
                    }
                }
                else
                {
                    _searchForST(host, port, st);
                }
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
}

UpnpRootDevice *UpnpControlPoint::addLocalRootDevice(UpnpRootDeviceDescription *description, int port, const QString &url)
{
    UpnpRootDevice *device = new UpnpRootDevice(netManager, m_macAddress, QString(), m_localRootDevice);
    connect(device, &UpnpRootDevice::aliveMessage, this, &UpnpControlPoint::_sendAliveMessage);
    connect(device, &UpnpRootDevice::byebyeMessage, this, &UpnpControlPoint::_sendByeByeMessage);
    connect(device, &UpnpRootDevice::searchResponse, this, &UpnpControlPoint::_sendSearchResponse);

    description->setDeviceAttribute("UDN", QString("uuid:%1").arg(device->id()));
    device->setDescription(description);
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

        device->setAdvertise(true);

        m_localRootDevice->appendRow(device);

        return true;
    }

    return false;
}

void UpnpControlPoint::_searchForST(const QHostAddress &host, const int &port, const QString &st)
{
    for (int i=0;i<m_localRootDevice->rowCount();++i)
    {
        auto root = qobject_cast<UpnpRootDevice*>(m_localRootDevice->at(i));
        root->searchForST(host, port, st);
    }
}

void UpnpControlPoint::_sendSearchResponse(const QHostAddress &host, const quint16 &port, const QString &st, const QString &usn)
{
    auto object = qobject_cast<UpnpObject*>(sender());

    if (object->type() != UpnpObject::T_RootDevice)
    {
        qCritical() << "SEARCH response message shall be sent from RootDevice" << object->type() << object;
    }
    else
    {
        auto root = qobject_cast<UpnpRootDevice*>(object);

        SsdpMessage message(HTTP);
        message.addHeader("CACHE-CONTROL", "max-age=1800");
        message.addHeader("DATE", QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
        message.addHeader("EXT", "");
        message.addHeader("LOCATION", root->url().toString());
        message.addHeader("SERVER", root->serverName());
        message.addHeader("ST", st);
        message.addHeader("USN", usn);
        message.addHeader("BOOTID.UPNP.ORG", QString("%1").arg(root->bootId()));
        message.addHeader("CONFIGID.UPNP.ORG", QString("%1").arg(root->configId()));

        _sendUnicastSsdpMessage(host, port, message);
    }
}

QString UpnpControlPoint::eventSubscribed(const QString &uuid, const QString &serviceId)
{
    for (auto it = m_sidEvent.constBegin(); it != m_sidEvent.constEnd(); ++it)
    {
        if (it.value().deviceUuid == uuid && it.value().serviceId==serviceId)
            return it.key();  // return sid of events subscribed for serviceId on device identified by uuid
    }

    return QString();
}

void UpnpControlPoint::subscribeEventing(QNetworkRequest request, const QString &uuid, const QString &serviceId)
{
    QString sid = eventSubscribed(uuid, serviceId);

    if (sid.isEmpty())
    {
        request.setRawHeader("Connection", "close");
        request.setRawHeader("HOST", QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());
        request.setRawHeader("CALLBACK", QString("<http://%1:%2/event/%3/%4>").arg(host().toString()).arg(m_eventPort).arg(uuid, serviceId).toUtf8());

        QNetworkReply *reply = netManager->sendCustomRequest(request, "SUBSCRIBE");
        connect(reply, &QNetworkReply::finished, this, &UpnpControlPoint::subscribeEventingFinished);
        reply->setProperty("deviceUuid", uuid);
        reply->setProperty("serviceId", serviceId);
    }
    else
    {
        qDebug() << "event already subscribed" << uuid << serviceId;
    }
}

void UpnpControlPoint::subscribeEventingFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());

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

                qDebug() << "new event subscribed" << sid << reply->request().url() << reply->property("serviceId").toString();
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
                    UpnpError error(UpnpError::BAD_REQUEST);
                    request->replyError(error);
                }
                else
                {
                    if (m_sidEvent.contains(sid))
                    {
                        QString deviceUuid = m_sidEvent[sid].deviceUuid;
                        QString serviceId = m_sidEvent[sid].serviceId;

                        UpnpService *service = Q_NULLPTR;
                        if (m_remoteRootDevice)
                            service = m_remoteRootDevice->getService(deviceUuid, serviceId);
                        if (service)
                        {
                            service->updateStateVariables(event.variables());
                            request->sendHeader(QStringList());
                        }
                        else
                        {
                            qCritical() << "unable to find service" << serviceId << deviceUuid << request->peerAddress() << sid << seq;
                            UpnpError error(UpnpError::PRECONDITIN_FAILED);
                            request->replyError(error);
                        }
                    }
                    else
                    {
                        qCritical() << "available sid" << m_sidEvent.keys();
                        qCritical() << "invalid sid for eventing." << request->peerAddress().toString() << sid;
                        UpnpError error(UpnpError::PRECONDITIN_FAILED);
                        request->replyError(error);
                    }
                }
            }
            else
            {
                qCritical() << "invalid NTS for eventing" << nts;
                UpnpError error(UpnpError::PRECONDITIN_FAILED);
                request->replyError(error);
            }
        }
        else
        {
            qCritical() << "invalid NT for eventing" << nt;
            UpnpError error(UpnpError::PRECONDITIN_FAILED);
            request->replyError(error);
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
        auto it = m_sidEvent.begin();
        while (it != m_sidEvent.end())
        {
            QDateTime timeout = QDateTime::fromString(it.value().timeOut);
            if (timeout.isValid())
            {
                if (QDateTime::currentDateTime().secsTo(timeout) < 0)
                {
                    qCritical() << QDateTime::currentDateTime() << "event" << it.key() << "is obsolete" << timeout;

                    // cancel event
                    qWarning() << "remove sid from event subscribed" << it.key();
                    it = m_sidEvent.erase(it);
                    continue;
                }

                if (QDateTime::currentDateTime().secsTo(timeout) < 100)
                {
                    QString deviceUuid = it.value().deviceUuid;
                    QString serviceId = it.value().serviceId;

                    qDebug() << QDateTime::currentDateTime() << "renew event" << it.key() << deviceUuid << serviceId << timeout;

                    UpnpService *service = Q_NULLPTR;
                    if (m_remoteRootDevice)
                        service = m_remoteRootDevice->getService(deviceUuid, serviceId);
                    if (service)
                    {
                        // renew subscription
                        QNetworkRequest request(service->eventSubUrl());
                        request.setRawHeader("Connection", "close");
                        request.setRawHeader("HOST", QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());
                        request.setRawHeader("SID", it.key().toUtf8());
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
                        qWarning() << "remove sid from event subscribed" << it.key();
                        it = m_sidEvent.erase(it);
                        continue;
                    }
                }
            }
            else
            {
                qCritical() << "invalid timeout" << timeout << "for event sid" << it.key();

                // cancel event
                qWarning() << "remove sid from event subscribed" << it.key();
                it = m_sidEvent.erase(it);
                continue;
            }

            ++it;
        }
    }
    else if (m_searchAnswer.contains(event->timerId()))
    {
        T_SEARCH_ANSWER search = m_searchAnswer[event->timerId()];
        _searchForST(search.host, search.port, search.st);
        killTimer(event->timerId());
        m_searchAnswer.remove(event->timerId());
    }
    else if (m_discover.contains(event->timerId()))
    {
        m_discover[event->timerId()].counter--;
        sendDiscover(m_discover[event->timerId()].searchTarget);

        if (m_discover[event->timerId()].counter < 1)
        {
            m_discover.remove(event->timerId());
            killTimer(event->timerId());
        }
    }
    else
    {
        qCritical() << "invalid timer event" << event->timerId();
    }
}

void UpnpControlPoint::removeSidEventFromUuid(const QString &deviceUuid)
{
    qDebug() << "removeSidEventFromUuid" << deviceUuid;
    auto it = m_sidEvent.begin();
    while (it != m_sidEvent.end())
    {
        QString uuid = it.value().deviceUuid;
        if (uuid == deviceUuid)
        {
            qDebug() << "remove sid" << it.key();
            it = m_sidEvent.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

QString UpnpControlPoint::macAddress() const
{
    return m_macAddress;
}

void UpnpControlPoint::startDiscover(const QString &searchTarget)
{
    if (searchTarget.isEmpty())
    {
        qCritical() << "invalid search target to discover" << searchTarget;
    }
    else
    {
        // start event for discovering, emit 3 times
        int eventDiscover = startTimer(2000);
        if (eventDiscover > 0)
        {
            T_DISCOVER data;
            data.counter = 3;
            data.searchTarget = searchTarget;
            m_discover[eventDiscover] = data;
        }
        else
        {
            qCritical() << "unable to start discover event";
        }
    }
}
