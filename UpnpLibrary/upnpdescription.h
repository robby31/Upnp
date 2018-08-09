#ifndef UPNPDESCRIPTION_H
#define UPNPDESCRIPTION_H

#include "xmldescription.h"

class UpnpDescription : public QObject
{
    Q_OBJECT

public:
    explicit UpnpDescription(const QString& rootName = "device", QObject *parent = Q_NULLPTR);

    bool setContent(const QByteArray &data);
    bool setContent(const QDomNode &data);

    QString attribute(const QString &name) const;

    QDomElement xmlDescription();
    QString stringDescription() const;

protected:
    XmlDescription m_xml;
};

#endif // UPNPDESCRIPTION_H
