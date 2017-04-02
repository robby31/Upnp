#include "upnprootdevice.h"

const QString UpnpRootDevice::UPNP_ROOTDEVICE = "upnp:rootdevice";

UpnpRootDevice::UpnpRootDevice(QObject *parent) :
    UpnpDevice(parent),
    netManager(0),
    m_servername(),
    m_url(),
    m_rootDescription(),
    m_iconUrl(),
    m_advertise(false),
    m_advertisingTimer(this),
    server(0)
{
    setType(RootDevice);

    initRoles();   
}

UpnpRootDevice::UpnpRootDevice(QNetworkAccessManager *nam, QString uuid, QObject *parent) :
    UpnpDevice(uuid, 0, parent),
    netManager(0),
    m_servername(),
    m_url(),
    m_rootDescription(),
    m_iconUrl(),
    m_advertise(false),
    m_advertisingTimer(3, 600000, this),
    server(0)
{
    setType(RootDevice);
    setNetworkManager(nam);

    initRoles();

    connect(this, SIGNAL(rootDescriptionChanged()), this, SIGNAL(itemChanged()));
    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
    connect(this, SIGNAL(urlChanged()), this, SLOT(requestDescription()));

    connect(this, SIGNAL(statusChanged()), this, SLOT(statusChangedSlot()));
    connect(&m_advertisingTimer, SIGNAL(timeout()), this, SLOT(sendAlive()));
}

void UpnpRootDevice::initRoles()
{
    QHash<int, QByteArray> roles;
    roles[HostRole] = "host";
    roles[UuidRole] = "uuid";
    roles[FriendlyNameRole] = "friendlyname";
    roles[IconUrlRole] = "iconurl";
    roles[AvailableRole] = "available";
    roles[DeviceTypeRole] = "devicetype";
    roles[PresentationUrlRole] = "presentationurl";
    roles[VersionRole] = "version";
    setRoles(roles);
}

QVariant UpnpRootDevice::data(int role) const
{
    switch (role) {
    case HostRole:
    {
        return host().toString();
    }
    case UuidRole:
    {
        return uuid();
    }
    case FriendlyNameRole:
    {
        return friendlyName();
    }
    case IconUrlRole:
    {
        return m_iconUrl;
    }
    case AvailableRole:
    {
        return available();
    }
    case DeviceTypeRole:
    {
        return deviceType();
    }
    case PresentationUrlRole:
    {
        return valueFromDescription("presentationURL");
    }
    case VersionRole:
    {
        return version();
    }
    default:
    {
        return QVariant::Invalid;
    }
    }

    return QVariant::Invalid;
}

QNetworkAccessManager *UpnpRootDevice::networkManager() const
{
    return netManager;
}

QString UpnpRootDevice::iconUrl() const
{
    return m_iconUrl;
}

QHostAddress UpnpRootDevice::host() const
{
    return QHostAddress(m_url.host());
}

int UpnpRootDevice::port() const
{
    return m_url.port();
}

void UpnpRootDevice::setNetworkManager(QNetworkAccessManager *nam)
{
    if (thread() != nam->thread())
        qWarning() << "NetworkManager and UpnpObject are in different thread.";

    netManager = nam;
}

QString UpnpRootDevice::serverName() const
{
    return m_servername;
}

void UpnpRootDevice::setServerName(const QString &name)
{
    m_servername = name;
}

QString UpnpRootDevice::version() const
{
    if (!m_rootDescription.isNull())
    {
        QDomNode root = m_rootDescription.firstChildElement("root");
        if (!root.isNull())
        {
            QDomNode specVersion = root.firstChildElement("specVersion");
            if (!specVersion.isNull())
            {
                QString major = specVersion.firstChildElement("major").firstChild().nodeValue();
                QString minor = specVersion.firstChildElement("minor").firstChild().nodeValue();
                if (!major.isNull() && !minor.isNull())
                    return QString("%1.%2").arg(major).arg(minor);
                else
                    qCritical() << "unable to find major and minor" << major << minor;
            }
            else
            {
                qCritical() << "unable to find specVersion";
            }
        }
        else
        {
            qCritical() << "unable to find root";
        }
    }

    return QString();
}

void UpnpRootDevice::requestDescription()
{
    qDebug() << "request description" << this;

    QNetworkRequest request(url());

    QNetworkReply *reply = get(request);
    if (reply == 0)
    {
        qCritical() << "Unable to get description" << this << url();
        setStatus(Error);
    }
    else
    {
        connect(reply, SIGNAL(finished()), this, SLOT(descriptionReceived()));
        setStatus(Loading);
    }
}

void UpnpRootDevice::setRootDescription(QByteArray data)
{
    m_rootDescription.setContent(data);
    emit rootDescriptionChanged();
}

QString UpnpRootDevice::rootDescription() const
{
    return m_rootDescription.toString();
}

