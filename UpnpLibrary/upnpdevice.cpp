#include "upnpdevice.h"

UpnpDevice::UpnpDevice(QObject *parent) :
    UpnpObject(parent),
    m_info(),
    m_host()
{
    initRoles();
}

UpnpDevice::UpnpDevice(QHostAddress host, QDomNode info, QObject *parent) :
    UpnpObject(Device, host, parent),
    m_info(info),
    m_host(host)
{
    initRoles();
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
    return getInfo("deviceType");
}

QVariant UpnpDevice::data(int role) const
{
    switch (role) {
    case HostRole:
        return m_host.toString();
    case DeviceTypeRole:
        return getInfo("deviceType");
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
