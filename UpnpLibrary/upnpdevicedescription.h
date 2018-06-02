#ifndef UPNPDEVICEDESCRIPTION_H
#define UPNPDEVICEDESCRIPTION_H

#include "upnpdescription.h"

class UpnpDeviceDescription : public UpnpDescription
{

public:
    UpnpDeviceDescription(QString rootName = "device");

    virtual QDomElement device() const;

    QString deviceAttribute(const QString &name) const;
    void setDeviceAttribute(const QString &name, const QString &value);

    void addIcon(const QString &mimeType, const int &width, const int &height, const int &depth, const QString &url);
    QString iconUrl();

    QList<QDomElement> devices();
    QList<QDomElement> services();
};

#endif // UPNPDEVICEDESCRIPTION_H
