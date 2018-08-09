#ifndef ABSTRACTSERVICE_H
#define ABSTRACTSERVICE_H

#include "upnpservice.h"

class AbstractService : public UpnpService
{
    Q_OBJECT

public:
    explicit AbstractService(QObject *parent = Q_NULLPTR);
    explicit AbstractService(UpnpObject *upnpParent, QObject *parent = Q_NULLPTR);
    explicit AbstractService(UpnpObject *upnpParent, QDomNode info, QObject *parent = Q_NULLPTR);

    UpnpServiceDescription *description() const;

private:
    virtual void initDescription() = 0;
    virtual void initActions() = 0;
    virtual void initStateVariables() = 0;

signals:

public slots:

};

#endif // ABSTRACTSERVICE_H
