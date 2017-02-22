#ifndef UPNPDEVICE_H
#define UPNPDEVICE_H

#include "upnpobject.h"
#include "upnpservice.h"
#include "Models/listmodel.h"

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
    explicit UpnpDevice(QString uuid, UpnpObject *upnpParent, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString uuid() const;
    QString deviceType() const;
    QString friendlyName() const;

    ListModel *devicesModel() const;
    ListModel *servicesModel() const;

    UpnpObject *getUpnpObjectFromUSN(const QString &usn);

private:
    void initRoles();

    void addService(const QDomNode &descr);
    void addDevice(const QDomNode &descr);

    UpnpService *getServiceFromType(const QString &type);

signals:
    void deviceTypeChanged();
    void servicesModelChanged();
    void devicesModelChanged();

public slots:

private slots:
    void itemAvailableChanged();
    void readServices();
    void readDevices();

private:
    QString m_uuid;
    ListModel *m_services;
    ListModel *m_devices;
};

#endif // UPNPDEVICE_H
