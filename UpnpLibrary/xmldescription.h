#ifndef XMLDESCRIPTION_H
#define XMLDESCRIPTION_H

#include <QDomDocument>
#include <QDebug>

class XmlDescription
{

public:
    XmlDescription(const QString &rootName = QString());

    bool setContent(const QByteArray &data);
    bool setContent(const QDomNode &data);
    void clear();

    QDomElement root() const;

    QDomElement addParam(const QString &param, QDomElement parent);

    QDomElement getParam(const QString &param, const QDomElement &parent = QDomElement(), bool recursive = true) const;
    void setParam(const QString &param, const QString &value, const QDomElement &parent = QDomElement());
    void setParam(QDomElement param, const QString &value);
    QString getParamValue(const QString &param, const QDomElement &parent = QDomElement()) const;

    QString toString() const;

private:
    QDomDocument m_xml;
};

#endif // XMLDESCRIPTION_H
