#include "upnprootdevice.h"

UpnpRootDevice::UpnpRootDevice(QObject *parent) :
    UpnpObject(parent),
    m_message(),
    m_uuid(),
    m_services(0),
    m_devices(0),
    m_iconUrl()
{
    initRoles();

    m_services = new ListModel(new UpnpService, this);
    m_devices = new ListModel(new UpnpDevice, this);    
}

UpnpRootDevice::UpnpRootDevice(QHostAddress host, QString uuid, SsdpMessage message, QObject *parent) :
    UpnpObject(RootDevice, host, parent),
    m_message(message),
    m_uuid(uuid),
    m_services(0),
    m_devices(0),
    m_iconUrl()
{
    connect(this, SIGNAL(availableChanged()), this, SLOT(availableSlotChanged()));

    initRoles();

    m_services = new ListModel(new UpnpService, this);
    m_devices = new ListModel(new UpnpDevice, this);
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
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString UpnpRootDevice::servername() const
{
    return getMessageHeader("SERVER");
}

QString UpnpRootDevice::getMessageHeader(const QString &param) const
{
    return m_message.getHeader(param);
}

void UpnpRootDevice::addService(const QDomNode &descr)
{
    QDomElement type = descr.firstChildElement("serviceType");
    if (type.isNull())
    {
        qCritical() << "unable to get service type";
    }
    else
    {
        QString serviceType = type.firstChild().nodeValue();

        UpnpService *service = qobject_cast<UpnpService*>(m_services->find(serviceType));

        if (service == 0)
        {
            UpnpService *service = new UpnpService(host(), descr, m_services);
            service->setNetworkManager(getNetworkManager());
            service->setUrl(url());
            service->getDescription();
            m_services->appendRow(service);
        }
        else
        {
            qCritical() << "service already exists" << serviceType;
        }
    }
}

void UpnpRootDevice::addDevice(const QDomNode &descr)
{
    QDomElement type = descr.firstChildElement("deviceType");
    if (type.isNull())
    {
        qCritical() << "unable to get device type";
    }
    else
    {
        QString deviceType = type.firstChild().nodeValue();

        UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->find(deviceType));

        if (device == 0)
        {
            UpnpDevice *device = new UpnpDevice(host(), descr, m_devices);
            device->setNetworkManager(getNetworkManager());
            device->setUrl(url());
            m_devices->appendRow(device);
        }
        else
        {
            qCritical() << "device already exists" << deviceType;
        }
    }
}

ListModel *UpnpRootDevice::devicesModel() const
{
    return m_devices;
}

ListModel *UpnpRootDevice::servicesModel() const
{
    return m_services;
}

QString UpnpRootDevice::friendlyName() const
{
    return valueFromDescription("friendlyName");
}

QString UpnpRootDevice::deviceType() const
{
    return valueFromDescription("deviceType");
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

                // read devices
                QDomElement l_devices = device.firstChildElement("deviceList");
                if (!l_devices.isNull())
                {
                    QDomNode deviceElt = l_devices.firstChild();
                    while (!deviceElt.isNull())
                    {
                        addDevice(deviceElt);
                        deviceElt = deviceElt.nextSibling();
                    }
                }

                // read services
                QDomElement l_services = device.firstChildElement("serviceList");
                if (!l_services.isNull())
                {
                    QDomNode serviceElt = l_services.firstChild();
                    while (!serviceElt.isNull())
                    {
                        addService(serviceElt);
                        serviceElt = serviceElt.nextSibling();
                    }
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

void UpnpRootDevice::updateCapability(const SsdpMessage &message)
{
    if (!description().isNull())
    {
        // description of device received

        QString nt = message.getHeader("NT");

        if (deviceType() == nt or nt.startsWith("uuid:"))
        {
            update(message);
        }
        else if (nt.contains(":service:"))
        {
            UpnpService *service = qobject_cast<UpnpService*>(m_services->find(nt));
            if (service)
                service->update(message);
            else
                qCritical() << host().toString() << "unable to find service" << nt;
        }
        else if (nt.contains(":device:"))
        {
            UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->find(nt));
            if (device)
                device->update(message);
            else
                qCritical() << host().toString() << "unable to find device" << nt;
        }
        else
        {
            qCritical() << host().toString() << "unable to update capability" << nt;
        }
    }
}

void UpnpRootDevice::availableSlotChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}
