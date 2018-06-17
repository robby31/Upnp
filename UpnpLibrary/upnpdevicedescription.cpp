#include "upnpdevicedescription.h"

UpnpDeviceDescription::UpnpDeviceDescription(QString rootName):
    UpnpDescription(rootName)
{
    QDomElement device = m_xml.root();

    m_xml.addParam("deviceType", device);
    m_xml.addParam("friendlyName", device);
    m_xml.addParam("manufacturer", device);
    m_xml.addParam("manufacturerURL", device);
    m_xml.addParam("modelDescription", device);
    m_xml.addParam("modelName", device);
    m_xml.addParam("modelNumber", device);
    m_xml.addParam("modelURL", device);
    m_xml.addParam("serialNumber", device);
    m_xml.addParam("UDN", device);
    m_xml.addParam("UPC", device);
    m_xml.addParam("iconList", device);
    m_xml.addParam("serviceList", device);
    m_xml.addParam("deviceList", device);
    m_xml.addParam("presentationURL", device);
}

QDomElement UpnpDeviceDescription::device() const
{
    return m_xml.root();
}

QString UpnpDeviceDescription::deviceAttribute(const QString &name) const
{
    return m_xml.getParamValue(name, device());
}

void UpnpDeviceDescription::setDeviceAttribute(const QString &name, const QString &value)
{
    m_xml.setParam(name, value, device());
}

void UpnpDeviceDescription::addIcon(const QString &mimeType, const int &width, const int &height, const int &depth, const QString &url)
{
    QDomElement iconList = m_xml.getParam("iconList", device(), false);
    if (iconList.isNull())
    {
        qCritical() << "unable to find iconList";
    }
    else
    {
        QDomElement icon = m_xml.addParam("icon", iconList);
        QDomElement elt = m_xml.addParam("mimetype", icon);
        m_xml.setParam("mimetype", mimeType, icon);

        elt = m_xml.addParam("width", icon);
        m_xml.setParam("width", QVariant::fromValue(width).toString(), icon);

        elt = m_xml.addParam("height", icon);
        m_xml.setParam("height", QVariant::fromValue(height).toString(), icon);

        elt = m_xml.addParam("depth", icon);
        m_xml.setParam("depth", QVariant::fromValue(depth).toString(), icon);

        elt = m_xml.addParam("url", icon);
        m_xml.setParam("url", url, icon);
    }

}

QString UpnpDeviceDescription::iconUrl()
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
    else
    {
        return QString();
    }
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
        UpnpServiceDescription *desc = (UpnpServiceDescription*)service->description();
        if (desc)
        {
            QDomNode node = services.appendChild(desc->xmlInfo().cloneNode(true));
            return !node.isNull();
        }
        else
        {
            qCritical() << "invalid service description" << service->description();
            return false;
        }
    }
    else
    {
        return false;
    }
}
