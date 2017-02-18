#include "upnprootdevice.h"

UpnpRootDevice::UpnpRootDevice(QObject *parent) :
    UpnpDevice(parent),
    m_rootDescription(),
    m_iconUrl()
{
    setType(RootDevice);

    initRoles();   
}

UpnpRootDevice::UpnpRootDevice(QHostAddress host, QString uuid, QObject *parent) :
    UpnpDevice(host, uuid, parent),
    m_rootDescription(),
    m_iconUrl()
{
    setType(RootDevice);

    initRoles();

    connect(this, SIGNAL(rootDescriptionChanged()), this, SIGNAL(itemChanged()));
    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
}

void UpnpRootDevice::initRoles()
{
    QHash<int, QByteArray> roles;
    roles[HostRole] = "host";
    roles[UuidRole] = "uuid";
    roles[FriendlyNameRole] = "friendlyname";
    roles[IconUrlRole] = "iconurl";
    roles[AvailableRole] = "available";
    roles[DeviceTypeRole] = "devicetype";
    roles[PresentationUrlRole] = "presentationurl";
    roles[VersionRole] = "version";
    setRoles(roles);
}

QVariant UpnpRootDevice::data(int role) const
{
    switch (role) {
    case HostRole:
        return host().toString();
    case UuidRole:
        return uuid();
    case FriendlyNameRole:
        return friendlyName();
    case IconUrlRole:
        return m_iconUrl;
    case AvailableRole:
        return available();
    case DeviceTypeRole:
        return deviceType();
    case PresentationUrlRole:
        return valueFromDescription("presentationURL");
    case VersionRole:
        return version();
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString UpnpRootDevice::version() const
{
    if (!m_rootDescription.isNull())
    {
        QDomNode root = m_rootDescription.firstChildElement("root");
        if (!root.isNull())
        {
            QDomNode specVersion = root.firstChildElement("specVersion");
            if (!specVersion.isNull())
            {
                QString major = specVersion.firstChildElement("major").firstChild().nodeValue();
                QString minor = specVersion.firstChildElement("minor").firstChild().nodeValue();
                if (!major.isNull() && !minor.isNull())
                    return QString("%1.%2").arg(major).arg(minor);
                else
                    qCritical() << "unable to find major and minor" << major << minor;
            }
            else
            {
                qCritical() << "unable to find specVersion";
            }
        }
        else
        {
            qCritical() << "unable to find root";
        }
    }

    return QString();
}

void UpnpRootDevice::requestDescription(QString location)
{
    QNetworkReply *reply = get(location);
    if (reply == 0)
    {
        qCritical() << "Unable to get description" << this << location;
    }
    else
    {
        connect(reply, SIGNAL(finished()), this, SLOT(descriptionReceived()));
    }
}

void UpnpRootDevice::setRootDescription(QByteArray data)
{
    m_rootDescription.setContent(data);
    emit rootDescriptionChanged();
}

QString UpnpRootDevice::rootDescription() const
{
    return m_rootDescription.toString();
}

void UpnpRootDevice::descriptionReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    setUrl(reply->request().url());

    if (reply->error() == QNetworkReply::NoError)
    {
        setRootDescription(reply->readAll());

        qDebug() << "description received" << this << reply->request().url();

        QDomElement root = m_rootDescription.firstChildElement("root");
        if (root.isNull())
        {
            qCritical() << "root not found in description";
        }
        else
        {
            QDomElement device = root.firstChildElement("device");
            if (!device.isNull())
            {
                setDescription(device);
                readServices();
                readDevices();

                // check uuid
                QDomElement udnElt = device.firstChildElement("UDN");

                if (!udnElt.isNull())
                {
                    QString udn = udnElt.firstChild().nodeValue();

                    if (udn.startsWith("uuid:"))
                    {
                        if (udn.right(udn.size()-5) != uuid())
                            qCritical() << "invalid uuid in description" << uuid() << "!=" << udn.right(udn.size()-5);
                    }
                    else
                    {
                        qCritical() << "invalid uuid in description" << udn;
                    }
                }
                else
                {
                    qCritical() << "invalid uuid in description (no UDN found)";
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
}

void UpnpRootDevice::itemAvailableChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}

