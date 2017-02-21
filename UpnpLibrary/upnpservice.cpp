#include "upnpservice.h"

UpnpService::UpnpService(QObject *parent) :
    UpnpObject(parent),
    m_info()
{
    initRoles();
}

UpnpService::UpnpService(QDomNode info, QObject *parent) :
    UpnpObject(Service, parent),
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
    QNetworkReply *reply = get(scpdUrl().url());
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
                    QNetworkReply *reply = sendAction(name);
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

QNetworkReply *UpnpService::sendAction(const QString &action)
{

    QNetworkRequest request;

    request.setUrl(controlUrl());
    request.setRawHeader(QByteArray("HOST"), QString("%1:%2").arg(request.url().host()).arg(request.url().port()).toUtf8());
    request.setRawHeader(QByteArray("CONTENT-TYPE"), "text/xml; charset=\"utf-8\"");
    request.setRawHeader(QByteArray("USER-AGENT"), serverName().toUtf8());
    request.setRawHeader(QByteArray("SOAPACTION"), QString("%1#%2").arg(serviceType()).arg(action).toUtf8());

    QDomDocument xml;
    xml.appendChild(xml.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));

    QDomElement envelope = xml.createElementNS("http://schemas.xmlsoap.org/soap/envelope/", "s:Envelope");
    envelope.setAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
    xml.appendChild(envelope);

    QDomElement body = xml.createElement("s:Body");
    envelope.appendChild(body);

    QDomElement xmlAction = xml.createElementNS(serviceType(), "u:"+action);
    body.appendChild(xmlAction);

    return post(request, xml.toByteArray());
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

    qCritical() << "Network Error" << error << reply->request().url();

    reply->deleteLater();
}
