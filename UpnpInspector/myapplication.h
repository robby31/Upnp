#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include "application.h"
#include "upnpcontrolpoint.h"
#include "Models/listmodel.h"
#include "messageitem.h"
#include "upnprootdevice.h"
#include "qmdnsengine/server.h"
#include "qmdnsengine/cache.h"
#include "qmdnsengine/browser.h"
#include "qmdnsengine/service.h"
#include "qmdnsengine/resolver.h"
#include "qmdnsengine/mdns.h"

class MyApplication : public Application
{
    Q_OBJECT

    Q_PROPERTY(ListModel *messageModel READ messageModel NOTIFY messageModelChanged)
    Q_PROPERTY(UpnpControlPoint *upnpControlPoint READ upnpControlPoint  NOTIFY upnpControlPointChanged)

public:
    explicit MyApplication(int &argc, char **argv);

    ListModel *messageModel() const { return m_messageModel; }
    UpnpControlPoint *upnpControlPoint() const { return m_upnp; }

signals:
    void messageModelChanged();
    void upnpControlPointChanged();

private slots:
    void messageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);

    void mDnsServiceAdded(QMdnsEngine::Service service);

private:
    QNetworkAccessManager netManager;
    UpnpControlPoint *m_upnp;
    ListModel *m_messageModel;
    QMdnsEngine::Server m_mDnsServer;
    QMdnsEngine::Cache m_mDnsCache;
    QMdnsEngine::Browser m_mDnsBrowser;
};

#endif // MYAPPLICATION_H
