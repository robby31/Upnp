#include "serviceconnectionmanager.h"

ServiceConnectionManager::ServiceConnectionManager(QObject *parent) :
    AbstractService(parent)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();
}

ServiceConnectionManager::ServiceConnectionManager(UpnpObject *upnpParent, QObject *parent) :
    AbstractService(upnpParent, parent)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();
}


ServiceConnectionManager::ServiceConnectionManager(UpnpObject *upnpParent, QDomNode info, QObject *parent) :
    AbstractService(upnpParent, info, parent)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();
}

void ServiceConnectionManager::initDescription()
{
    UpnpServiceDescription *serviceDescription = new UpnpServiceDescription();
    serviceDescription->setServiceAttribute("serviceType", "urn:schemas-upnp-org:service:ConnectionManager:1");
    serviceDescription->setServiceAttribute("serviceId", "urn:upnp-org:serviceId:ConnectionManager");
    serviceDescription->setServiceAttribute("SCPDURL", "/UPnP_AV_ConnectionManager_1.0.xml");
    serviceDescription->setServiceAttribute("controlURL", "/upnp/control/connection_manager");
    serviceDescription->setServiceAttribute("eventSubURL", "/upnp/event/connection_manager");

    setInfo(serviceDescription->xmlInfo());

    setDescription(serviceDescription);
}

void ServiceConnectionManager::initActions()
{
    UpnpServiceDescription *serviceDescription = description();

    if (description())
    {
        QDomElement action = serviceDescription->addAction("GetProtocolInfo");
        serviceDescription->addArgument(action, "Source", UpnpServiceDescription::OUT, "SourceProtocolInfo");
        serviceDescription->addArgument(action, "Sink", UpnpServiceDescription::OUT, "SinkProtocolInfo");

        action = serviceDescription->addAction("GetCurrentConnectionIDs");
        serviceDescription->addArgument(action, "ConnectionIDs", UpnpServiceDescription::OUT, "CurrentConnectionIDs");

        action = serviceDescription->addAction("GetCurrentConnectionInfo");
        serviceDescription->addArgument(action, "ConnectionID", UpnpServiceDescription::IN, "A_ARG_TYPE_ConnectionID");
        serviceDescription->addArgument(action, "RcsID", UpnpServiceDescription::OUT, "A_ARG_TYPE_RcsID");
        serviceDescription->addArgument(action, "AVTransportID", UpnpServiceDescription::OUT, "A_ARG_TYPE_AVTransportID");
        serviceDescription->addArgument(action, "ProtocolInfo", UpnpServiceDescription::OUT, "A_ARG_TYPE_ProtocolInfo");
        serviceDescription->addArgument(action, "PeerConnectionManager", UpnpServiceDescription::OUT, "A_ARG_TYPE_ConnectionManager");
        serviceDescription->addArgument(action, "PeerConnectionID", UpnpServiceDescription::OUT, "A_ARG_TYPE_ConnectionID");
        serviceDescription->addArgument(action, "Direction", UpnpServiceDescription::OUT, "A_ARG_TYPE_Direction");
        serviceDescription->addArgument(action, "Status", UpnpServiceDescription::OUT, "A_ARG_TYPE_ConnectionStatus");
    }
    else
    {
        qCritical() << "invalid service description";
    }
}

void ServiceConnectionManager::initStateVariables()
{
    UpnpServiceDescription *serviceDescription = description();

    if (description())
    {
        QDomElement stateVariable = serviceDescription->addStateVariable("SourceProtocolInfo", true, false, "string");

        stateVariable = serviceDescription->addStateVariable("SinkProtocolInfo", true, false, "string");

        stateVariable = serviceDescription->addStateVariable("CurrentConnectionIDs", true, false, "string", "0");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_ConnectionStatus", false, false, "string");
        QStringList l_values;
        l_values << "OK" << "ContentFormatMismatch" << "InsufficientBandwidth" << "UnreliableChannel" << "Unknown";
        serviceDescription->addAllowedValueList(stateVariable, l_values);

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_ConnectionManager", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Direction", false, false, "string");
        l_values.clear();
        l_values << "Output" << "Input";
        serviceDescription->addAllowedValueList(stateVariable, l_values);

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_ProtocolInfo", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_ConnectionID", false, false, "i4");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_AVTransportID", false, false, "i4");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_RcsID", false, false, "i4");
    }
    else
    {
        qCritical() << "invalid service description";
    }
}

bool ServiceConnectionManager::replyAction(HttpRequest *request, const SoapAction &action)
{
    if (action.actionName() == "GetProtocolInfo")
    {
        StateVariableItem *sourceProtocal = findStateVariableByName("SourceProtocolInfo");
        StateVariableItem *sinkProtocal = findStateVariableByName("SinkProtocolInfo");
        if (sourceProtocal && sinkProtocal)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("Source", sourceProtocal->data(StateVariableItem::ValueRole).toString());
            response.addArgument("Sink", sinkProtocal->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SourceProtocolInfo or SinkProtocolInfo";
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }

        return true;
    }
    else if (action.actionName() == "GetCurrentConnectionIDs")
    {
        StateVariableItem *currentConnection = findStateVariableByName("CurrentConnectionIDs");
        if (currentConnection)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("ConnectionIDs", currentConnection->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable CurrentConnectionIDs";
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }

        return true;
    }
    else if (action.actionName() == "GetCurrentConnectionInfo")
    {
        QString connectionID = action.argumentValue("ConnectionID");
        qWarning() << "GetCurrentConnectionInfo : " << connectionID;

        return true;
    }
    else
    {
        qCritical() << "unknwon action" << action.actionName();
        UpnpError error(UpnpError::INVALID_ACTION);
        request->replyError(error);
        return false;
    }
}
