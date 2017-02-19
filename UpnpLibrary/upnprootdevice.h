#ifndef UPNPROOTDEVICE_H
#define UPNPROOTDEVICE_H

#include "upnpdevice.h"

class UpnpRootDevice : public UpnpDevice
{
    Q_OBJECT

    Q_PROPERTY(QString rootDescription READ rootDescription  NOTIFY rootDescriptionChanged)

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

public:
    explicit UpnpRootDevice(QObject *parent = 0);
    explicit UpnpRootDevice(QHostAddress host, QString uuid, QObject *parent = 0);

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString version() const;

    void requestDescription(QString location);
    QString rootDescription() const;
    void setRootDescription(QByteArray data);

private:
    void initRoles();

signals:
    void rootDescriptionChanged();
    void upnpObjectAvailabilityChanged(UpnpObject *object);

public slots:

private slots:
    void itemAvailableChanged();
    void descriptionReceived();

private:
    QDomDocument m_rootDescription;
    QString m_iconUrl;
};

#endif // UPNPROOTDEVICE_H
