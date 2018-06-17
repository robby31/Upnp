#ifndef SERVICECONTENTDIRECTORY_H
#define SERVICECONTENTDIRECTORY_H

#include "abstractservice.h"

class ServiceContentDirectory : public AbstractService
{
    Q_OBJECT

public:
    explicit ServiceContentDirectory(QObject *parent = nullptr);
    explicit ServiceContentDirectory(UpnpObject *upnpParent, QObject *parent = nullptr);
    explicit ServiceContentDirectory(UpnpObject *upnpParent, QDomNode info, QObject *parent = nullptr);

private:
    virtual void initDescription() Q_DECL_OVERRIDE;
    virtual void initActions() Q_DECL_OVERRIDE;
    virtual void initStateVariables() Q_DECL_OVERRIDE;

protected:
    virtual bool replyAction(HttpRequest *request, const SoapAction &action) Q_DECL_OVERRIDE;

signals:

public slots:

};

#endif // SERVICECONTENTDIRECTORY_H
