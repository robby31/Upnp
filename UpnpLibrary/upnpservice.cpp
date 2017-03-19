#include "upnpservice.h"

UpnpService::UpnpService(QObject *parent) :
    UpnpObject(parent),
    m_info()
{
    initRoles();
}

UpnpService::UpnpService(UpnpObject *upnpParent, QDomNode info, QObject *parent) :
    UpnpObject(Service, upnpParent, parent),
    m_info(info)
{
    initRoles();

    connect(this, SIGNAL(infoChanged()), this, SLOT(requestDescription()));
    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));

    emit infoChanged();
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
    {
        return serviceType();
    }
    case AvailableRole:
    {
        return available();
    }
    default:
    {
        return QVariant::Invalid;
    }
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

QUrl UpnpService::scpdUrl() const
{
    return QUrl(urlFromRelativePath(getInfo("SCPDURL")).url());
}

QUrl UpnpService::controlUrl() const
{
    return QUrl(urlFromRelativePath(getInfo("controlURL")).url());
}

QUrl UpnpService::eventUrl() const
{
    return QUrl(urlFromRelativePath(getInfo("eventURL")).url());
}

QString UpnpService::getInfo(const QString &param) const
{
    QDomElement elt = m_info.firstChildElement(param);
    return elt.firstChild().nodeValue();
}

void UpnpService::requestDescription()
{    
    QNetworkRequest request(scpdUrl());

    QNetworkReply *reply = get(request);
    if (reply == 0)
    {
        qCritical() << "Unable to get description" << this << serviceType() << scpdUrl();
        setStatus(Error);
    }
    else
    {
        connect(reply, SIGNAL(finished()), this, SLOT(descriptionReceived()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));
        setStatus(Loading);
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

        setStatus(Ready);
    }
    else
    {
        qCritical() << reply->errorString();
        setStatus(Error);
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

                if (in.isEmpty())
                {
                    SoapAction action(serviceType(), name);

                    QNetworkReply *reply = sendAction(action);
                    connect(reply, SIGNAL(finished()), this, SLOT(actionFinished()));
                    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));
                }
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

QNetworkReply *UpnpService::sendAction(const SoapAction &action)
{

    QNetworkRequest request;

    request.setUrl(controlUrl());

    request.setRawHeader(QByteArray("CONTENT-TYPE"), "text/xml; charset=\"utf-8\"");

    request.setRawHeader(QByteArray("SOAPACTION"), action.soapaction());

    return post(request, action.toByteArray());
}

void UpnpService::actionFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() == QNetworkReply::NoError)
    {
        qWarning() << "action done" << this << reply->request().rawHeader("SOAPACTION");
        qWarning() << reply->readAll();
    }
    else
    {
        qCritical() << reply->errorString();
    }

    reply->deleteLater();
}

void UpnpService::networkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    UpnpError upnpError(error, reply->readAll());

    qCritical() << "Network Error" << upnpError.netError() << reply->request().url() << upnpError.code() << upnpError.description();

    reply->deleteLater();
}

void UpnpService::sendAlive(const QString &uuid)
{
    emit aliveMessage(uuid, serviceType());
}

void UpnpService::sendByeBye(const QString &uuid)
{
    emit byebyeMessage(uuid, serviceType());
}

void UpnpService::searchForST(const QString &st, const QString &uuid)
{
    if (st == "ssdp:all" or st == serviceType())
        emit searchResponse(st, QString("uuid:%1::%2").arg(uuid).arg(st));
}
