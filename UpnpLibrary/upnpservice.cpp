#include "upnpservice.h"

UpnpService::UpnpService(QObject *parent) :
    UpnpObject(parent),
    m_info(),
    m_stateVariablesModel(new StateVariableItem)
{
    initRoles();
}

UpnpService::UpnpService(UpnpObject *upnpParent, QDomNode info, QObject *parent) :
    UpnpObject(Service, upnpParent, parent),
    m_info(info),
    m_stateVariablesModel(new StateVariableItem)
{
    initRoles();

    connect(this, SIGNAL(infoChanged()), this, SLOT(requestDescription()));
    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
    connect(this, SIGNAL(descriptionChanged()), this, SLOT(subscribeEventing()));
    connect(this, SIGNAL(descriptionChanged()), this, SIGNAL(itemChanged()));

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
        doc.setContent(reply->readAll(), true);
        setDescription(doc.documentElement());

        qDebug() << "description received" << this << reply->request().url();

        readActions();
        readStateVariables();

        setStatus(Ready);
    }
    else
    {
        qCritical() << reply->request().url() << reply->errorString();
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

    emit actionsModelChanged();
}

void UpnpService::readStateVariables()
{
    m_stateVariablesModel.clear();

    QDomNode root = description();
    if (root.nodeName() == "scpd")
    {
        QDomElement variableList = root.firstChildElement("serviceStateTable");
        if (!variableList.isNull())
        {
            QDomNodeList l_variables = variableList.elementsByTagName("stateVariable");
            for (int i=0;i<l_variables.size();++i)
            {
                QDomNode variable = l_variables.at(i);

                QString name = variable.firstChildElement("name").firstChild().nodeValue();

                if (name != "LastChange")
                {
                    StateVariableItem *item = new StateVariableItem(&m_stateVariablesModel);
                    item->setData(name, StateVariableItem::NameRole);
                    m_stateVariablesModel.appendRow(item);
                }
            }
        }
        else
        {
            qCritical() << "unable to find serviceStateTable element";
        }
    }
    else
    {
        qCritical() << host() << serviceType() << "unable to find scpd element";
    }

    emit stateVariablesModelChanged();
}

QStringList UpnpService::actionsModel() const
{
    return m_actionsModel;
}

ListModel *UpnpService::stateVariablesModel()
{
    return &m_stateVariablesModel;
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

//                qWarning() << name << in << out;

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
    if (description().isNull())
    {
        qCritical() << "cannot answer to discover request, device is not ready" << this << st;
    }
    else
    {
        if (st == "ssdp:all" || st == serviceType())
            emit searchResponse(serviceType(), QString("uuid:%1::%2").arg(uuid).arg(serviceType()));
    }
}

void UpnpService::subscribeEventing()
{
    QString streventUrl = getInfo("eventSubURL");
    if (!streventUrl.isEmpty())
    {
        QNetworkRequest request(urlFromRelativePath(streventUrl));
        request.setRawHeader("NT", "upnp:event");
        request.setRawHeader("TIMEOUT", "Second-300");

        emit subscribeEventingSignal(request, serviceId());
    }
}

void UpnpService::updateStateVariables(QHash<QString, QString> data)
{
    qDebug() << "update state variables" << host() << serviceType() << data;

    for (int i=0;i<m_stateVariablesModel.rowCount();++i)
    {
        StateVariableItem *item = qobject_cast<StateVariableItem*>(m_stateVariablesModel.at(i));
        if (item)
        {
            QString name = item->data(StateVariableItem::NameRole).toString();
            if (data.contains(name) && !data[name].isEmpty())
                item->setData(data[name], StateVariableItem::ValueRole);
        }
        else
        {
            qCritical() << "invalid state variable, index" << i;
        }
    }

    // check all variables have been updated
    foreach (const QString &name, data.keys())
    {
        if (name == "LastChange")
        {
            if (!data[name].isEmpty())
                updateLastChange(data[name]);
        }
        else
        {
            bool found = false;
            for (int i=0;i<m_stateVariablesModel.rowCount();++i)
            {
                StateVariableItem *item = qobject_cast<StateVariableItem*>(m_stateVariablesModel.at(i));
                if (item)
                {
                    if (item->data(StateVariableItem::NameRole) == name)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
                qCritical() << "state variable not found" << name;
        }
    }
}

void UpnpService::updateLastChange(QString data)
{
    qDebug() << "update LastChange data" << data;

    QDomDocument xml;
    xml.setContent(data, true);

    QDomElement event = xml.documentElement();
    if (event.localName() == "Event")
    {
        QDomNode instanceId = event.firstChild();
        if (instanceId.localName() == "InstanceID")
        {
            QDomAttr attrInstanceId = instanceId.toElement().attributeNode("val");
            if (!attrInstanceId.isNull())
            {
                QString valInstanceId = attrInstanceId.value();
                qDebug() << "instanceID" << valInstanceId;

                QHash<QString,QString> stateVariables;

                QDomNodeList child = instanceId.childNodes();
                for (int i=0;i<child.size();++i)
                {
                    QDomNode param = child.at(i);
                    QDomAttr attrValue = param.toElement().attributeNode("val");
                    if (!attrValue.isNull())
                    {
                        qDebug() << param.localName() << attrValue.value();
                        if (param.localName() != "LastChange")
                        {
                            stateVariables[param.localName()] = attrValue.value();
                        }
                        else
                        {
                            qCritical() << "invalid param name" << param.localName() << data;
                        }
                    }
                    else
                    {
                        qCritical() << "invalid xml LastChange (attribute value not found)" << param.localName() << data;
                    }
                }

                if (!stateVariables.isEmpty())
                    updateStateVariables(stateVariables);
            }
            else
            {
                qCritical() << "invalid xml LastChange (instanceID value not found)" << data;
            }
        }
        else
        {
            qCritical() << "invalid xml LastChange (instanceID not found)" << data;
        }
    }
    else
    {
        qCritical() << "invalid xml LastChange (event not found)" << data;
    }
}
