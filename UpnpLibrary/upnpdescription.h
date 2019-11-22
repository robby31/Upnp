#ifndef UPNPDESCRIPTION_H
#define UPNPDESCRIPTION_H

#include "xmldescription.h"
#include "debuginfo.h"

class UpnpDescription : public QObject
{
    Q_OBJECT

public:
    explicit UpnpDescription(const QString& rootName = "device", QObject *parent = Q_NULLPTR);

    XmlDescription data() const { return m_xml; }

    void clear();

    bool setContent(const QByteArray &data);
    bool setContent(const QDomNode &data);

    QString attribute(const QString &name) const;

    QDomElement xmlDescription() const;
    QString stringDescription() const;

    QDomElement addParam(const QString &param, const QDomElement &parent);

    QDomElement getParam(const QString &param, const QDomElement &parent = QDomElement(), bool recursive = true) const;
    void setParam(const QString &param, const QString &value, const QDomElement &parent = QDomElement());
    void setParam(const QDomElement &param, const QString &value);
    QString getParamValue(const QString &param, const QDomElement &parent = QDomElement()) const;

private:
    XmlDescription m_xml;
};

#endif // UPNPDESCRIPTION_H
