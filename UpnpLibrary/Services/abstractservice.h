#ifndef ABSTRACTSERVICE_H
#define ABSTRACTSERVICE_H

#include "upnpservice.h"

class AbstractService : public UpnpService
{
    Q_OBJECT

public:
    explicit AbstractService(QObject *parent = Q_NULLPTR);
    explicit AbstractService(UpnpObject *upnpParent, QObject *parent = Q_NULLPTR);
    explicit AbstractService(UpnpObject *upnpParent, const QDomNode &info, QObject *parent = Q_NULLPTR);

    UpnpServiceDescription *description() const;

protected:
    void replyAnswer(HttpRequest *request, const SoapActionResponse &response);
};

#endif // ABSTRACTSERVICE_H
