#include "upnpservice.h"

UpnpService::UpnpService(QObject *parent) :
    UpnpObject(T_Service, Q_NULLPTR, parent),
    m_stateVariablesModel(new StateVariableItem)
{
    initRoles();

    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
}

UpnpService::UpnpService(UpnpObject *upnpParent, QObject *parent) :
    UpnpObject(T_Service, upnpParent, parent),
    m_stateVariablesModel(new StateVariableItem)
{
    initRoles();

    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
}

UpnpService::UpnpService(UpnpObject *upnpParent, const QDomNode &info, QObject *parent) :
    UpnpObject(T_Service, upnpParent, parent),
    m_info(info),
    m_stateVariablesModel(new StateVariableItem)
{
    initRoles();

    connect(this, SIGNAL(availableChanged()), this, SLOT(itemAvailableChanged()));
}

bool UpnpService::setInfo(const QDomNode &info)
{
    m_info = info;
    return true;
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

QUrl UpnpService::eventSubUrl() const
{
    return QUrl(urlFromRelativePath(getInfo("eventSubURL")).url());
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
    if (!reply)
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
    auto reply = qobject_cast<QNetworkReply*>(sender());

    if (reply->error() == QNetworkReply::NoError)
    {
        auto description = new UpnpServiceDescription(this);
        description->setContent(reply->readAll());
        setDescription(description);

#if !defined(QT_NO_DEBUG_OUTPUT)
        qDebug() << "description received" << this << reply->request().url();
#endif
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

    auto descr = qobject_cast<UpnpServiceDescription*>(description());
    if (descr)
        m_actionsModel += descr->actionsName();

    emit actionsModelChanged();
}

void UpnpService::readStateVariables()
{
    m_stateVariablesModel.clear();

    auto descr = qobject_cast<UpnpServiceDescription*>(description());
    if (descr)
    {
        QDomNodeList l_variables = descr->stateVariables().elementsByTagName("stateVariable");
        for (int i=0;i<l_variables.size();++i)
        {
            QDomNode variable = l_variables.at(i);
            QString name = variable.firstChildElement("name").firstChild().nodeValue();
            QDomNamedNodeMap attr = variable.attributes();

            auto item = new StateVariableItem(&m_stateVariablesModel);
            item->setData(name, StateVariableItem::NameRole);
            if (!attr.namedItem("sendEvents").isNull() && attr.namedItem("sendEvents").nodeValue() == "yes")
                item->setData(true, StateVariableItem::SendEventsRole);
            if (!attr.namedItem("multicast").isNull() && attr.namedItem("multicast").nodeValue() == "yes")
                item->setData(true, StateVariableItem::MulticastRole);
            if (!variable.firstChildElement("defaultValue").isNull())
            {
                item->setData(variable.firstChildElement("defaultValue").firstChild().nodeValue(), StateVariableItem::DefaultValueRole);
                item->setData(variable.firstChildElement("defaultValue").firstChild().nodeValue(), StateVariableItem::ValueRole);
            }
            m_stateVariablesModel.appendRow(item);
        }
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

QDomNode UpnpService::getAction(const QString &actionName)
{
    if (description())
    {
        QDomElement root = description()->xmlDescription();
        if (root.nodeName() == "scpd")
        {
            QDomElement actionList = root.firstChildElement("actionList");
            if (!actionList.isNull())
            {
                QDomNodeList l_action = actionList.elementsByTagName("action");
                for (int index=0;index<l_action.size();++index)
                {
                    QDomNode action = l_action.at(index);
                    QString name = action.firstChildElement("name").firstChild().nodeValue();
                    if (name == actionName)
                        return action;
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

    return QDomNode();
}

UpnpActionReply *UpnpService::runAction(const SoapAction &action)
{
    QNetworkReply *reply = sendAction(action);
    auto actionReply = new UpnpActionReply(reply);
    return actionReply;
}

UpnpActionReply *UpnpService::runAction(const QString &actionName, QVariantMap args)
{
    QDomNode actionDefinition = getAction(actionName);

    if (!actionDefinition.isNull())
    {
        SoapAction action(serviceType(), actionName);

        bool error = false;

        QDomElement argumentList = actionDefinition.firstChildElement("argumentList");
        if (!argumentList.isNull())
        {
            QDomNodeList l_arguments = argumentList.elementsByTagName("argument");
            for (int i=0;i<l_arguments.size();++i)
            {
                QDomNode argument = l_arguments.at(i);

                QString argName = argument.firstChildElement("name").firstChild().nodeValue();
                QString direction = argument.firstChildElement("direction").firstChild().nodeValue();

                if (direction == "in")
                {
                    if (args.contains(argName))
                    {
                        action.addArgument(argName, args[argName].toString());
                    }
                    else
                    {
                        qCritical() << "missing argument" << argName << "in action" << actionName;
                        error = true;
                    }
                }
            }
        }

        if (!error)
            return runAction(action);

        qCritical() << "unable to run action" << actionName;
    }
    else
    {
        qCritical() << "invalid action" << actionName;
    }

    return Q_NULLPTR;
}

UpnpActionReply *UpnpService::runAction(const int &index)
{
    QString name;
    QStringList in;
    QStringList out;

    if (description())
    {
        QDomElement root = description()->xmlDescription();
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
                        return runAction(action);
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

    return Q_NULLPTR;
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

void UpnpService::networkError(QNetworkReply::NetworkError error)
{
    auto reply = qobject_cast<QNetworkReply*>(sender());

    UpnpError upnpError(error, reply->readAll());
    emit errorOccured(upnpError);

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

void UpnpService::searchForST(const QHostAddress &host, const quint16 &port, const QString &st, const QString &uuid)
{
    if (status() != Ready)
    {
        qCritical() << "cannot answer to discover request, service is not ready" << this << st;
    }
    else
    {
        if (st == "ssdp:all" || st == serviceType())
            emit searchResponse(host, port, serviceType(), QString("uuid:%1::%2").arg(uuid, serviceType()));
    }
}

void UpnpService::subscribeEventing()
{
    if (status() == Ready)
    {
        QString streventSubUrl = getInfo("eventSubURL");
        if (!streventSubUrl.isEmpty())
        {
            QNetworkRequest request(urlFromRelativePath(streventSubUrl));
            request.setRawHeader("NT", "upnp:event");
            request.setRawHeader("TIMEOUT", "Second-300");

            emit subscribeEventingSignal(request, serviceId());
        }
    }
    else
    {
        qCritical() << "unable to subscibe event, service not ready";
    }
}

StateVariableItem *UpnpService::findStateVariableByName(const QString &name)
{
    for (int i=0;i<m_stateVariablesModel.rowCount();++i)
    {
        auto item = qobject_cast<StateVariableItem*>(m_stateVariablesModel.at(i));
        if (item)
        {
            QString var_name = item->data(StateVariableItem::NameRole).toString();
            if (var_name == name)
                return item;
        }
    }

    return Q_NULLPTR;
}

void UpnpService::updateStateVariable(const QString &name, const QString &value)
{
    StateVariableItem *item = findStateVariableByName(name);

    if (item)
        item->setData(value, StateVariableItem::ValueRole);
    else
        qCritical() << "unable to update state variable" << name << "(variable not found).";
}

void UpnpService::updateStateVariables(QHash<QString, QString> data)
{
#if !defined(QT_NO_DEBUG_OUTPUT)
    qDebug() << "update state variables" << host() << serviceType() << data;
#endif

    for (int i=0;i<m_stateVariablesModel.rowCount();++i)
    {
        auto item = qobject_cast<StateVariableItem*>(m_stateVariablesModel.at(i));
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
    for (auto it = data.constBegin(); it != data.constEnd(); ++it)
    {
        if (it.key() == "LastChange")
        {
            if (!it.value().isEmpty())
                updateLastChange(it.value());
        }
        else
        {
            bool found = false;
            for (int i=0;i<m_stateVariablesModel.rowCount();++i)
            {
                auto item = qobject_cast<StateVariableItem*>(m_stateVariablesModel.at(i));
                if (item)
                {
                    if (item->data(StateVariableItem::NameRole) == it.key())
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
                qCritical() << "state variable not found" << it.key();
        }
    }
}

void UpnpService::updateLastChange(const QString &data)
{
#if !defined(QT_NO_DEBUG_OUTPUT)
    qDebug() << "update LastChange data" << data;
#endif

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
#if !defined(QT_NO_DEBUG_OUTPUT)
                qDebug() << "instanceID" << valInstanceId;
#endif

                QHash<QString,QString> stateVariables;

                QDomNodeList child = instanceId.childNodes();
                for (int i=0;i<child.size();++i)
                {
                    QDomNode param = child.at(i);
                    QDomAttr attrValue = param.toElement().attributeNode("val");
                    if (!attrValue.isNull())
                    {
#if !defined(QT_NO_DEBUG_OUTPUT)
                        qDebug() << param.localName() << attrValue.value();
#endif
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

void UpnpService::parseObject()
{
    readActions();

    readStateVariables();

    if (status() != Error)
    {
        setStatus(Ready);
    }
}

bool UpnpService::replyRequest(HttpRequest *request)
{
    QUrl requestUrl = urlFromRelativePath(request->url().toString());

    if ((request->operation() == QNetworkAccessManager::GetOperation or request->operation() == QNetworkAccessManager::HeadOperation) && scpdUrl() == requestUrl)
    {
        // returns description of service
        request->replyData(description()->stringDescription().toUtf8());
        return true;
    }

    if (request->operationString() == "SUBSCRIBE" && requestUrl == eventSubUrl() && !request->header("NT").isEmpty())
    {
        replyNewSubscription(request);
        return true;
    }

    if (request->operationString() == "SUBSCRIBE" && requestUrl == eventSubUrl() && !request->header("SID").isEmpty())
    {
        replyRenewSubscription(request);
        return true;
    }

    if (request->operation() == QNetworkAccessManager::PostOperation && requestUrl == controlUrl())
    {
        SoapAction action(request->requestData());

        QString soapaction = request->header("SOAPACTION");

        if (!action.isValid() || soapaction != action.soapaction())
        {
            qCritical() << "invalid action" << soapaction << action.soapaction();
            replyUpnpError(request, UpnpError::INVALID_ACTION);
        }
        else
        {
            replyAction(request, action);
        }

        return true;
    }

    return false;
}

bool UpnpService::replyNewSubscription(HttpRequest *request)
{
    if (request->header("NT") == "upnp:event")
    {
        QStringList l_url;
        QRegularExpression callback("<(.+)>");
        QRegularExpressionMatchIterator i = callback.globalMatch(request->header("CALLBACK"));
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            if (match.hasMatch())
                l_url << match.captured(1);
        }

        if (l_url.isEmpty())
        {
            replyUpnpError(request, UpnpError::PRECONDITIN_FAILED);
        }
        else
        {
            QStringList header;

            header << QString("DATE: %1").arg(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
            if (!request->serverName().isEmpty())
                header << QString("SERVER: %1").arg(request->serverName());

            QString uuid = generateUuid();
            int timeOut = 1800;

            if (m_subscription.contains(uuid))
                qCritical() << "invalid subscription uuid" << uuid << m_subscription.keys();

            header << QString("SID: uuid:%1").arg(uuid);
            header << QString("Content-Length: 0");
            header << QString("TIMEOUT: Second-%1").arg(timeOut);

            if (!request->sendHeader(header))
            {
                request->setError(QString("unable to send header to client"));
            }
            else
            {                
                m_subscription[uuid].urls = l_url;
                m_subscription[uuid].eventKey = 0;
                m_subscription[uuid].timeOut = timeOut;
                m_subscription[uuid].timeOver = QDateTime::currentDateTime().addSecs(timeOut);

                if (m_timerCheckSubscription <= 0)
                    startCheckSubscription();

                // send events in 1 second
                int timerId = startTimer(1000);
                if (timerId > 0)
                {
                    m_sendEventTimer[timerId] = uuid;
                }
                else
                {
                    qCritical() << "unable to start timer to send events" << uuid;
                    sendEvent(uuid);
                }
            }
        }
    }
    else
    {
        // param NT in header is invalid
        replyUpnpError(request, UpnpError::PRECONDITIN_FAILED);
    }

    return true;
}

bool UpnpService::replyRenewSubscription(HttpRequest *request)
{
    QRegularExpression pattern("uuid:\\s*(.+)");
    QRegularExpressionMatch match = pattern.match(request->header("SID"));
    if (match.hasMatch())
    {
        QString sid = match.captured(1);
        if (m_subscription.contains(sid))
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "RENEW SUBSCRIPTION" << sid;
#endif

            int timeOut = 1800;
            QStringList header;

            header << QString("DATE: %1").arg(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
            if (!request->serverName().isEmpty())
                header << QString("SERVER: %1").arg(request->serverName());

            header << QString("SID: uuid:%1").arg(sid);
            header << QString("Content-Length: 0");
            header << QString("TIMEOUT: Second-%1").arg(timeOut);

            if (!request->sendHeader(header))
            {
                request->setError(QString("RENEW: unable to send header to client"));
            }
            else
            {
#if !defined(QT_NO_DEBUG_OUTPUT)
                qDebug() << "RENEW EVENT" << QDateTime::currentDateTime().secsTo(m_subscription[sid].timeOver);
#endif
                m_subscription[sid].timeOver = QDateTime::currentDateTime().addSecs(timeOut);
            }
        }
        else
        {
            qCritical() << "Invalid subscription renewal" << sid;
            replyUpnpError(request, UpnpError::PRECONDITIN_FAILED);
        }
    }
    else
    {
        qCritical() << "invalid SID" << request->header("SID") << "in SUBSCRIPTION renewal";
        replyUpnpError(request, UpnpError::BAD_REQUEST);
    }

    return true;
}

void UpnpService::sendEvent(const QString &uuid)
{
    if (!m_subscription[uuid].urls.isEmpty())
    {
        QNetworkRequest request(QUrl(m_subscription[uuid].urls.at(0)));
        request.setRawHeader("HOST", QString("%1:%2").arg(request.url().host()).arg(request.url().port(80)).toUtf8());
        request.setRawHeader("CONTENT-TYPE", "text/xml; charset=\"utf-8\"");
        request.setRawHeader("NT", "upnp:event");
        request.setRawHeader("NTS", "upnp:propchange");
        request.setRawHeader("SID", QString("uuid:%1").arg(uuid).toUtf8());
        request.setRawHeader("SEQ", QVariant::fromValue(m_subscription[uuid].eventKey++).toString().toUtf8());
        request.setRawHeader("CONNECTION", "close");

        XmlEvent event;

        for (int index=0;index<m_stateVariablesModel.rowCount();++index)
        {
            auto item = qobject_cast<StateVariableItem*>(m_stateVariablesModel.at(index));
            bool sendEvents = item->data(StateVariableItem::SendEventsRole).toBool();
            if (sendEvents)
            {
                QString name = item->data(StateVariableItem::NameRole).toString();
                QString value = item->data(StateVariableItem::ValueRole).toString();

                // state variable shall be sent by eventing
                event.addProperty(name, value);
            }
        }

        QNetworkReply *reply = MyNetwork::manager().sendCustomRequest(request, "NOTIFY", event.toString().toUtf8());
        connect(reply, SIGNAL(finished()), this, SLOT(sendEventReply()));

        // check answer is received (30 seconds timeout)
        int timerId = startTimer(30000);
        m_checkSendEvent[timerId].reply = reply;
    }
}

void UpnpService::sendEventReply()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());

#if !defined(QT_NO_DEBUG_OUTPUT)
    qDebug() << "reply received" << reply;
#endif

    if (reply)
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qCritical() << "send event reply" << reply << "from" << reply->request().url();
            qCritical() << "answer data" << reply->errorString() << reply->rawHeaderList() << reply->readAll();
        }

        auto it = m_checkSendEvent.begin();
        while (it != m_checkSendEvent.end())
        {
            if (it.value().reply == reply)
            {
#if !defined(QT_NO_DEBUG_OUTPUT)
                qDebug() << "kill timer timeout" << it.key();
#endif
                killTimer(it.key());
                it = m_checkSendEvent.erase(it);
                break;
            }

            ++it;
        }

        reply->deleteLater();
    }
}

bool UpnpService::replyAction(HttpRequest *request, const SoapAction &action)
{
    Q_UNUSED(request)
    Q_UNUSED(action)
    qCritical() << "function replyAction shall be defined by class";
    return false;
}

void UpnpService::timerEvent(QTimerEvent *event)
{
    if (m_sendEventTimer.contains(event->timerId()))
    {
        sendEvent(m_sendEventTimer[event->timerId()]);
        killTimer(event->timerId());
        m_sendEventTimer.remove(event->timerId());
    }
    else if (m_timerCheckSubscription == event->timerId())
    {
        // check event subscribed are still valid
        auto it = m_subscription.begin();
        while (it != m_subscription.end())
        {
            if (QDateTime::currentDateTime().secsTo(it.value().timeOver) < 1)
            {
                qWarning() << it.key() << it.value().timeOver << "subscription over";
                it = m_subscription.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (m_subscription.isEmpty())
            stopCheckSubscription();
    }
    else if (m_checkSendEvent.contains(event->timerId()))
    {
        qWarning() << "SEND EVENT TIMEOUT" << event->timerId();

        m_checkSendEvent[event->timerId()].reply->deleteLater();

        killTimer(event->timerId());
        m_checkSendEvent.remove(event->timerId());
    }
    else
    {
        qCritical() << "invalid timer" << event->timerId();
        killTimer(event->timerId());
    }
}

void UpnpService::startCheckSubscription()
{
    if (m_timerCheckSubscription > 0)
    {
        qCritical() << "check subscription already started in service" << id();
    }
    else
    {
        m_timerCheckSubscription = startTimer(60000);
    }
}

void UpnpService::stopCheckSubscription()
{
    if (m_timerCheckSubscription > 0)
    {
        killTimer(m_timerCheckSubscription);
        m_timerCheckSubscription = -1;
    }
}
