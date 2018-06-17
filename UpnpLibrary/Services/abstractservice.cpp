#include "abstractservice.h"

AbstractService::AbstractService(QObject *parent) :
    UpnpService(parent)
{

}

AbstractService::AbstractService(UpnpObject *upnpParent, QObject *parent) :
    UpnpService(upnpParent, parent)
{

}


AbstractService::AbstractService(UpnpObject *upnpParent, QDomNode info, QObject *parent) :
    UpnpService(upnpParent, parent)
{
    Q_UNUSED(info)

    qCritical() << "this class doesn't support initialisation of info";
}

UpnpServiceDescription *AbstractService::description() const
{
    return (UpnpServiceDescription*) UpnpService::description();
}

void AbstractService::initDescription()
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
