#ifndef UPNPDESCRIPTION_H
#define UPNPDESCRIPTION_H

#include "xmldescription.h"

class UpnpDescription : public QObject
{
    Q_OBJECT

public:
    explicit UpnpDescription(QString rootName = "device", QObject *parent = 0);

    bool setContent(const QByteArray &data);
    bool setContent(const QDomNode &data);

    QString attribute(const QString &name) const;

    QDomElement xmlDescription();
    QString stringDescription() const;

protected:
    XmlDescription m_xml;
};

#endif // UPNPDESCRIPTION_H
