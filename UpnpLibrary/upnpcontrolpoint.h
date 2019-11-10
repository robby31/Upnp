#ifndef UPNPCONTROLPOINT_H
#define UPNPCONTROLPOINT_H

#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QHostAddress>
#include <QTimer>
#include <QUdpSocket>
#include <QNetworkInterface>
#include "ssdpmessage.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include "upnprootdevice.h"
#include "Models/listmodel.h"
#include <chrono>
#include "eventresponse.h"
#include <QRandomGenerator>
#include"Devices/devicesmodel.h"
#include "debuginfo.h"

typedef struct {
    QString deviceUuid;
    QString serviceId;
    QString timeOut;
} T_EVENT;

typedef struct {
    QHostAddress host;
    quint16 port;
    QString st;
} T_SEARCH_ANSWER;

typedef struct {
    QString searchTarget;
    int counter;
} T_DISCOVER;

class UpnpControlPoint : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString serverName READ serverName NOTIFY serverNameChanged)
    Q_PROPERTY(ListModel *localRootDevices READ localRootDevices NOTIFY localRootDevicesChanged)
    Q_PROPERTY(DevicesModel *remoteRootDevices READ remoteRootDevices NOTIFY remoteRootDevicesChanged)

public:
    explicit UpnpControlPoint(QObject *parent = Q_NULLPTR);
    explicit UpnpControlPoint(quint16 eventPort = UPNP_PORT, QObject *parent = Q_NULLPTR);
    ~UpnpControlPoint() Q_DECL_OVERRIDE;

    void close();

    QString macAddress() const;
    QString serverName() const;
    QHostAddress host() const;

    ListModel *localRootDevices() const;
    DevicesModel *remoteRootDevices() const;

    UpnpRootDevice *addLocalRootDevice(UpnpRootDeviceDescription *description, int port, const QString& url);
    bool addLocalRootDevice(UpnpRootDevice *device);

    void startDiscover(const QString &searchTarget = QString("ssdp:all"));

protected:
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private:
    void initializeHostAdress();

    void sendDiscover(const QString &search_target);

    // return sid event subscribed
    QString eventSubscribed(const QString &uuid, const QString &serviceId);

signals:
    void serverNameChanged();
    void localRootDevicesChanged();
    void remoteRootDevicesChanged();

    void messageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);

    void newRootDevice(UpnpRootDevice *device);

private slots:
    void _sendMulticastSsdpMessage(const SsdpMessage& message);
    void _sendUnicastSsdpMessage(const QHostAddress &host, const quint16 &port, const SsdpMessage& message);

    void _sendAliveMessage(const QString &uuid, const QString &nt);
    void _sendByeByeMessage(const QString &uuid, const QString &nt);

    void _searchForST(const QHostAddress &host, const quint16 &port, const QString &st);
    void _sendSearchResponse(const QHostAddress &host, const quint16 &port, const QString &st, const QString &usn);

    // Function called when a request is received
    void _processPendingMulticastDatagrams();
    void _processPendingUnicastDatagrams();

    void _processSsdpMessageReceived(const QHostAddress &host, const quint16 &port, const SsdpMessage &message);

    void subscribeEventing(QNetworkRequest request, const QString &uuid, const QString &serviceId);
    void subscribeEventingFinished();
    void requestEventReceived(HttpRequest *request);

    void removeSidEventFromUuid(const QString &deviceUuid);

private:
    HttpServer eventServer;
    quint16 m_eventPort;
    QHash<QString, T_EVENT> m_sidEvent;
    int m_eventCheckSubscription = -1;
    QHash<int, T_SEARCH_ANSWER> m_searchAnswer;
    QHash<int, T_DISCOVER> m_discover;

    QString m_servername;
    QHostAddress m_hostAddress;
    QString m_macAddress;

    QUdpSocket udpSocketMulticast;
    QUdpSocket udpSocketUnicast;

    int m_bootid = 0;
    int m_configid = 0;

    DevicesModel *m_remoteRootDevice = Q_NULLPTR;
    ListModel *m_localRootDevice = Q_NULLPTR;

    static const QString UPNP_VERSION;

    /*
     * IPv4 Multicast channel reserved for SSDP by Internet Assigned Numbers Authority (IANA).
     * MUST be 239.255.255.250.
     */
    static const QHostAddress IPV4_UPNP_HOST;

    /*
     * Multicast channel reserved for SSDP by Internet Assigned Numbers Authority (IANA).
     * MUST be 1900.
     */
    static const quint16 UPNP_PORT;
};

#endif // UPNPCONTROLPOINT_H
