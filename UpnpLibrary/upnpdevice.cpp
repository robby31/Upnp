#include "upnpdevice.h"

UpnpDevice::UpnpDevice(QObject *parent) :
    UpnpObject(parent),
    m_info(),
    m_host(),
    m_services(0),
    m_devices(0)
{
    initRoles();
}

UpnpDevice::UpnpDevice(QHostAddress host, QDomNode info, QObject *parent) :
    UpnpObject(Device, host, parent),
    m_info(info),
    m_host(host),
    m_services(0),
    m_devices(0)
{
    initRoles();

    m_services = new ListModel(new UpnpService, this);
    m_devices = new ListModel(new UpnpDevice, this);

    connect(this, SIGNAL(availableChanged()), this, SLOT(availableSlotChanged()));

    if (description().isNull())
        setDescription(info);
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
    return getUuid();
}

QVariant UpnpDevice::data(int role) const
{
    switch (role) {
    case HostRole:
        return m_host.toString();
    case DeviceTypeRole:
        return deviceType();
    case AvailableRole:
        return available();
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}


QString UpnpDevice::getInfo(const QString &param) const
{
    QDomElement elt = m_info.firstChildElement(param);
    return elt.firstChild().nodeValue();
}

QString UpnpDevice::deviceType() const
{
    return getInfo("deviceType");
}

QString UpnpDevice::friendlyName() const
{
    return getInfo("friendlyName");
}

ListModel *UpnpDevice::servicesModel() const
{
    return m_services;
}

void UpnpDevice::readServices()
{
    QDomElement l_services = m_info.firstChildElement("serviceList");
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
    // read devices
    QDomElement l_devices = m_info.firstChildElement("deviceList");
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
            service->getDescription();
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
                UpnpDevice *device = new UpnpDevice(host(), descr, m_devices);
                device->setNetworkManager(getNetworkManager());
                device->setUrl(url());
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

QString UpnpDevice::getUuid() const
{
    QString udn = getInfo("UDN");

    if (udn.startsWith("uuid:"))
    {
        return udn.right(udn.size()-5);
    }
    else
    {
        return QString();
    }
}

ListModel *UpnpDevice::devicesModel() const
{
    return m_devices;
}

UpnpDevice *UpnpDevice::getDeviceFromUuid(const QString &uuid)
{
    for (int i=0;i<m_devices->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->at(i));
        if (device->getUuid() == uuid)
        {
            return device;
        }
    }

    return 0;
}

UpnpDevice *UpnpDevice::getDeviceFromType(const QString &type)
{
    for (int i=0;i<m_devices->rowCount();++i)
    {
        UpnpDevice *device = qobject_cast<UpnpDevice*>(m_devices->at(i));
        if (device->deviceType() == type)
        {
            return device;
        }
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

void UpnpDevice::availableSlotChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}
