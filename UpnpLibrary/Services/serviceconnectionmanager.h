#ifndef SERVICECONNECTIONMANAGER_H
#define SERVICECONNECTIONMANAGER_H

#include "abstractservice.h"

class ServiceConnectionManager : public AbstractService
{
    Q_OBJECT

public:
    explicit ServiceConnectionManager(QObject *parent = nullptr);
    explicit ServiceConnectionManager(UpnpObject *upnpParent, QObject *parent = nullptr);
    explicit ServiceConnectionManager(UpnpObject *upnpParent, QDomNode info, QObject *parent = nullptr);

private:
    virtual void initDescription() Q_DECL_OVERRIDE;
    virtual void initActions() Q_DECL_OVERRIDE;
    virtual void initStateVariables() Q_DECL_OVERRIDE;

protected:
    virtual bool replyAction(HttpRequest *request, const SoapAction &action) Q_DECL_OVERRIDE;

signals:

public slots:
};

#endif // SERVICECONNECTIONMANAGER_H
