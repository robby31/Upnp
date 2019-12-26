#include "abstractservice.h"

AbstractService::AbstractService(QObject *parent) :
    UpnpService(parent)
{

}

AbstractService::AbstractService(UpnpObject *upnpParent, QObject *parent) :
    UpnpService(upnpParent, parent)
{

}


AbstractService::AbstractService(UpnpObject *upnpParent, const QDomNode &info, QObject *parent) :
    UpnpService(upnpParent, parent)
{
    Q_UNUSED(info)

    qCritical() << "this class doesn't support initialisation of info";
}

UpnpServiceDescription *AbstractService::description() const
{
    return qobject_cast<UpnpServiceDescription*>(UpnpService::description());
}

void AbstractService::replyAnswer(HttpRequest *request, const SoapActionResponse &response)
{
    QByteArray data = response.toByteArray();

    QStringList header;
    header << QString("Content-Type: text/xml; charset=\"utf-8\"");
    header << QString("Content-Length: %1").arg(data.size());
    header << QString("DATE: %1").arg(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
    header << QString("EXT:");
    if (!serverName().isEmpty())
        header << QString("SERVER: %1").arg(serverName());

    request->replyData(HttpRequest::HTTP_200_OK, header, data);
}
