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

class UpnpControlPoint : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString serverName READ serverName NOTIFY serverNameChanged)
    Q_PROPERTY(ListModel *localRootDevices READ localRootDevices NOTIFY localRootDevicesChanged)
    Q_PROPERTY(ListModel *remoteRootDevices READ remoteRootDevices NOTIFY remoteRootDevicesChanged)

public:
    explicit UpnpControlPoint(QObject *parent = 0);
    virtual ~UpnpControlPoint();

    void close();

    QString serverName() const;
    QHostAddress host() const;

    void setNetworkManager(QNetworkAccessManager *nam);

    ListModel *localRootDevices() const;
    ListModel *remoteRootDevices() const;

    UpnpRootDevice *addLocalRootDevice(int port, QString uuid, QString url);
    void advertiseLocalRootDevice();

    void sendDiscover(const QString &search_target);

    QString generateUuid();

private:
    void initializeHostAdress();

    void addRootDevice(SsdpMessage message);
    UpnpRootDevice *getRootDeviceFromUuid(const QString &uuid);

    UpnpObject *getUpnpObjectFromUSN(const QString &usn);

signals:
    void serverNameChanged();
    void localRootDevicesChanged();
    void remoteRootDevicesChanged();

    void messageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);

    void newRootDevice(UpnpRootDevice *device);

private slots:
    void _sendMulticastSsdpMessage(SsdpMessage message);
    void _sendAliveMessage(const QString &uuid, const QString &nt);
    void _sendByeByeMessage(const QString &uuid, const QString &nt);

    void _searchForST(const QString &st);
    void _sendSearchResponse(const QString &st, const QString &usn);

    // Function called when a request is received
    void _processPendingMulticastDatagrams();
    void _processPendingUnicastDatagrams();

    void _processSsdpMessageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);

    void _rootDeviceStatusChanged();
    void _rootDeviceAvailableChanged();

    void subscribeEventing(QNetworkRequest request, const QString &uuid, const QString &serviceId);
    void subscribeEventingFinished();
    void requestEventReceived(HttpRequest *request);

private:
    QNetworkAccessManager *netManager;

    HttpServer eventServer;
    QHash<QString, QStringList> m_sidEvent;

    QString m_servername;
    QHostAddress m_hostAddress;
    QString m_macAddress;

    QUdpSocket udpSocketMulticast;
    QUdpSocket udpSocketUnicast;

    int m_bootid;
    int m_configid;

    ListModel *m_remoteRootDevice;
    ListModel *m_localRootDevice;

    static const QString UPNP_VERSION;
    static const QString ALIVE;
    static const QString BYEBYE;
    static const QString DISCOVER;

    /*
     * IPv4 Multicast channel reserved for SSDP by Internet Assigned Numbers Authority (IANA).
     * MUST be 239.255.255.250.
     */
    static const QHostAddress IPV4_UPNP_HOST;

    /*
     * Multicast channel reserved for SSDP by Internet Assigned Numbers Authority (IANA).
     * MUST be 1900.
     */
    static const int UPNP_PORT;
};

#endif // UPNPCONTROLPOINT_H
