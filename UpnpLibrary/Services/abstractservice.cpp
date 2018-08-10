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
    return qobject_cast<UpnpServiceDescription*>(UpnpService::description());
}
