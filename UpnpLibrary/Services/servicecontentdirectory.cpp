#include "servicecontentdirectory.h"

ServiceContentDirectory::ServiceContentDirectory(QObject *parent) :
    AbstractService(parent)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();
}

ServiceContentDirectory::ServiceContentDirectory(UpnpObject *upnpParent, QObject *parent) :
    AbstractService(upnpParent, parent)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();
}


ServiceContentDirectory::ServiceContentDirectory(UpnpObject *upnpParent, QDomNode info, QObject *parent) :
    AbstractService(upnpParent, info, parent)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();
}

void ServiceContentDirectory::initDescription()
{
    UpnpServiceDescription *serviceDescription = new UpnpServiceDescription();
    serviceDescription->setServiceAttribute("serviceType", "urn:schemas-upnp-org:service:ContentDirectory:1");
    serviceDescription->setServiceAttribute("serviceId", "urn:upnp-org:serviceId:ContentDirectory");
    serviceDescription->setServiceAttribute("SCPDURL", "/UPnP_AV_ContentDirectory_1.0.xml");
    serviceDescription->setServiceAttribute("controlURL", "/upnp/control/content_directory");
    serviceDescription->setServiceAttribute("eventSubURL", "/upnp/event/content_directory");

    setInfo(serviceDescription->xmlInfo());

    setDescription(serviceDescription);
}

void ServiceContentDirectory::initActions()
{
    UpnpServiceDescription *serviceDescription = description();

    if (description())
    {
        QDomElement action = serviceDescription->addAction("GetSearchCapabilities");
        serviceDescription->addArgument(action, "SearchCaps", UpnpServiceDescription::OUT, "SearchCapabilities");

        action = serviceDescription->addAction("GetSortCapabilities");
        serviceDescription->addArgument(action, "SortCaps", UpnpServiceDescription::OUT, "SortCapabilities");

        action = serviceDescription->addAction("GetSystemUpdateID");
        serviceDescription->addArgument(action, "Id", UpnpServiceDescription::OUT, "SystemUpdateID");

        action = serviceDescription->addAction("Browse");
        serviceDescription->addArgument(action, "ObjectID", UpnpServiceDescription::IN, "A_ARG_TYPE_ObjectID");
        serviceDescription->addArgument(action, "BrowseFlag", UpnpServiceDescription::IN, "A_ARG_TYPE_BrowseFlag");
        serviceDescription->addArgument(action, "Filter", UpnpServiceDescription::IN, "A_ARG_TYPE_Filter");
        serviceDescription->addArgument(action, "StartingIndex", UpnpServiceDescription::IN, "A_ARG_TYPE_Index");
        serviceDescription->addArgument(action, "RequestedCount", UpnpServiceDescription::IN, "A_ARG_TYPE_Count");
        serviceDescription->addArgument(action, "SortCriteria", UpnpServiceDescription::IN, "A_ARG_TYPE_SortCriteria");
        serviceDescription->addArgument(action, "Result", UpnpServiceDescription::OUT, "A_ARG_TYPE_Result");
        serviceDescription->addArgument(action, "NumberReturned", UpnpServiceDescription::OUT, "A_ARG_TYPE_Count");
        serviceDescription->addArgument(action, "TotalMatches", UpnpServiceDescription::OUT, "A_ARG_TYPE_Count");
        serviceDescription->addArgument(action, "UpdateID", UpnpServiceDescription::OUT, "A_ARG_TYPE_UpdateID");
    }
    else
    {
        qCritical() << "invalid service description";
    }
}

void ServiceContentDirectory::initStateVariables()
{
    UpnpServiceDescription *serviceDescription = description();

    if (description())
    {
        QDomElement stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_ObjectID", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Result", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_BrowseFlag", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Filter", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_SortCriteria", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Index", false, false, "ui4");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Count", false, false, "ui4");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_UpdateID", false, false, "ui4");

        stateVariable = serviceDescription->addStateVariable("SearchCapabilities", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("SortCapabilities", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("SystemUpdateID", true, false, "ui4");
    }
    else
    {
        qCritical() << "invalid service description";
    }
}

bool ServiceContentDirectory::replyAction(HttpRequest *request, const SoapAction &action)
{
    if (action.actionName() == "GetSearchCapabilities")
    {
        StateVariableItem *searchCaps = findStateVariableByName("SearchCapabilities");

        if (action.arguments().size() != 0)
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }
        else if (searchCaps)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("SearchCaps", searchCaps->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SearchCapabilities";
            UpnpError error(UpnpError::ACTION_FAILED);
            request->replyError(error);
        }

        return true;
    }
    else if (action.actionName() == "GetSortCapabilities")
    {
        StateVariableItem *sortCaps = findStateVariableByName("SortCapabilities");

        if (action.arguments().size() != 0)
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }
        else if (sortCaps)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("SortCaps", sortCaps->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SortCapabilities";
            UpnpError error(UpnpError::ACTION_FAILED);
            request->replyError(error);
        }

        return true;
    }
    else if (action.actionName() == "GetSystemUpdateID")
    {
        StateVariableItem *sysUpdateId = findStateVariableByName("SystemUpdateID");

        if (action.arguments().size() != 0)
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }
        else if (sysUpdateId)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("Id", sysUpdateId->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SystemUpdateID";
            UpnpError error(UpnpError::ACTION_FAILED);
            request->replyError(error);
        }

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