void UpnpRootDevice::descriptionReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() == QNetworkReply::NoError)
    {
        setRootDescription(reply->readAll());

        qDebug() << "description received" << this << reply->request().url();

        QDomElement root = m_rootDescription.firstChildElement("root");
        if (root.isNull())
        {
            qCritical() << "root not found in description";
            setStatus(Error);
        }
        else
        {
            QDomElement device = root.firstChildElement("device");
            if (!device.isNull())
            {
                // check uuid
                QDomElement udnElt = device.firstChildElement("UDN");

                if (!udnElt.isNull())
                {
                    QString udn = udnElt.firstChild().nodeValue();

                    if (udn.startsWith("uuid:"))
                    {
                        if (udn.right(udn.size()-5) != uuid())
                        {
                            qCritical() << "invalid uuid in description" << uuid() << "!=" << udn.right(udn.size()-5);
                            setStatus(Error);
                        }
                    }
                    else
                    {
                        qCritical() << "invalid uuid in description" << udn;
                        setStatus(Error);
                    }
                }
                else
                {
                    qCritical() << "invalid uuid in description (no UDN found)";
                    setStatus(Error);
                }

                // read icon
                QDomElement l_icon = device.firstChildElement("iconList");
                if (!l_icon.isNull())
                {
                    QString iconUrl;
                    int iconWidth = -1;
                    QString iconMimeType;

                    QDomNode icon = l_icon.firstChild();
                    while (!icon.isNull())
                    {
                        int width = icon.firstChildElement("width").firstChild().nodeValue().toInt();
                        QString mimetype = icon.firstChildElement("mimetype").firstChild().nodeValue();
                        if (width > iconWidth)
                        {
                            // select larger icon
                            iconWidth = width;
                            iconUrl = icon.firstChildElement("url").firstChild().nodeValue();
                            iconMimeType = mimetype;
                        }

                        icon = icon.nextSibling();
                    }

                    // get icon data
                    QUrl iconLocation = urlFromRelativePath(iconUrl);
                    if (iconLocation.isValid())
                    {
                        m_iconUrl = iconLocation.url();

                        QVector<int> roles;
                        roles << IconUrlRole;
                        emit itemChanged(roles);
                    }
                }

                setDescription(device);
            }
            else
            {
                qCritical() << "device not found in description";
                setStatus(Error);
            }
        }
    }
    else
    {
        qCritical() << reply->errorString();
        setStatus(Error);
    }

    reply->deleteLater();
}

void UpnpRootDevice::itemAvailableChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}

QUrl UpnpRootDevice::url() const
{
    return m_url;
}

void UpnpRootDevice::setUrl(QUrl url)
{
    if (url.isValid())
    {
        m_url = url;
        emit urlChanged();
    }
    else
    {
        qCritical() << "invalid URL" << url;
    }
}

void UpnpRootDevice::statusChangedSlot()
{
    if (status() == Ready && m_advertise)
        startAdvertising();
}

void UpnpRootDevice::setAdvertise(const bool &flag)
{
    m_advertise = flag;

    if (status() == Ready && m_advertise)
        startAdvertising();
}

void UpnpRootDevice::startAdvertising()
{
    m_advertisingTimer.start(500);
}

void UpnpRootDevice::startServer()
{
    if (!server)
    {
        server = new HttpServer(this);
        server->setDeviceUuid(uuid());
        connect(server, SIGNAL(newRequest(HttpRequest*)), this, SIGNAL(newRequest(HttpRequest*)));
        connect(server, SIGNAL(requestCompleted(HttpRequest*)), this, SIGNAL(requestCompleted(HttpRequest*)));
    }

    if (!server->isListening())
    {
        if (!url().isValid())
        {
            qCritical() << "unable to start HTTP server, url is not valid" << url();
            emit serverError(QString("unable to start HTTP server, url %1 is not valid").arg(url().toString()));
        }
        else if (host().isNull())
        {
            qCritical() << "unable to start HTTP server, host address is null.";
            emit serverError(QString("unable to start HTTP server : host address is null"));
        }
        else if (port() <= 0)
        {
            qCritical() << "unable to start HTTP server, port is invalid" << port();
            emit serverError(QString("unable to start HTTP server : port %1 is invalid.").arg(port()));
        }
        else if (!server->listen(host(), port()))
        {
            qCritical() << "unable to start HTTP server" << this << host() << port();
            emit serverError(QString("unable to start HTTP server : %1 %2").arg(host().toString()).arg(port()));
        }
        else
        {
            qDebug() << "HTTP server started" << host() << port();
            emit serverStarted();
        }
    }
}

void UpnpRootDevice::sendAlive()
{
    emit aliveMessage(uuid(), UPNP_ROOTDEVICE);

    UpnpDevice::sendAlive();
}

void UpnpRootDevice::sendByeBye()
{
    emit byebyeMessage(uuid(), UPNP_ROOTDEVICE);

    UpnpDevice::sendByeBye();
}

void UpnpRootDevice::searchForST(const QString &st)
{
    if (st == "ssdp:all" or st == UPNP_ROOTDEVICE)
        emit searchResponse(UPNP_ROOTDEVICE, QString("uuid:%1::%2").arg(uuid()).arg(UPNP_ROOTDEVICE));

    if (st != UPNP_ROOTDEVICE)
    {
        if (description().isNull())
            qWarning() << "cannot answer to discover request, device is not ready" << this << st;
        else
            UpnpDevice::searchForST(st);
    }
}
