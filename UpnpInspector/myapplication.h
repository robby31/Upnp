#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include "application.h"
#include "upnpcontrolpoint.h"
#include "Models/listmodel.h"
#include "messageitem.h"
#include "upnprootdevice.h"

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

    virtual void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private:
    QNetworkAccessManager netManager;
    UpnpControlPoint *m_upnp;
    ListModel *m_messageModel;
};

#endif // MYAPPLICATION_H
