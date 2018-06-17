#ifndef ABSTRACTSERVICE_H
#define ABSTRACTSERVICE_H

#include "upnpservice.h"

class AbstractService : public UpnpService
{
    Q_OBJECT

public:
    explicit AbstractService(QObject *parent = nullptr);
    explicit AbstractService(UpnpObject *upnpParent, QObject *parent = nullptr);
    explicit AbstractService(UpnpObject *upnpParent, QDomNode info, QObject *parent = nullptr);

    UpnpServiceDescription *description() const;

private:
    virtual void initDescription() = 0;
    virtual void initActions() = 0;
    virtual void initStateVariables() = 0;

signals:

public slots:

};

#endif // ABSTRACTSERVICE_H
