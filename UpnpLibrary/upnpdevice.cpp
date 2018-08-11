#include "upnpdevice.h"

UpnpDevice::UpnpDevice(QObject *parent) :
    UpnpObject(parent),
    m_uuid()
{
    initRoles();
}

UpnpDevice::UpnpDevice(const QString &uuid, UpnpObject *upnpParent, QObject *parent) :
    UpnpObject(T_Device, upnpParent, parent),
    m_uuid(uuid)
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
    roles[PresentationUrlRole] = "presentationurl";
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
    {
        return host().toString();
    }
    case DeviceTypeRole:
    {
        return deviceType();
    }
    case AvailableRole:
    {
        return available();
    }
    case PresentationUrlRole:
    {
        return valueFromDescription("presentationURL");
    }
    default:
    {
        return QVariant::Invalid;
    }
    }
}

QString UpnpDevice::uuid() const
{
    return m_uuid;
}

void UpnpDevice::setUuid(const QString &uuid)
{
    if (m_uuid.isEmpty())
        m_uuid = uuid;
    else
        qCritical() << "unable to set uuid, uuid is already initialised.";
}

QString UpnpDevice::deviceType() const
{
    auto descr = qobject_cast<UpnpDeviceDescription*>(description());
    if (descr)
        return descr->deviceAttribute("deviceType");

    return QString();
}

QString UpnpDevice::friendlyName() const
{
    auto descr = qobject_cast<UpnpDeviceDescription*>(description());
    if (descr)
        return descr->deviceAttribute("friendlyName");

    return QString();
}

ListModel *UpnpDevice::servicesModel() const
{
    return m_services;
}

void UpnpDevice::parseObject()
{
    readDevices();

    readServices();

    if (status() != Error)
        setStatus(Ready);
}

void UpnpDevice::readServices()
{
    auto descr = qobject_cast<UpnpDeviceDescription*>(description());
    if (descr)
    {
        foreach (const QDomElement &service, descr->services())
            addService(service);
    }
}

void UpnpDevice::readDevices()
{
    auto descr = qobject_cast<UpnpDeviceDescription*>(description());
    if (descr)
    {
        foreach (const QDomElement &device, descr->devices())
            addDevice(device);
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

        UpnpService *service = getService(serviceId);

        if (!service)
        {
            auto service = new UpnpService(this, descr, m_services);
            connect(service, SIGNAL(aliveMessage(QString,QString)), this, SIGNAL(aliveMessage(QString,QString)));
            connect(service, SIGNAL(byebyeMessage(QString,QString)), this, SIGNAL(byebyeMessage(QString,QString)));
            connect(service, &UpnpService::searchResponse, this, &UpnpDevice::searchResponse);
            connect(service, SIGNAL(subscribeEventingSignal(QNetworkRequest,QString)), this, SLOT(subscribeEventingSlot(QNetworkRequest,QString)));
            m_services->appendRow(service);
            service->requestDescription();
        }
        else
        {
            qCritical() << "service already exists" << serviceId;
        }
    }
}

bool UpnpDevice::addService(UpnpService *p_service)
{
    if (p_service->serviceId().isNull())
        return false;

    UpnpService *service = getService(p_service->serviceId());
    if (!service)
    {
        // update xml description of the device
        auto desc = qobject_cast<UpnpDeviceDescription*>(description());
        if (desc)
        {
            if (!desc->addService(p_service))
            {
                qCritical() << "unable to update description of the device";
                return false;
            }
        }
        else
        {
            qCritical() << "invalid description" << this;
            return false;
        }

        p_service->setParent(m_services);
        connect(p_service, SIGNAL(aliveMessage(QString,QString)), this, SIGNAL(aliveMessage(QString,QString)));
        connect(p_service, SIGNAL(byebyeMessage(QString,QString)), this, SIGNAL(byebyeMessage(QString,QString)));
        connect(p_service, &UpnpService::searchResponse, this, &UpnpDevice::searchResponse);
        connect(p_service, SIGNAL(subscribeEventingSignal(QNetworkRequest,QString)), this, SLOT(subscribeEventingSlot(QNetworkRequest,QString)));
        m_services->appendRow(p_service);

        return true;
    }

    qCritical() << "unable to add service, service already exists" << p_service->serviceId();
    return false;
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

            auto device = qobject_cast<UpnpDevice*>(m_devices->find(strUuid));

            if (!device)
            {
                auto device = new UpnpDevice(strUuid, this, m_devices);
                connect(device, SIGNAL(aliveMessage(QString,QString)), this, SIGNAL(aliveMessage(QString,QString)));
                connect(device, SIGNAL(byebyeMessage(QString,QString)), this, SIGNAL(byebyeMessage(QString,QString)));
                connect(device, &UpnpDevice::searchResponse, this, &UpnpDevice::searchResponse);
                connect(device, SIGNAL(subscribeEventingSignal(QNetworkRequest,QString,QString)), this, SIGNAL(subscribeEventingSignal(QNetworkRequest,QString,QString)));
                UpnpDeviceDescription *description = new UpnpDeviceDescription();
                description->setContent(descr);
                device->setDescription(description);
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
            if (nt.isEmpty() || nt == deviceType())
                return this;

            if (nt.contains(":service:"))
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
                auto device = qobject_cast<UpnpDevice*>(m_devices->at(i));

                UpnpObject *foundDevice = device->getUpnpObjectFromUSN(usn);
                if (foundDevice)
                    return foundDevice;
            }
        }
    }
    else
    {
        qCritical() << "invalid USN format" << usn;
    }

    return Q_NULLPTR;
}

