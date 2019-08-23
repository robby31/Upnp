#include "mediarenderer.h"

MediaRenderer::MediaRenderer(QObject *parent) :
    ListItem(parent),
    m_device(Q_NULLPTR),
    status("standby")
{
    m_roles[statusRole] = "status";
    m_roles[nameRole] = "name";
    m_roles[networkAddressRole] = "networkAddress";
    m_roles[iconUrlRole] = "iconurl";
    m_roles[availableRole] = "available";
    m_roles[sinkProtocolRole] = "sinkProtocol";
}

MediaRenderer::MediaRenderer(UpnpRootDevice *device, QObject *parent) :
    ListItem(parent),
    m_device(device),
    status("standby")
{
    auto service = qobject_cast<UpnpService*>(device->getService("urn:upnp-org:serviceId:ConnectionManager"));
    if (service)
    {
        connect(service, SIGNAL(statusChanged()), this, SLOT(serviceStatusChanged()));

        ListModel * varModel = service->stateVariablesModel();
        if (varModel)
            connect(varModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(stateVarChanged(QModelIndex,QModelIndex,QVector<int>)));
    }

    m_roles[statusRole] = "status";
    m_roles[nameRole] = "name";
    m_roles[networkAddressRole] = "networkAddress";
    m_roles[iconUrlRole] = "iconurl";
    m_roles[availableRole] = "available";

    connect(m_device, SIGNAL(itemChanged(QVector<int>)), this, SLOT(deviceItemChanged(QVector<int>)));
    connect(m_device, SIGNAL(destroyed(QObject*)), this, SLOT(deviceDestroyed(QObject*)));
}

MediaRenderer::MediaRenderer(UpnpRootDevice *device, const QString &profilesPath, QObject *parent):
    ListItem(parent),
    m_device(device),
    status("standby")
{
    auto service = qobject_cast<UpnpService*>(device->getService("urn:upnp-org:serviceId:ConnectionManager"));
    if (service)
    {
        connect(service, SIGNAL(statusChanged()), this, SLOT(serviceStatusChanged()));

        ListModel * varModel = service->stateVariablesModel();
        if (varModel)
            connect(varModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(stateVarChanged(QModelIndex,QModelIndex,QVector<int>)));
    }

    m_roles[statusRole] = "status";
    m_roles[nameRole] = "name";
    m_roles[networkAddressRole] = "networkAddress";
    m_roles[iconUrlRole] = "iconurl";
    m_roles[availableRole] = "available";

    connect(m_device, SIGNAL(itemChanged(QVector<int>)), this, SLOT(deviceItemChanged(QVector<int>)));
    connect(m_device, SIGNAL(destroyed(QObject*)), this, SLOT(deviceDestroyed(QObject*)));

    m_dlnaProfiles = new Protocol(profilesPath, this);
}

QHash<int, QByteArray> MediaRenderer::roleNames() const
{
    return m_roles;
}

QString MediaRenderer::id() const
{
    if (m_device)
        return m_device->uuid();

    return ListItem::id();
}

QVariant MediaRenderer::data(int role) const
{
    switch (role) {
    case statusRole:
    {
        return status;
    }
    case nameRole:
    {
        if (m_device)
            return m_device->friendlyName();

        return QString();
    }
    case networkAddressRole:
    {
        return netWorkAddress();
    }
    case iconUrlRole:
    {
        if (m_device)
            return m_device->iconUrl();

        return QString();
    }
    case availableRole:
    {
        if (m_device)
            return m_device->available();

        return false;
    }

    case sinkProtocolRole:
    {
        return sinkProtocols();
    }

    default:
    {
        return QVariant::Invalid;
    }
    }
}

bool MediaRenderer::setData(const QVariant &value, const int &role)
{
    QVector<int> roles;
    roles << role;

    switch(role)
    {
    case statusRole:
    {
        if (value != status)
        {
            status = value.toString();
            emit itemChanged(roles);
        }
        return true;
    }

    default:
    {
        qWarning() << "unable to set data" << value << role;
        return false;
    }
    }
}

void MediaRenderer::deviceItemChanged(const QVector<int> &roles)
{
    emit itemChanged();

    if (roles.contains(UpnpRootDevice::AvailableRole) && !m_device->available())
        emit removeRenderer();
}

QString MediaRenderer::netWorkAddress() const
{
    if (m_device)
        return m_device->host().toString();

    return QString();
}

void MediaRenderer::deviceDestroyed(QObject *obj)
{
    if (obj == m_device)
        m_device = Q_NULLPTR;
}

void MediaRenderer::serviceStatusChanged()
{
    auto service = qobject_cast<UpnpService*>(sender());
    if (service->status() == UpnpService::Ready)
        service->subscribeEventing();
}

void MediaRenderer::stateVarChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)

    auto model = qobject_cast<ListModel*>(sender());

    auto item = qobject_cast<StateVariableItem*>(model->at(topLeft.row()));
    if (item)
    {
        QString name = item->data(StateVariableItem::NameRole).toString();
        QString value = item->data(StateVariableItem::ValueRole).toString();

        if (name == "SinkProtocolInfo")
        {
            m_sinkProtocol.clear();

            QStringList l_protocol = value.split(",");
            foreach (QString protocol, l_protocol)
                m_sinkProtocol << protocol;

            m_dlnaProfiles->setProtocols(m_sinkProtocol);

            QVector<int> roles;
            roles << sinkProtocolRole;
            emit itemChanged(roles);
        }
    }
}

QStringList MediaRenderer::sinkProtocols() const
{
    return m_sinkProtocol;
}

Protocol *MediaRenderer::dlnaProfiles() const
{
    return m_dlnaProfiles;
}
