#ifndef UPNPCONTROLPOINT_H
#define UPNPCONTROLPOINT_H

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

class UpnpControlPoint : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString serverName READ serverName NOTIFY serverNameChanged)
    Q_PROPERTY(ListModel *rootDevices READ rootDevices NOTIFY rootDevicesChanged)

public:
    explicit UpnpControlPoint(QObject *parent = 0);
    virtual ~UpnpControlPoint();

    void start() { emit startSignal(); }
    void close();

    void setUuid(const QString &uuid)       { m_uuid = uuid;        }
    void setServerUrl(const QString &url)   { m_serverurl = url;    }
    void setHost(const QString &host)       { m_host = host;        }

    void setNetworkManager(QNetworkAccessManager *nam);

    QString serverName() const { return m_servername; }

    ListModel *rootDevices() const { return m_rootDevice; }

private:
    void addRootDevice(QHostAddress host, SsdpMessage message);
    UpnpRootDevice *getRootDeviceFromUuid(const QString &uuid);

    void _sendAlive(const QString &notification_type);
    void _sendByeBye(const QString &notification_type);
    void _sendDiscover(const QString &search_target);

    void _sendDiscoverAnswer(const QHostAddress &host, const int &port, const QString &st);

    QNetworkReply *_sendAction(const QHostAddress &host, const int &port, const QString &url, const QString &service, const QString &action);

signals:
    void startSignal();

    void serverNameChanged();
    void rootDevicesChanged();

    void messageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);
    void deviceAlive(const QHostAddress &host, const int &port, const SsdpMessage &info);
    void deviceByeBye(const QHostAddress &host, const int &port, const SsdpMessage &info);
    void newMediaRenderer(const QHostAddress &host, const int &port, const SsdpMessage &info);

private slots:
    void _start();

    void _sendAlive();
    void _sendByeBye();

    // Function called when a request is received
    void _processPendingMulticastDatagrams();
    void _processPendingUnicastDatagrams();

    void _processSsdpMessageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);


private:
    QNetworkAccessManager *netManager;

    QString m_uuid;
    QString m_servername;
    QString m_serverurl;
    QString m_host;

    QUdpSocket udpSocketMulticast;
    QUdpSocket udpSocketUnicast;

    // Timer to broadcast UPnP ALIVE messages
    QTimer timerAlive;
    int counterAlive;

    int m_bootid;
    int m_configid;

    ListModel *m_rootDevice;

    // The Constant ALIVE.
    static const QString ALIVE;

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

    // The Constant BYEBYE.
    static const QString BYEBYE;
};

#endif // UPNPCONTROLPOINT_H
