#ifndef UPNPROOTDEVICE_H
#define UPNPROOTDEVICE_H

#include "upnpdevice.h"
#include "upnptimer.h"
#include "Http/httpserver.h"
#include "upnprootdevicedescription.h"

class UpnpRootDevice : public UpnpDevice
{
    Q_OBJECT

public:

    enum Roles {
        HostRole = Qt::UserRole+1,
        UuidRole,
        FriendlyNameRole,
        IconUrlRole,
        AvailableRole,
        DeviceTypeRole,
        PresentationUrlRole,
        VersionRole
    };

    explicit UpnpRootDevice(QObject *parent = 0);
    explicit UpnpRootDevice(QNetworkAccessManager *nam, QString macAddress, QString uuid, QObject *parent = 0);

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    UpnpRootDeviceDescription *description() const;

    QString version() const;

    virtual QNetworkAccessManager *networkManager() const Q_DECL_OVERRIDE;
    void setNetworkManager(QNetworkAccessManager *nam);

    int bootId() const;
    virtual QHostAddress host() const Q_DECL_OVERRIDE;
    int port() const;

    QString configId() const;

    QString iconUrl() const;
    QStringList iconUrls() const;

    virtual QString serverName() const Q_DECL_OVERRIDE;
    void setServerName(const QString &name);

    virtual QUrl url() const Q_DECL_OVERRIDE;
    void setUrl(QUrl url);

    void setAdvertise(const bool &flag);
    void startAdvertising();
    void startServer();

    virtual void searchForST(const QHostAddress &host, const int &port, const QString &st) Q_DECL_OVERRIDE;

    virtual QString generateUuid() Q_DECL_OVERRIDE;

private:
    void initRoles();

signals:
    void urlChanged();
    void newRequest(HttpRequest *request);
    void requestCompleted(HttpRequest *request);
    void serverStarted();
    void serverError(const QString &message);

public slots:
    virtual void sendAlive() Q_DECL_OVERRIDE;
    virtual void sendByeBye() Q_DECL_OVERRIDE;

    void requestDescription();

private slots:
    void itemAvailableChanged();
    void statusChangedSlot();

    void descriptionReceived();

    virtual void replyRequest(HttpRequest *request) Q_DECL_OVERRIDE;
    virtual void replyGetIcon(HttpRequest *request);

public:
    static const QString UPNP_ROOTDEVICE;

private:
    QNetworkAccessManager *netManager;
    QString m_servername;
    int m_bootId = 0;
    QUrl m_url;
    QString m_iconUrl;

    bool m_advertise;
    UpnpTimer m_advertisingTimer;
    HttpServer *server;
    QString m_macAddress;
};

#endif // UPNPROOTDEVICE_H
