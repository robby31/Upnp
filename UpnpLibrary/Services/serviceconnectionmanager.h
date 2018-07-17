#ifndef SERVICECONNECTIONMANAGER_H
#define SERVICECONNECTIONMANAGER_H

#include "abstractservice.h"

class ServiceConnectionManager : public AbstractService
{
    Q_OBJECT

public:
    enum T_DIRECTION { Input, Output };
    enum T_STATUS { Unknown, Ok };

    explicit ServiceConnectionManager(QObject *parent = nullptr);
    explicit ServiceConnectionManager(UpnpObject *upnpParent, QObject *parent = nullptr);
    explicit ServiceConnectionManager(UpnpObject *upnpParent, QDomNode info, QObject *parent = nullptr);

    void setDirection(const T_DIRECTION &direction);
    T_DIRECTION direction() const;

    void setConnectionStatus(const T_STATUS &status);
    T_STATUS connectionStatus() const;

private:
    virtual void initDescription() Q_DECL_OVERRIDE;
    virtual void initActions() Q_DECL_OVERRIDE;
    virtual void initStateVariables() Q_DECL_OVERRIDE;

protected:
    virtual bool replyAction(HttpRequest *request, const SoapAction &action) Q_DECL_OVERRIDE;

signals:

public slots:

private:
    T_DIRECTION m_direction = Input;
    T_STATUS m_connectionStatus = Ok;
};

#endif // SERVICECONNECTIONMANAGER_H
