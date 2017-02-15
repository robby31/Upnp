#ifndef UPNPDEVICE_H
#define UPNPDEVICE_H

#include <QObject>
#include <QDateTime>
#include "upnpobject.h"

class UpnpDevice : public UpnpObject
{
    Q_OBJECT

    enum Roles {
        HostRole = Qt::UserRole+1,
        DeviceTypeRole,
        AvailableRole
    };

public:
    explicit UpnpDevice(QObject *parent = 0);
    explicit UpnpDevice(QHostAddress host, QDomNode info, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString getInfo(const QString &param) const;

private:
    void initRoles();

signals:

public slots:

private:
    QDomNode m_info;
    QHostAddress m_host;
};

#endif // UPNPDEVICE_H
