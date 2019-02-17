#include "upnpobject.h"

UpnpObject::UpnpObject(QObject *parent) :
    ListItem(parent),
    m_timeout(QDateTime::currentDateTime())
{

}

UpnpObject::UpnpObject(TypeObject type, UpnpObject *upnpParent, QObject *parent) :
    ListItem(parent),
    m_type(type),
    m_timeout(QDateTime::currentDateTime())
{
    setUpnpParent(upnpParent);

    connect(this, SIGNAL(descriptionChanged()), this, SLOT(parseObject()));
    connect(this, SIGNAL(descriptionChanged()), this, SIGNAL(itemChanged()));
    connect(this, SIGNAL(statusChanged()), this, SIGNAL(availableChanged()));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void UpnpObject::setRoles(const QHash<int, QByteArray> &roles)
{
    m_roles = roles;
}

UpnpObject *UpnpObject::upnpParent() const
{
    return m_upnpParent;
}

void UpnpObject::setUpnpParent(UpnpObject *parent)
{
    if (m_type == T_RootDevice && parent != Q_NULLPTR)
    {
        qCritical() << "RootDevice shall not have parent";
    }
    else if (m_type == T_Device && parent && parent->type() == T_Service)
    {
        qCritical() << "Device shall not have Service as parent";
    }
    else if (m_type == T_Service && parent && parent->type() == T_Service)
    {
        qCritical() << "Service shall not have Service as parent";
    }
    else
    {
        m_upnpParent = parent;
        emit upnpParentChanged();
    }
}

UpnpObject::TypeObject UpnpObject::type() const
{
    return m_type;
}

void UpnpObject::setType(const TypeObject &type)
{
    m_type = type;
}

UpnpObject::Status UpnpObject::status() const
{
    return m_status;
}

void UpnpObject::setStatus(const Status &status)
{
    if (status < m_status)
    {
        qCritical() << "cannot change status to " << status << ", status = " << m_status;
    }
    else if (status != m_status)
    {
        m_status = status;
        emit statusChanged();
    }
    else
    {
        qDebug() << "same status, not necessary to update anything" << m_status << status;
    }
}

QHash<int, QByteArray>  UpnpObject::roleNames() const
{
    return m_roles;
}

bool UpnpObject::setData(const QVariant &value, const int &role)
{
    qWarning() << "unable to set data" << value << role;
    return false;
}

void UpnpObject::update(const SsdpMessage &message)
{
    QString nts = message.getHeader("NTS");

    if (nts == "ssdp:byebye")
    {
        setAvailable(false);
    }
    else
    {
        int cacheControl = message.cacheControl();
        if (cacheControl > 0)
        {
            m_timeout = QDateTime::currentDateTime().addSecs(cacheControl);

            m_timer.start(message.cacheControl()*1000);

            setAvailable(true);
        }
        else
        {
            qCritical() << "UpnpRootDevice, invalid cache-control" << cacheControl;
        }
    }

}

QNetworkReply *UpnpObject::get(QNetworkRequest request)
{
    request.setRawHeader("Connection", "close");
    request.setRawHeader(QByteArray("HOST"), QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());

    return MyNetwork::manager().get(request);
}

QNetworkReply *UpnpObject::post(QNetworkRequest request, const QByteArray &data)
{
    request.setRawHeader("Connection", "close");

    request.setRawHeader(QByteArray("HOST"), QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());

    request.setRawHeader(QByteArray("USER-AGENT"), serverName().toUtf8());

    return MyNetwork::manager().post(request, data);
}

QHostAddress UpnpObject::host() const
{
    if (m_upnpParent)
        return m_upnpParent->host();

    return QHostAddress();
}

quint16 UpnpObject::port() const
{
    if (m_upnpParent)
        return m_upnpParent->port();

    return 0;
}

QString UpnpObject::serverName() const
{
    if (m_upnpParent)
        return m_upnpParent->serverName();

    return QString();
}

UpnpDescription *UpnpObject::description() const
{
    return m_description;
}

QString UpnpObject::strDescription() const
{
    if (m_description)
        return m_description->stringDescription();

    return QString();
}

QString UpnpObject::valueFromDescription(const QString &param) const
{
    if (m_description)
        return m_description->attribute(param);

    return QString();
}

bool UpnpObject::setDescription(UpnpDescription *descr)
{
    if (m_description && m_description->parent() == this)
        m_description->deleteLater();

    descr->setParent(this);
    m_description = descr;
    emit descriptionChanged();
    return m_description != Q_NULLPTR;
}

void UpnpObject::timeout()
{
    setAvailable(false);
}

bool UpnpObject::available() const
{
    return m_available && m_status == Ready;
}

void UpnpObject::setAvailable(bool flag)
{
    if (flag != m_available)
    {
        m_available = flag;
        emit availableChanged();
    }
}

QDateTime UpnpObject::timeoutDateTime() const
{
    return m_timeout;
}

QUrl UpnpObject::url() const
{
    if (m_upnpParent)
        return m_upnpParent->url();

    return QUrl();
}

QUrl UpnpObject::urlFromRelativePath(const QString &path) const
{
    return url().resolved(path);
}

QString UpnpObject::generateUuid()
{
    if (m_upnpParent)
        return m_upnpParent->generateUuid();

    qCritical() << "unable to generate uuid, no upnp parent defined";
    return QString();
}
