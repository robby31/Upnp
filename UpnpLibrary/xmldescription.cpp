#include "xmldescription.h"

XmlDescription::XmlDescription(const QString &rootName)
{
    m_xml.appendChild(m_xml.createProcessingInstruction("xml", "version=\"1.0\""));
    m_xml.appendChild(m_xml.createElement(rootName));
}

bool XmlDescription::setContent(const QByteArray &data)
{
    return m_xml.setContent(data, true);
}

bool XmlDescription::setContent(const QDomNode &data)
{
    m_xml.clear();
    m_xml.appendChild(m_xml.createProcessingInstruction("xml", "version=\"1.0\""));
    return !m_xml.appendChild(data).isNull();
}

void XmlDescription::clear()
{
    QString rootName = m_xml.childNodes().at(1).toElement().tagName();

    m_xml.clear();

    m_xml.appendChild(m_xml.createProcessingInstruction("xml", "version=\"1.0\""));
    m_xml.appendChild(m_xml.createElement(rootName));
}

QDomElement XmlDescription::root() const
{
    return m_xml.firstChildElement();
}

QDomElement XmlDescription::addParam(const QString &param, QDomElement parent)
{
    QDomElement elt = m_xml.createElement(param);
    parent.appendChild(elt);
    return elt;
}

QDomElement XmlDescription::getParam(const QString &param, const QDomElement &parent, bool recursive) const
{
    QDomNodeList nodes;
    if (parent.isNull())
        nodes = m_xml.elementsByTagName(param);
    else
        nodes = parent.elementsByTagName(param);

    int indexFound = -1;
    if (!recursive)
    {
        for (int index=0;index<nodes.size();++index)
        {
            QDomNode node = nodes.at(index);
            if (node.parentNode() == parent)
            {
                if (indexFound == -1)
                {
                    indexFound = index;
                }
                else
                {
                    indexFound = -1;
                    break;
                }
            }
        }
    }

    if (indexFound != -1)
        return nodes.at(indexFound).toElement();

    if (nodes.size() == 1)
        return nodes.at(0).toElement();

    qCritical() << nodes.size() << "nodes" << param << "found below" << parent.tagName();
    return QDomElement();
}

QString XmlDescription::getParamValue(const QString &param, const QDomElement &parent) const
{
    QDomElement elt = getParam(param, parent, false);
    return elt.firstChild().nodeValue();
}

void XmlDescription::setParam(const QString &param, const QString &value, const QDomElement &parent)
{
    QDomNode node = getParam(param, parent, false);
    if (!node.isNull())
    {
        if (node.firstChild().isNull())
            node.appendChild(m_xml.createTextNode(value));
        else
            node.firstChild().setNodeValue(value);
    }
    else
    {
        qCritical() << "unable to find node" << param;
    }
}

void XmlDescription::setParam(QDomElement param, const QString &value)
{
    if (param.firstChild().isNull())
        param.appendChild(m_xml.createTextNode(value));
    else
        param.firstChild().setNodeValue(value);
}

QString XmlDescription::toString() const
{
    return m_xml.toString();
}
