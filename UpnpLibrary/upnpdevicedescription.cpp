#include "upnpdevicedescription.h"

UpnpDeviceDescription::UpnpDeviceDescription(const QString &rootName):
    UpnpDescription(rootName)
{
    QDomElement device = xmlDescription();

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

QDomElement UpnpDeviceDescription::device() const
{
    return xmlDescription();
}

QString UpnpDeviceDescription::deviceAttribute(const QString &name) const
{
    return getParamValue(name, device());
}

void UpnpDeviceDescription::setDeviceAttribute(const QString &name, const QString &value)
{
    setParam(name, value, device());
}

void UpnpDeviceDescription::addIcon(const QString &mimeType, const int &width, const int &height, const int &depth, const QString &url)
{
    QDomElement iconList = getParam("iconList", device(), false);
    if (iconList.isNull())
    {
        qCritical() << "unable to find iconList";
    }
    else
    {
        QDomElement icon = addParam("icon", iconList);
        QDomElement elt = addParam("mimetype", icon);
        setParam("mimetype", mimeType, icon);

        elt = addParam("width", icon);
        setParam("width", QVariant::fromValue(width).toString(), icon);

        elt = addParam("height", icon);
        setParam("height", QVariant::fromValue(height).toString(), icon);

        elt = addParam("depth", icon);
        setParam("depth", QVariant::fromValue(depth).toString(), icon);

        elt = addParam("url", icon);
        setParam("url", url, icon);
    }

}

QString UpnpDeviceDescription::iconUrl() const
{
    QDomElement l_icon = device().firstChildElement("iconList");
    if (!l_icon.isNull())
    {
        QString iconUrl;
        int iconWidth = -1;
        QString iconMimeType;

        QDomNode icon = l_icon.firstChild();
        while (!icon.isNull())
        {
            int width = icon.firstChildElement("width").firstChild().nodeValue().toInt();
            QString mimetype = icon.firstChildElement("mimetype").firstChild().nodeValue();
            if (width > iconWidth)
            {
                // select larger icon
                iconWidth = width;
                iconUrl = icon.firstChildElement("url").firstChild().nodeValue();
                iconMimeType = mimetype;
            }

            icon = icon.nextSibling();
        }

        return iconUrl;
    }

    return QString();
}

QStringList UpnpDeviceDescription::iconUrls() const
{
    QStringList res;

    QDomElement l_icon = device().firstChildElement("iconList");
    if (!l_icon.isNull())
    {
        QDomNode icon = l_icon.firstChild();
        while (!icon.isNull())
        {
            res << icon.firstChildElement("url").firstChild().nodeValue();
            icon = icon.nextSibling();
        }
    }

    return res;
}

QList<QDomElement> UpnpDeviceDescription::devices()
{
    QList<QDomElement> res;

    QDomElement devices = device().firstChildElement("deviceList");
    if (!devices.isNull())
    {
        QDomNode deviceElt = devices.firstChild();
        while (!deviceElt.isNull())
        {
            res << deviceElt.toElement();
            deviceElt = deviceElt.nextSibling();
        }
    }

    return res;
}

QList<QDomElement> UpnpDeviceDescription::services()
{
    QList<QDomElement> res;

    QDomElement services = device().firstChildElement("serviceList");
    if (!services.isNull())
    {
        QDomNode serviceElt = services.firstChild();
        while (!serviceElt.isNull())
        {
            res << serviceElt.toElement();
            serviceElt = serviceElt.nextSibling();
        }
    }

    return res;
}

bool UpnpDeviceDescription::addService(UpnpService *service)
{

    QDomElement services = device().firstChildElement("serviceList");
    if (!services.isNull())
    {
        auto desc = qobject_cast<UpnpServiceDescription*>(service->description());
        if (desc)
        {
            QDomNode node = services.appendChild(desc->xmlInfo().cloneNode(true));
            return !node.isNull();
        }

        qCritical() << "invalid service description" << service->description();
        return false;
    }

    return false;
}
