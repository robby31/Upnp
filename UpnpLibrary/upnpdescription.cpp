#include "upnpdescription.h"

UpnpDescription::UpnpDescription(const QString &rootName, QObject *parent):
    QObject(parent),
    m_xml(rootName)
{

}

bool UpnpDescription::setContent(const QByteArray &data)
{
    return m_xml.setContent(data);
}

bool UpnpDescription::setContent(const QDomNode &data)
{
    return m_xml.setContent(data.cloneNode(true));
}

QDomElement UpnpDescription::xmlDescription()
{
    return m_xml.root();
}

QString UpnpDescription::stringDescription() const
{
    return m_xml.toString();
}

QString UpnpDescription::attribute(const QString &name) const
{
    return m_xml.getParamValue(name, m_xml.root());
}
