#include "upnpdevice.h"

UpnpDevice::UpnpDevice(QObject *parent) :
    UpnpObject(parent),
    m_uuid(),
    m_services(0),
    m_devices(0)
{
    initRoles();
}

UpnpDevice::UpnpDevice(QHostAddress host, QString uuid, QObject *parent) :
    UpnpObject(Device, host, parent),
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
            UpnpService *service = new UpnpService(host(), descr, m_services);
            service->setNetworkManager(getNetworkManager());
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
                UpnpDevice *device = new UpnpDevice(host(), strUuid, m_devices);
                device->setNetworkManager(getNetworkManager());
                device->setUrl(url());
                device->setDescription(descr);
                device->readServices();
                device->readDevices();
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

UpnpDevice *UpnpDevice::getDeviceFromUuid(const QString &p_uuid)
{
    if (uuid() == p_uuid)
        return this;

    for (int i=0;i<m_devices->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->at(i));

        UpnpDevice *foundDevice = device->getDeviceFromUuid(p_uuid);
        if (foundDevice != 0)
            return foundDevice;
    }

    return 0;
}

UpnpDevice *UpnpDevice::getDeviceFromType(const QString &type)
{
    if (deviceType() == type)
        return this;

    for (int i=0;i<m_devices->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->at(i));

        UpnpDevice *foundDevice = device->getDeviceFromType(type);
        if (foundDevice != 0)
            return foundDevice;
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

    for (int i=0;i<m_devices->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->at(i));

        UpnpService *foundService = device->getServiceFromType(type);
        if (foundService != 0)
            return foundService;
    }

    return 0;
}

void UpnpDevice::itemAvailableChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}
