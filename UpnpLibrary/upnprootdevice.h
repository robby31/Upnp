#ifndef UPNPROOTDEVICE_H
#define UPNPROOTDEVICE_H

#include <QObject>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "ssdpmessage.h"
#include "upnpservice.h"
#include "upnpdevice.h"
#include "upnpobject.h"
#include "Models/listmodel.h"

class UpnpRootDevice : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(ListModel *devicesModel READ devicesModel NOTIFY devicesModelChanged)
    Q_PROPERTY(ListModel *servicesModel READ servicesModel NOTIFY servicesModelChanged)
    Q_PROPERTY(QString deviceType READ deviceType NOTIFY deviceTypeChanged)

    enum Roles {
        HostRole = Qt::UserRole+1,
        UuidRole,
        ServerNameRole,
        FriendlyNameRole,
        IconUrlRole,
        AvailableRole,
        DeviceTypeRole
    };

public:
    explicit UpnpRootDevice(QObject *parent = 0);
    explicit UpnpRootDevice(QHostAddress host, QString uuid, SsdpMessage message, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString servername() const;
    QString friendlyName() const;

    ListModel *devicesModel() const;
    ListModel *servicesModel() const;

    QString getMessageHeader(const QString &param) const;

    void getDescription();
    QString deviceType() const;

    void updateCapability(const SsdpMessage &message);


private:
    void initRoles();
    void addService(const QDomNode &descr);
    void addDevice(const QDomNode &descr);

signals:
    void devicesModelChanged();
    void servicesModelChanged();
    void deviceTypeChanged();

public slots:

private slots:
    void availableSlotChanged();
    void descriptionReceived();

private:
    SsdpMessage m_message;
    QString m_uuid;
    ListModel *m_services;
    ListModel *m_devices;
    QString m_iconUrl;
};

#endif // UPNPROOTDEVICE_H
