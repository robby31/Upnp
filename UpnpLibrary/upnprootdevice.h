#ifndef UPNPROOTDEVICE_H
#define UPNPROOTDEVICE_H

#include "upnpdevice.h"
#include "upnptimer.h"
#include "httprequest.h"
#include "upnprootdevicedescription.h"
#include "httpserver.h"

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

    explicit UpnpRootDevice(QObject *parent = Q_NULLPTR);
    explicit UpnpRootDevice(const QString& macAddress, const QString& uuid, QObject *parent = Q_NULLPTR);

    QVariant data(int role) const Q_DECL_OVERRIDE;

    UpnpRootDeviceDescription *description() const;

    QString version() const;

    int bootId() const;
    QHostAddress host() const Q_DECL_OVERRIDE;
    quint16 port() const Q_DECL_OVERRIDE;

    QString configId() const;

    QString iconUrl() const;
    QStringList iconUrls() const;

    QString serverName() const Q_DECL_OVERRIDE;
    void setServerName(const QString &name);

    QUrl url() const Q_DECL_OVERRIDE;
    void setUrl(const QUrl& url);

    void setAdvertise(const bool &flag);
    void startAdvertising();
    void startServer();

    void searchForST(const QHostAddress &host, const quint16 &port, const QString &st) Q_DECL_OVERRIDE;

    QString generateUuid() Q_DECL_OVERRIDE;

private:
    void initRoles();
    QString _generateUuid();

signals:
    void urlChanged();
    void newRequest(HttpRequest *request);
    void requestCompleted(HttpRequest *request);
    void serverStarted();
    void serverError(const QString &message);

public slots:
    void sendAlive() Q_DECL_OVERRIDE;
    void sendByeBye() Q_DECL_OVERRIDE;

    void requestDescription();

private slots:
    void itemAvailableChanged();
    void statusChangedSlot();

    void descriptionReceived();

    void replyRequest(HttpRequest *request) Q_DECL_OVERRIDE;
    virtual void replyGetIcon(HttpRequest *request);

public:
    static const QString UPNP_ROOTDEVICE;

private:
    QString m_servername;
    int m_bootId = 0;
    QUrl m_url;
    QString m_iconUrl;

    bool m_advertise = false;
    UpnpTimer m_advertisingTimer;
    HttpServer *server = Q_NULLPTR;
    QString m_macAddress;
};

#endif // UPNPROOTDEVICE_H
