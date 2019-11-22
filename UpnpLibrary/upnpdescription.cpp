#include "upnpdescription.h"

UpnpDescription::UpnpDescription(const QString &rootName, QObject *parent):
    QObject(parent),
    m_xml(rootName)
{
    DebugInfo::add_object(this);
}

bool UpnpDescription::setContent(const QByteArray &data)
{
    return m_xml.setContent(data);
}

bool UpnpDescription::setContent(const QDomNode &data)
{
    return m_xml.setContent(data.cloneNode(true));
}

QDomElement UpnpDescription::xmlDescription() const
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

QDomElement UpnpDescription::addParam(const QString &param, const QDomElement &parent)
{
    return m_xml.addParam(param, parent);
}

QDomElement UpnpDescription::getParam(const QString &param, const QDomElement &parent, bool recursive) const
{
    return m_xml.getParam(param, parent, recursive);
}

void UpnpDescription::setParam(const QString &param, const QString &value, const QDomElement &parent)
{
    m_xml.setParam(param, value, parent);
}

void UpnpDescription::setParam(const QDomElement &param, const QString &value)
{
    m_xml.setParam(param, value);
}

QString UpnpDescription::getParamValue(const QString &param, const QDomElement &parent) const
{
    return m_xml.getParamValue(param, parent);
}

void UpnpDescription::clear()
{
    m_xml.clear();
}
