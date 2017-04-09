#include "eventresponse.h"

EventResponse::EventResponse(QByteArray data, QObject *parent):
    QObject(parent),
    m_valid(false)
{
    m_xml.setContent(data, true);

    QDomElement elt = m_xml.documentElement();
    if (elt.localName() == "propertyset" && elt.namespaceURI() == "urn:schemas-upnp-org:event-1-0")
    {
        QDomNodeList l_nodes = elt.childNodes();

        int childCorrect = 0;
        for (int i=0;i<l_nodes.size();++i)
        {
            if (l_nodes.at(i).localName() == "property")
                ++childCorrect;

            if (l_nodes.at(i).childNodes().size() != 1)
                break;

            QString name = l_nodes.at(i).firstChild().localName();
            QString value = l_nodes.at(i).firstChild().firstChild().nodeValue();
            if (!m_variables.contains(name))
                m_variables[name] = value;
            else
                break;
        }

        if (childCorrect == l_nodes.size())
            m_valid = true;
    }
}

bool EventResponse::isValid() const
{
    return m_valid;
}

QString EventResponse::toString() const
{
    return m_xml.toString(-1);
}

QStringList EventResponse::variablesName() const
{
    return m_variables.keys();
}

QString EventResponse::value(const QString &name) const
{
    if (m_variables.contains(name))
        return m_variables[name];
    else
        return QString();
}
