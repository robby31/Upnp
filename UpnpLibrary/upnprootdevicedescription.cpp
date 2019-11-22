#include "upnprootdevicedescription.h"

UpnpRootDeviceDescription::UpnpRootDeviceDescription():
    UpnpDeviceDescription("root")
{
    clear();

    QDomElement root = xmlDescription();
    root.setAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");
    root.setAttribute("configId", "0");

    QDomElement specVersion = addParam("specVersion", root);
    addParam("major", specVersion);
    setParam("major", "1", specVersion);
    addParam("minor", specVersion);
    setParam("minor", "0", specVersion);

    QDomElement device = addParam("device", root);
    addParam("deviceType", device);
    addParam("friendlyName", device);
    addParam("manufacturer", device);
    addParam("manufacturerURL", device);
    addParam("modelDescription", device);
    addParam("modelName", device);
    addParam("modelNumber", device);
    addParam("modelURL", device);
    addParam("serialNumber", device);
    addParam("UDN", device);
    addParam("UPC", device);
    addParam("iconList", device);
    addParam("serviceList", device);
    addParam("deviceList", device);
    addParam("presentationURL", device);
}

QDomElement UpnpRootDeviceDescription::device() const
{
    return getParam("device", xmlDescription(), false);
}

QString UpnpRootDeviceDescription::version() const
{
    QDomElement specVersion = getParam("specVersion", xmlDescription(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
        return QString("null");
    }

    return QString("%1.%2").arg(getParamValue("major", specVersion), getParamValue("minor", specVersion));
}

void UpnpRootDeviceDescription::setVersion(const int &major, const int &minor)
{
    QDomElement specVersion = getParam("specVersion", xmlDescription(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
    }
    else
    {
        setParam("major", QVariant::fromValue(major).toString(), specVersion);
        setParam("minor", QVariant::fromValue(minor).toString(), specVersion);
    }
}

QString UpnpRootDeviceDescription::configId() const
{
    return xmlDescription().attribute("configId");
}

void UpnpRootDeviceDescription::setConfigId(const QString &id)
{
    QDomElement root = xmlDescription();
    root.setAttribute("configId", id);
}

