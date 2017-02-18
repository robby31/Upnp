#ifndef UPNPDEVICE_H
#define UPNPDEVICE_H

#include <QObject>
#include <QDateTime>
#include <QDebug>
#include "upnpobject.h"
#include "Models/listmodel.h"
#include "upnpservice.h"

class UpnpDevice : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(QString deviceType READ deviceType NOTIFY deviceTypeChanged)
    Q_PROPERTY(ListModel *devicesModel READ devicesModel NOTIFY devicesModelChanged)
    Q_PROPERTY(ListModel *servicesModel READ servicesModel NOTIFY servicesModelChanged)

public:

    enum Roles {
        HostRole = Qt::UserRole+1,
        DeviceTypeRole,
        AvailableRole
    };

    explicit UpnpDevice(QObject *parent = 0);
    explicit UpnpDevice(QHostAddress host, QDomNode info, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString getInfo(const QString &param) const;

    QString deviceType() const;
    QString friendlyName() const;

    ListModel *devicesModel() const;
    ListModel *servicesModel() const;

    void readServices();
    void readDevices();

    QString getUuid() const;

    UpnpDevice *getDeviceFromUuid(const QString &uuid);
    UpnpDevice *getDeviceFromType(const QString &type);
    UpnpService *getServiceFromType(const QString &type);

private:
    void initRoles();

    void addService(const QDomNode &descr);
    void addDevice(const QDomNode &descr);

signals:
    void deviceTypeChanged();
    void servicesModelChanged();
    void devicesModelChanged();

public slots:

private slots:
    void availableSlotChanged();

private:
    QDomNode m_info;
    QHostAddress m_host;
    ListModel *m_services;
    ListModel *m_devices;
};

#endif // UPNPDEVICE_H
