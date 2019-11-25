#ifndef UPNPROOTDEVICEDESCRIPTION_H
#define UPNPROOTDEVICEDESCRIPTION_H

#include "upnpdevicedescription.h"

class UpnpRootDeviceDescription : public UpnpDeviceDescription
{
    Q_OBJECT

public:
    explicit UpnpRootDeviceDescription(QObject *parent = Q_NULLPTR);

    QDomElement device() const Q_DECL_OVERRIDE;

    QString configId() const;
    void setConfigId(const QString &id);

    QString version() const;
    void setVersion(const int &major, const int &minor);
};

#endif // UPNPROOTDEVICEDESCRIPTION_H
