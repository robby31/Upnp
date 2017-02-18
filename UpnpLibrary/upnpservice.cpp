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

    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
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
    return serviceId();
}

QVariant UpnpService::data(int role) const
{
    switch (role) {
    case ServiceTypeRole:
        return serviceType();
    case AvailableRole:
        return available();
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

QString UpnpService::serviceType() const
{
    return getInfo("serviceType");
}

QString UpnpService::serviceId() const
{
    return getInfo("serviceId");
}

QString UpnpService::getInfo(const QString &param) const
{
    QDomElement elt = m_info.firstChildElement(param);
    return elt.firstChild().nodeValue();
}

void UpnpService::requestDescription()
{
    QString p_url = urlFromRelativePath(getInfo("SCPDURL")).url();

    QNetworkReply *reply = get(p_url);
    if (reply == 0)
    {
        qCritical() << "Unable to get description" << this << serviceType() << p_url;
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
        QDomDocument doc;
        doc.setContent(reply->readAll());
        setDescription(doc.documentElement());

        qDebug() << "description received" << this << reply->request().url();

        readActions();
    }
    else
    {
        qCritical() << reply->errorString();
    }

    reply->deleteLater();
}

void UpnpService::readActions()
{
    m_actionsModel.clear();

    QDomNode root = description();
    if (root.nodeName() == "scpd")
    {
        QDomElement actionList = root.firstChildElement("actionList");
        if (!actionList.isNull())
        {
            QDomNodeList l_action = actionList.elementsByTagName("action");
            for (int i=0;i<l_action.size();++i)
            {
                QDomNode action = l_action.at(i);
                m_actionsModel << action.firstChildElement("name").firstChild().nodeValue();
            }
        }
        else
        {
            qCritical() << "unable to find actionList element";
        }
    }
    else
    {
        qCritical() << host() << serviceType() << "unable to find scpd element";
    }
}

QStringList UpnpService::actionsModel() const
{
    return m_actionsModel;
}

void UpnpService::runAction(const int &index)
{
    QString name;
    QStringList in;
    QStringList out;

    QDomNode root = description();
    if (root.nodeName() == "scpd")
    {
        QDomElement actionList = root.firstChildElement("actionList");
        if (!actionList.isNull())
        {
            QDomNodeList l_action = actionList.elementsByTagName("action");
            if (index < l_action.size())
            {
                QDomNode action = l_action.at(index);

                name = action.firstChildElement("name").firstChild().nodeValue();

                QDomElement argumentList = action.firstChildElement("argumentList");
                if (!argumentList.isNull())
                {
                    QDomNodeList l_arguments = argumentList.elementsByTagName("argument");
                    for (int i=0;i<l_arguments.size();++i)
                    {
                        QDomNode argument = l_arguments.at(i);

                        QString argName = argument.firstChildElement("name").firstChild().nodeValue();
                        QString direction = argument.firstChildElement("direction").firstChild().nodeValue();

                        if (direction == "in")
                            in << argName;
                        else if (direction == "out")
                            out << argName;
                        else
                            qCritical() << "invalid direction" << direction;
                    }
                }

                qWarning() << name << in << out;
            }
        }
        else
        {
            qCritical() << "unable to find actionList element";
        }
    }
    else
    {
        qCritical() << host() << serviceType() << "unable to find scpd element";
    }
}

void UpnpService::itemAvailableChanged()
{
    QVector<int> roles;
    roles << AvailableRole;
    emit itemChanged(roles);
}
