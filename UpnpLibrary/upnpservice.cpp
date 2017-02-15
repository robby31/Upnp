#include "upnpservice.h"

UpnpService::UpnpService(QObject *parent) :
    UpnpObject(parent),
    m_info()
{
    initRoles();
}

UpnpService::UpnpService(QHostAddress host, QDomNode info, QObject *parent) :
    UpnpObject(Service, host, parent),
    m_info(info)
{
    initRoles();
}

void UpnpService::initRoles()
{
    QHash<int, QByteArray> roles;
    roles[ServiceTypeRole] = "servicetype";
    roles[AvailableRole] = "available";
    setRoles(roles);
}

QString UpnpService::id() const
{
    return getInfo("serviceType");
}

QVariant UpnpService::data(int role) const
{
    switch (role) {
    case ServiceTypeRole:
        return getInfo("serviceType");
    case AvailableRole:
        return available();
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString UpnpService::getInfo(const QString &param) const
{
    QDomElement elt = m_info.firstChildElement(param);
    return elt.firstChild().nodeValue();
}

void UpnpService::getDescription()
{
    QString p_url = urlFromRelativePath(getInfo("SCPDURL")).url();

    QNetworkReply *reply = get(p_url);
    if (reply == 0)
    {
        qCritical() << "Unable to get description" << this << getInfo("serviceType") << p_url;
    }
    else
    {
        connect(reply, SIGNAL(finished()), this, SLOT(descriptionReceived()));
    }
}

void UpnpService::descriptionReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() == QNetworkReply::NoError)
    {
        setDescription(reply->readAll());

        qDebug() << "description received" << this << reply->request().url();
    }
    else
    {
        qCritical() << reply->errorString();
    }

    reply->deleteLater();

    emit itemChanged();
}
