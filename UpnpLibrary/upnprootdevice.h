#ifndef UPNPROOTDEVICE_H
#define UPNPROOTDEVICE_H

#include <QObject>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "ssdpmessage.h"
#include "upnpdevice.h"
#include "upnpobject.h"
#include "Models/listmodel.h"

class UpnpRootDevice : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(QString deviceType READ deviceType NOTIFY deviceTypeChanged)
    Q_PROPERTY(bool deviceAvailable READ deviceAvailable NOTIFY deviceAvailableChanged)
    Q_PROPERTY(ListModel *devicesModel READ devicesModel NOTIFY devicesModelChanged)
    Q_PROPERTY(ListModel *servicesModel READ servicesModel NOTIFY servicesModelChanged)

    enum Roles {
        HostRole = Qt::UserRole+1,
        UuidRole,
        ServerNameRole,
        FriendlyNameRole,
        IconUrlRole,
        AvailableRole,
        DeviceTypeRole,
        PresentationUrlRole,
        VersionRole
    };

public:
    explicit UpnpRootDevice(QObject *parent = 0);
    explicit UpnpRootDevice(QHostAddress host, QString uuid, SsdpMessage message, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString version() const;
    QString servername() const;
    QString deviceType() const;
    bool deviceAvailable() const;
    QString friendlyName() const;

    ListModel *devicesModel() const;
    ListModel *servicesModel() const;

    QString getMessageHeader(const QString &param) const;

    void getDescription();

    UpnpDevice *getDeviceFromUuid(const QString &uuid);
    UpnpDevice *getDeviceFromType(const QString &type);
    UpnpService *getServiceFromType(const QString &type);

private:
    void initRoles();

signals:
    void deviceTypeChanged();
    void deviceAvailableChanged();
    void devicesModelChanged();
    void servicesModelChanged();

public slots:

private slots:
    void availableSlotChanged();
    void descriptionReceived();

private:
    SsdpMessage m_message;
    QString m_uuid;
    UpnpDevice *m_device;
    QString m_iconUrl;
};

#endif // UPNPROOTDEVICE_H
