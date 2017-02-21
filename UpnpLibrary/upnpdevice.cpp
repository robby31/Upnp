#include "upnpdevice.h"

UpnpDevice::UpnpDevice(QObject *parent) :
    UpnpObject(parent),
    m_uuid(),
    m_services(0),
    m_devices(0)
{
    initRoles();
}

UpnpDevice::UpnpDevice(QString uuid, QObject *parent) :
    UpnpObject(Device, parent),
    m_uuid(uuid),
    m_services(0),
    m_devices(0)
{
    initRoles();

    m_services = new ListModel(new UpnpService, this);
    m_devices = new ListModel(new UpnpDevice, this);

    connect(this, SIGNAL(descriptionChanged()), this, SIGNAL(deviceTypeChanged()));
    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
}

void UpnpDevice::initRoles()
{
    QHash<int, QByteArray> roles;
    roles[HostRole] = "host";
    roles[DeviceTypeRole] = "devicetype";
    roles[AvailableRole] = "available";
    setRoles(roles);
}

QString UpnpDevice::id() const
{
    return m_uuid;
}

QVariant UpnpDevice::data(int role) const
{
    switch (role) {
    case HostRole:
        return host().toString();
    case DeviceTypeRole:
        return deviceType();
    case AvailableRole:
        return available();
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString UpnpDevice::uuid() const
{
    return m_uuid;
}

QString UpnpDevice::deviceType() const
{
    return valueFromDescription("deviceType");
}

QString UpnpDevice::friendlyName() const
{
    return valueFromDescription("friendlyName");
}

ListModel *UpnpDevice::servicesModel() const
{
    return m_services;
}

void UpnpDevice::readServices()
{
    QDomNode desc = description();

    QDomElement l_services = desc.firstChildElement("serviceList");
    if (!l_services.isNull())
    {
        QDomNode serviceElt = l_services.firstChild();
        while (!serviceElt.isNull())
        {
            addService(serviceElt);
            serviceElt = serviceElt.nextSibling();
        }
    }
}

void UpnpDevice::readDevices()
{
    QDomNode desc = description();
    QDomElement l_devices = desc.firstChildElement("deviceList");
    if (!l_devices.isNull())
    {
        QDomNode deviceElt = l_devices.firstChild();
        while (!deviceElt.isNull())
        {
            addDevice(deviceElt);
            deviceElt = deviceElt.nextSibling();
        }
    }
}

void UpnpDevice::addService(const QDomNode &descr)
{
    QDomElement id = descr.firstChildElement("serviceId");
    if (id.isNull())
    {
        qCritical() << this << host().toString() << "unable to get serviceId";
    }
    else
    {
        QString serviceId = id.firstChild().nodeValue();

        UpnpService *service = qobject_cast<UpnpService*>(m_services->find(serviceId));

        if (service == 0)
        {
            UpnpService *service = new UpnpService(descr, m_services);
            service->setUpnpParent(this);
            service->setUrl(url());
            service->requestDescription();
            m_services->appendRow(service);
        }
        else
        {
            qCritical() << "service already exists" << serviceId;
        }
    }
}

void UpnpDevice::addDevice(const QDomNode &descr)
{
    QDomElement uuid = descr.firstChildElement("UDN");
    if (uuid.isNull())
    {
        qCritical() << "unable to get device uuid";
    }
    else
    {
        QString strUuid = uuid.firstChild().nodeValue();

        if (strUuid.startsWith("uuid:"))
        {
            strUuid = strUuid.right(strUuid.size()-5);

            UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->find(strUuid));

            if (device == 0)
            {
                UpnpDevice *device = new UpnpDevice(strUuid, m_devices);
                device->setUpnpParent(this);
                device->setUrl(url());
                device->setDescription(descr);
                device->readServices();
                device->readDevices();
                device->setStatus(Ready);
                m_devices->appendRow(device);
            }
            else
            {
                qCritical() << "device already exists" << strUuid;
            }
        }
        else
        {
            qCritical() << "invalid uuid" << strUuid;
        }
    }
}

ListModel *UpnpDevice::devicesModel() const
{
    return m_devices;
}

UpnpObject *UpnpDevice::getUpnpObjectFromUSN(const QString &usn)
{
    if (usn.startsWith("uuid:"))
    {
        QString tmp = usn.right(usn.size()-5);

        QString uuid;
        QString nt;
        if (tmp.contains("::"))
        {
            QStringList l_elt = tmp.split("::");
            uuid = l_elt.at(0);
            if (l_elt.size() == 2)
                nt = l_elt.at(1);
            else
                qCritical() << "invalid USN format" << usn;
        }
        else
        {
            uuid = tmp;
        }

        if (m_uuid == uuid)
        {
            if (nt.isEmpty() or nt == deviceType())
            {
                return this;
            }
            else if (nt.contains(":service:"))
            {
                UpnpObject *foundService = getServiceFromType(nt);
                if (foundService)
                    return foundService;
            }
        }
        else
        {
            for (int i=0;i<m_devices->rowCount();++i)
            {
                UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->at(i));

                UpnpObject *foundDevice = device->getUpnpObjectFromUSN(usn);
                if (foundDevice != 0)
                    return foundDevice;
            }
        }
    }
    else
    {
        qCritical() << "invalid USN format" << usn;
    }

    return 0;
}

UpnpService *UpnpDevice::getServiceFromType(const QString &type)
{
    for (int i=0;i<m_services->rowCount();++i)
    {
        UpnpService *service = qobject_cast<UpnpService*>(m_services->at(i));

        if (service->serviceType() == type)
        {
            return service;
        }
    }

    return 0;
}

void UpnpDevice::itemAvailableChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}
