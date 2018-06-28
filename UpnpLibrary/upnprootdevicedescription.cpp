#include "upnprootdevicedescription.h"

UpnpRootDeviceDescription::UpnpRootDeviceDescription():
    UpnpDeviceDescription("root")
{
    m_xml.clear();

    QDomElement root = m_xml.root();
    root.setAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");
    root.setAttribute("configId", "0");

    QDomElement specVersion = m_xml.addParam("specVersion", root);
    m_xml.addParam("major", specVersion);
    m_xml.setParam("major", "1", specVersion);
    m_xml.addParam("minor", specVersion);
    m_xml.setParam("minor", "0", specVersion);

    QDomElement device = m_xml.addParam("device", root);
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

QDomElement UpnpRootDeviceDescription::device() const
{
    return m_xml.getParam("device", m_xml.root(), false);
}

QString UpnpRootDeviceDescription::version() const
{
    QDomElement specVersion = m_xml.getParam("specVersion", m_xml.root(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
        return QString("null");
    }
    else
    {
        return QString("%1.%2").arg(m_xml.getParamValue("major", specVersion)).arg(m_xml.getParamValue("minor", specVersion));
    }
}

void UpnpRootDeviceDescription::setVersion(const int &major, const int &minor)
{
    QDomElement specVersion = m_xml.getParam("specVersion", m_xml.root(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
    }
    else
    {
        m_xml.setParam("major", QVariant::fromValue(major).toString(), specVersion);
        m_xml.setParam("minor", QVariant::fromValue(minor).toString(), specVersion);
    }
}

QString UpnpRootDeviceDescription::configId() const
{
    return m_xml.root().attribute("configId");
}

void UpnpRootDeviceDescription::setConfigId(const QString &id)
{
    QDomElement root = m_xml.root();
    root.setAttribute("configId", id);
}

