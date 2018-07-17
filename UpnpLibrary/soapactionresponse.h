#ifndef SOAPACTIONRESPONSE_H
#define SOAPACTIONRESPONSE_H

#include <QObject>
#include <QHash>
#include <QDebug>
#include <QDomDocument>
#include <QRegularExpressionMatch>

class SoapActionResponse : public QObject
{
    Q_OBJECT

public:
    explicit SoapActionResponse(QString serviceType, QString actionName, QObject *parent = 0);
    explicit SoapActionResponse(QByteArray data, QObject *parent = 0);

    bool isValid() const;

    QString serviceType() const;
    QString actionName() const;

    bool addArgument(const QString &name, const QString &value);
    QStringList arguments() const;
    QString value(const QString &argumentName) const;

    QByteArray toByteArray() const;

signals:

public slots:

private:
    bool m_valid;
    QString m_serviceType;
    QString m_actionName;
    QDomDocument m_xml;
    QDomElement m_xmlAction;
};

#endif // SOAPACTIONRESPONSE_H
