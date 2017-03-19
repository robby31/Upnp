#include "didllite.h"

DidlLite::DidlLite():
    QDomDocument()
{
    m_didl = createElementNS("urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/", "DIDL-Lite");
    m_didl.setAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_didl.setAttribute("xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
    appendChild(m_didl);
}

QDomNode DidlLite::addElement(QDomNode node)
{
    return m_didl.appendChild(node);
}
