#include "devicesmodel.h"

DevicesModel::DevicesModel(QObject *parent) :
    ListModel(parent)
{

}

DevicesModel::DevicesModel(ListItem *prototype, QObject *parent) :
    ListModel(prototype, parent)
{

}

DevicesModel::~DevicesModel()
{
}

UpnpObject *DevicesModel::getUpnpObjectFromUSN(const QString &usn)
{
    for (int i=0;i<rowCount();++i)
    {
        UpnpObject *object = qobject_cast<UpnpRootDevice*>(at(i))->getUpnpObjectFromUSN(usn);
        if (object != 0)
            return object;
    }

    return 0;
}

UpnpService *DevicesModel::getService(const QString &deviceUuid, const QString &serviceId)
{
    UpnpDevice *device = qobject_cast<UpnpDevice*>(getUpnpObjectFromUSN(QString("uuid:%1").arg(deviceUuid)));
    if (device)
        return device->getService(serviceId);
    else
        return Q_NULLPTR;
}

void DevicesModel::addRootDevice(SsdpMessage message)
{
    // read uuid
    QString uuid = message.getUuidFromUsn();

    if (!uuid.isEmpty())
    {
        UpnpRootDevice *device = getRootDeviceFromUuid(uuid);

        if (device == 0)
        {
            device = new UpnpRootDevice(m_nam, m_macAddress, uuid, this);
            connect(device, &UpnpRootDevice::availableChanged, this, &DevicesModel::_rootDeviceAvailableChanged);
            connect(device, &UpnpRootDevice::statusChanged, this, &DevicesModel::_rootDeviceStatusChanged);
            connect(device, &UpnpRootDevice::subscribeEventingSignal, this, &DevicesModel::subscribeEventing);
            device->setServerName(message.getHeader("SERVER"));
            device->update(message);
            device->setUrl(message.getHeader("LOCATION"));
            device->requestDescription();
            appendRow(device);
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

UpnpRootDevice *DevicesModel::getRootDeviceFromUuid(const QString &uuid)
{
    if (!uuid.isEmpty())
    {
        UpnpRootDevice *device = qobject_cast<UpnpRootDevice*>(find(uuid));
        return device;
    }

    return 0;
}

void DevicesModel::ssdpMessageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message)
{
    if (message.format() == NOTIFY)
    {
        QString nts = message.getHeader("NTS");
        QString nt = message.getHeader("NT");

        if (nts == SsdpMessage::ALIVE)
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
                            qCritical() << "unable to find" << host << port << nt;
                    }
                }
                else
                {
                    qCritical() << "unable to find device" << host << port << uuid;
                }
            }
        }
        else if (nts == SsdpMessage::BYEBYE)
        {
            if (nt == UpnpRootDevice::UPNP_ROOTDEVICE)
            {
                UpnpRootDevice *device = getRootDeviceFromUuid(message.getUuidFromUsn());
                if (device != 0)
                {
                    device->setAvailable(false);
                    emit removeSidEventFromUuid(device->uuid());
                }
            }
            else
            {
                UpnpObject *object = getUpnpObjectFromUSN(message.getHeader("USN"));
                if (object != 0)
                {
                    if (object->type() == UpnpObject::T_Device)
                    {
                        UpnpDevice *device = (UpnpDevice*) object;
                        device->setAvailable(false);
                        emit removeSidEventFromUuid(device->uuid());
                    }
                    else
                    {
                        object->setAvailable(false);
                        qWarning() << "object BYEBYE" << object;
                    }
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
}

void DevicesModel::setNetworkManager(QNetworkAccessManager *nam)
{
    m_nam = nam;
}

void DevicesModel::setMacAddress(const QString &m_address)
{
    m_macAddress = m_address;
}

void DevicesModel::_rootDeviceAvailableChanged()
{
    UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(sender());

    if (root->available() == false)
    {
        QModelIndex index = indexFromItem(root);
        if (index.isValid())
            removeRow(index.row());
    }

}

void DevicesModel::_rootDeviceStatusChanged()
{
    UpnpRootDevice *root = qobject_cast<UpnpRootDevice*>(sender());

    if (root->status() == UpnpObject::Ready)
        emit newRootDevice(root);
}
