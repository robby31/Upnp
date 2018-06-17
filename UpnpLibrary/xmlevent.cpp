#include "xmlevent.h"

XmlEvent::XmlEvent()
{
    m_xml.appendChild(m_xml.createProcessingInstruction("xml", "version=\"1.0\""));
    m_xml.appendChild(m_xml.createElementNS("urn:schemas-upnp-org:event-1-0", "e:propertyset"));
}

QDomElement XmlEvent::addProperty(const QString &name, const QString &value)
{
    QDomElement propertySet = m_xml.firstChildElement("propertyset");
    QDomElement property = m_xml.createElement("e:property");
    propertySet.appendChild(property);
    QDomElement param = m_xml.createElement(name);
    param.appendChild(m_xml.createTextNode(value));
    property.appendChild(param);
    return property;
}

QString XmlEvent::toString() const
{
    return m_xml.toString();
}
