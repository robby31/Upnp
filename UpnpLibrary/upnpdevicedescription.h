#ifndef UPNPDEVICEDESCRIPTION_H
#define UPNPDEVICEDESCRIPTION_H

#include "upnpdescription.h"
#include "upnpservice.h"

class UpnpDeviceDescription : public UpnpDescription
{

    Q_OBJECT

public:
    explicit UpnpDeviceDescription(const QString &rootName = "device", QObject *parent = Q_NULLPTR);

    virtual QDomElement device() const;

    QString deviceAttribute(const QString &name) const;
    void setDeviceAttribute(const QString &name, const QString &value);

    void addIcon(const QString &mimeType, const int &width, const int &height, const int &depth, const QString &url);
    QString iconUrl() const;
    QStringList iconUrls() const;

    bool addService(UpnpService *service);

    QList<QDomElement> devices();
    QList<QDomElement> services();
};

#endif // UPNPDEVICEDESCRIPTION_H
