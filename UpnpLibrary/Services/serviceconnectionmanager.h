#ifndef SERVICECONNECTIONMANAGER_H
#define SERVICECONNECTIONMANAGER_H

#include "abstractservice.h"

class ServiceConnectionManager : public AbstractService
{
    Q_OBJECT

public:
    enum T_DIRECTION { Input, Output };
    enum T_STATUS { Unknown, Ok };

    explicit ServiceConnectionManager(QObject *parent = Q_NULLPTR);
    explicit ServiceConnectionManager(UpnpObject *upnpParent, QObject *parent = Q_NULLPTR);
    explicit ServiceConnectionManager(UpnpObject *upnpParent, const QDomNode& info, QObject *parent = Q_NULLPTR);

    void setDirection(const T_DIRECTION &direction);
    T_DIRECTION direction() const;

    void setConnectionStatus(const T_STATUS &status);
    T_STATUS connectionStatus() const;

private:
    void initDescription();
    void initActions();
    void initStateVariables();

protected:
    bool replyAction(HttpRequest *request, const SoapAction &action) Q_DECL_OVERRIDE;

signals:

public slots:

private:
    T_DIRECTION m_direction = Input;
    T_STATUS m_connectionStatus = Ok;
};

#endif // SERVICECONNECTIONMANAGER_H
