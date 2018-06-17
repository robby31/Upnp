#ifndef UPNPDEVICE_H
#define UPNPDEVICE_H

#include "upnpobject.h"
#include "upnpservice.h"
#include "Models/listmodel.h"
#include "upnpdevicedescription.h"
#include "Http/httprequest.h"

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
        AvailableRole,
        PresentationUrlRole
    };

    explicit UpnpDevice(QObject *parent = 0);
    explicit UpnpDevice(QString uuid, UpnpObject *upnpParent, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString deviceType() const;
    QString friendlyName() const;

    ListModel *devicesModel() const;
    ListModel *servicesModel() const;

    bool addService(UpnpService *p_service);

    UpnpObject *getUpnpObjectFromUSN(const QString &usn);
    UpnpService *getService(const QString &serviceId);

    virtual void searchForST(const QString &st);

    virtual void replyRequest(HttpRequest *request);

private:
    void initRoles();

    void addService(const QDomNode &descr);
    void addDevice(const QDomNode &descr);

    UpnpService *getServiceFromType(const QString &type);

signals:
    void deviceTypeChanged();
    void servicesModelChanged();
    void devicesModelChanged();
    void subscribeEventingSignal(const QNetworkRequest &request, const QString &uuid, const QString &serviceId);

public slots:
    virtual void sendAlive();
    virtual void sendByeBye();

private slots:
    void itemAvailableChanged();

    virtual void parseObject() Q_DECL_OVERRIDE; // parse device to read Device and Services
    void readServices();
    void readDevices();

    void subscribeEventingSlot(const QNetworkRequest &request, const QString &serviceId);

private:
    QString m_uuid;
    ListModel *m_services;
    ListModel *m_devices;
};

#endif // UPNPDEVICE_H
