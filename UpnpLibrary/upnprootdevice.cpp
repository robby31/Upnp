#include "upnprootdevice.h"

UpnpRootDevice::UpnpRootDevice(QObject *parent) :
    UpnpObject(parent),
    m_message(),
    m_uuid(),
    m_device(0),
    m_iconUrl()
{
    initRoles();   
}

UpnpRootDevice::UpnpRootDevice(QHostAddress host, QString uuid, SsdpMessage message, QObject *parent) :
    UpnpObject(RootDevice, host, parent),
    m_message(message),
    m_uuid(uuid),
    m_device(0),
    m_iconUrl()
{
    connect(this, SIGNAL(availableChanged()), this, SLOT(availableSlotChanged()));

    initRoles();
}

void UpnpRootDevice::initRoles()
{
    QHash<int, QByteArray> roles;
    roles[HostRole] = "host";
    roles[UuidRole] = "uuid";
    roles[ServerNameRole] = "servername";
    roles[FriendlyNameRole] = "friendlyname";
    roles[IconUrlRole] = "iconurl";
    roles[AvailableRole] = "available";
    roles[DeviceTypeRole] = "devicetype";
    roles[PresentationUrlRole] = "presentationurl";
    roles[VersionRole] = "version";
    setRoles(roles);
}

QString UpnpRootDevice::id() const
{
    return m_uuid;
}

QVariant UpnpRootDevice::data(int role) const
{
    switch (role) {
    case HostRole:
        return host().toString();
    case UuidRole:
        return m_uuid;
    case ServerNameRole:
        return servername();
    case FriendlyNameRole:
        return friendlyName();
    case IconUrlRole:
        return m_iconUrl;
    case AvailableRole:
        return available();
    case DeviceTypeRole:
        return deviceType();
    case PresentationUrlRole:
        return valueFromDescription("presentationURL");
    case VersionRole:
        return version();
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString UpnpRootDevice::version() const
{
    QDomDocument desc = description();

    if (!desc.isNull())
    {
        QDomNode root = desc.firstChildElement("root");
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

QString UpnpRootDevice::servername() const
{
    return getMessageHeader("SERVER");
}

QString UpnpRootDevice::friendlyName() const
{
    if (m_device)
        return m_device->friendlyName();
    else
        return QString();
}

QString UpnpRootDevice::getMessageHeader(const QString &param) const
{
    return m_message.getHeader(param);
}

QString UpnpRootDevice::deviceType() const
{
    if (m_device)
        return m_device->deviceType();
    else
        return QString();
}

bool UpnpRootDevice::deviceAvailable() const
{
    if (m_device)
        return m_device->data(UpnpDevice::AvailableRole).toBool();
    else
        return false;
}
void UpnpRootDevice::getDescription()
{
    QString p_url = getMessageHeader("LOCATION");

    QNetworkReply *reply = get(p_url);
    if (reply == 0)
    {
        qCritical() << "Unable to get description" << this << p_url;
    }
    else
    {
        connect(reply, SIGNAL(finished()), this, SLOT(descriptionReceived()));
    }
}

void UpnpRootDevice::descriptionReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    setUrl(reply->request().url());

    if (reply->error() == QNetworkReply::NoError)
    {
        setDescription(reply->readAll());

        // read description to initialize device and services
        QDomDocument desc = description();

        qDebug() << "description received" << this << reply->request().url();

        QDomElement root = desc.firstChildElement("root");
        if (root.isNull())
        {
            qCritical() << "root not found in description";
        }
        else
        {
            QDomElement device = root.firstChildElement("device");
            if (!device.isNull())
            {
                m_device = new UpnpDevice(host(), device, this);
                connect(m_device, SIGNAL(availableChanged()), this, SIGNAL(deviceAvailableChanged()));
                m_device->setNetworkManager(getNetworkManager());
                m_device->setUrl(url());
                m_device->readServices();
                m_device->readDevices();

                emit deviceTypeChanged();
                emit deviceAvailableChanged();

                // read UUID
                QDomElement udn = device.firstChildElement("UDN");
                if (!udn.isNull())
                {
                    QString uuid = udn.firstChild().nodeValue();
                    if (uuid.startsWith("uuid:"))
                    {
                        if (m_uuid != uuid.right(uuid.size()-5))
                            qCritical() << "uuid in description differs than uuid in creation" << m_uuid << uuid.right(uuid.size()-5);
                    }
                    else
                    {
                        qCritical() << "invalid UDN format" << uuid;
                    }
                }
                else
                {
                    qCritical() << "unable to find UUID in description";
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
            }
            else
            {
                qCritical() << "device not found in description";
            }
        }
    }
    else
    {
        qCritical() << reply->errorString();
    }

    reply->deleteLater();

    emit itemChanged();
}

void UpnpRootDevice::availableSlotChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}

UpnpDevice *UpnpRootDevice::getDeviceFromUuid(const QString &uuid)
{
    if (m_device)
        return m_device->getDeviceFromUuid(uuid);
    else
        return 0;
}

UpnpDevice *UpnpRootDevice::getDeviceFromType(const QString &type)
{
    if (m_device)
    {
        if (m_device->deviceType() == type)
            return m_device;
        else
            return m_device->getDeviceFromType(type);
    }
    else
        return 0;
}

UpnpService *UpnpRootDevice::getServiceFromType(const QString &type)
{
    if (m_device)
    {
        return m_device->getServiceFromType(type);
    }
    else
        return 0;
}

ListModel *UpnpRootDevice::devicesModel() const
{
    if (m_device)
        return m_device->devicesModel();
    else
        return 0;
}

ListModel *UpnpRootDevice::servicesModel() const
{
    if (m_device)
        return m_device->servicesModel();
    else
        return 0;
}

