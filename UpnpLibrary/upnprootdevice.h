#ifndef UPNPROOTDEVICE_H
#define UPNPROOTDEVICE_H

#include "upnpdevice.h"
#include "upnptimer.h"

class UpnpRootDevice : public UpnpDevice
{
    Q_OBJECT

    Q_PROPERTY(QString rootDescription READ rootDescription  NOTIFY rootDescriptionChanged)

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
    explicit UpnpRootDevice(QNetworkAccessManager *nam, QHostAddress host, QString uuid, QObject *parent = 0);

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString version() const;

    virtual QNetworkAccessManager *networkManager() const Q_DECL_OVERRIDE;
    void setNetworkManager(QNetworkAccessManager *nam);

    virtual QHostAddress host() const Q_DECL_OVERRIDE;

    QString iconUrl() const;

    virtual QString serverName() const Q_DECL_OVERRIDE;
    void setServerName(const QString &name);

    virtual QUrl url() const Q_DECL_OVERRIDE;
    void setUrl(QUrl url);

    QString rootDescription() const;
    void setRootDescription(QByteArray data);

    void setAdvertise(const bool &flag);
    void startAdvertising();

    virtual void searchForST(const QString &st) Q_DECL_OVERRIDE;

private:
    void initRoles();

signals:
    void urlChanged();
    void rootDescriptionChanged();

public slots:
    virtual void sendAlive() Q_DECL_OVERRIDE;
    virtual void sendByeBye() Q_DECL_OVERRIDE;

private slots:
    void itemAvailableChanged();
    void statusChangedSlot();


    void requestDescription();
    void descriptionReceived();

public:
    static const QString UPNP_ROOTDEVICE;

private:
    QNetworkAccessManager *netManager;
    QHostAddress m_host;
    QString m_servername;
    QUrl m_url;
    QDomDocument m_rootDescription;
    QString m_iconUrl;

    bool m_advertise;
    UpnpTimer m_advertisingTimer;
};

#endif // UPNPROOTDEVICE_H
