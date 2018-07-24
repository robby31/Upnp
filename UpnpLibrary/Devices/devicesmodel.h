#ifndef DEVICESMODEL_H
#define DEVICESMODEL_H

#include "Models/listmodel.h"
#include "upnpobject.h"
#include "upnprootdevice.h"

class DevicesModel : public ListModel
{
    Q_OBJECT

public:
    explicit DevicesModel(ListItem* prototype, QObject* parent = 0);
    explicit DevicesModel(QObject *parent = nullptr);
    virtual ~DevicesModel() Q_DECL_OVERRIDE;

    void setNetworkManager(QNetworkAccessManager *nam);
    void setMacAddress(const QString &m_address);

    UpnpObject *getUpnpObjectFromUSN(const QString &usn);

    UpnpService *getService(const QString &deviceUuid, const QString &serviceId);

private:
    void addRootDevice(SsdpMessage message);
    UpnpRootDevice *getRootDeviceFromUuid(const QString &uuid);

signals:
    void newRootDevice(UpnpRootDevice *device);

    void subscribeEventing(QNetworkRequest request, const QString &uuid, const QString &serviceId);

    void removeSidEventFromUuid(const QString &deviceUuid);

public slots:
    void ssdpMessageReceived(const QHostAddress &host, const int &port, const SsdpMessage &message);

    void _rootDeviceStatusChanged();
    void _rootDeviceAvailableChanged();

private:
    QNetworkAccessManager *m_nam;
    QString m_macAddress;
};

#endif // DEVICESMODEL_H
