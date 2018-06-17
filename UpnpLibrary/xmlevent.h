#ifndef XMLEVENT_H
#define XMLEVENT_H

#include <QDomDocument>
#include <QDebug>

class XmlEvent
{

public:
    XmlEvent();

    QDomElement addProperty(const QString &name, const QString &value);

    QString toString() const;

private:
    QDomDocument m_xml;
};

#endif // XMLEVENT_H