UpnpService *UpnpDevice::getServiceFromType(const QString &type)
{
    for (int i=0;i<m_services->rowCount();++i)
    {
        auto service = qobject_cast<UpnpService*>(m_services->at(i));

        if (service->serviceType() == type)
            return service;
    }

    return Q_NULLPTR;
}

void UpnpDevice::itemAvailableChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}

void UpnpDevice::sendAlive()
{
    emit aliveMessage(uuid(), "");
    emit aliveMessage(uuid(), deviceType());

    for (int i=0;i<m_services->rowCount();++i)
    {
        auto service = qobject_cast<UpnpService*>(m_services->at(i));
        service->sendAlive(uuid());
    }

    for (int i=0;i<m_devices->rowCount();++i)
    {
        auto device = qobject_cast<UpnpDevice*>(m_devices->at(i));
        device->sendAlive();
    }
}


void UpnpDevice::sendByeBye()
{
    emit byebyeMessage(uuid(), "");
    emit byebyeMessage(uuid(), deviceType());

    for (int i=0;i<m_services->rowCount();++i)
    {
        auto service = qobject_cast<UpnpService*>(m_services->at(i));
        service->sendByeBye(uuid());
    }

    for (int i=0;i<m_devices->rowCount();++i)
    {
        auto device = qobject_cast<UpnpDevice*>(m_devices->at(i));
        device->sendByeBye();
    }
}

void UpnpDevice::searchForST(const QHostAddress &host, const quint16 &port, const QString &st)
{
    if (status() != Ready)
    {
        qWarning() << "cannot answer to discover request, device is not ready" << this << st;
    }
    else
    {
        if (st == "ssdp:all" || st == QString("uuid:%1").arg(uuid()))
            emit searchResponse(host, port, QString("uuid:%1").arg(uuid()), QString("uuid:%1").arg(uuid()));

        if (st == "ssdp:all" || st == deviceType())
            emit searchResponse(host, port, deviceType(), QString("uuid:%1::%2").arg(uuid(), deviceType()));

        for (int i=0;i<m_services->rowCount();++i)
        {
            auto service = qobject_cast<UpnpService*>(m_services->at(i));
            service->searchForST(host, port, st, uuid());
        }

        for (int i=0;i<m_devices->rowCount();++i)
        {
            auto device = qobject_cast<UpnpDevice*>(m_devices->at(i));
            device->searchForST(host, port, st);
        }
    }
}

void UpnpDevice::subscribeEventingSlot(const QNetworkRequest &request, const QString &serviceId)
{
    emit subscribeEventingSignal(request, uuid(), serviceId);
}

UpnpService *UpnpDevice::getService(const QString &serviceId)
{
    return qobject_cast<UpnpService*>(m_services->find(serviceId));;
}

void UpnpDevice::replyRequest(HttpRequest *request)
{
    for (int i=0;i<m_services->rowCount();++i)
    {
        auto service = qobject_cast<UpnpService*>(m_services->at(i));
        if (service->replyRequest(request))
            return;
    }


    qWarning() << this << "unknown request" << request->operationString() << request->url();
}
