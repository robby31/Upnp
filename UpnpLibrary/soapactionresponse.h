#ifndef SOAPACTIONRESPONSE_H
#define SOAPACTIONRESPONSE_H

#include <QObject>
#include <QHash>
#include <QDebug>
#include <QDomDocument>
#include <QRegularExpressionMatch>
#include "debuginfo.h"

class SoapActionResponse : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList arguments READ arguments NOTIFY argumentsChanged)

public:
    explicit SoapActionResponse(const QString& serviceType, const QString& actionName, QObject *parent = Q_NULLPTR);
    explicit SoapActionResponse(const QByteArray& data, QObject *parent = Q_NULLPTR);
    ~SoapActionResponse() Q_DECL_OVERRIDE;

    bool isValid() const;

    QString serviceType() const;
    QString actionName() const;

    bool addArgument(const QString &name, const QString &value);
    QStringList arguments() const;
    Q_INVOKABLE QString value(const QString &argumentName) const;

    QByteArray toByteArray() const;

signals:
    void argumentsChanged();

public slots:

private:
    bool m_valid;
    QString m_serviceType;
    QString m_actionName;
    QDomDocument m_xml;
    QDomElement m_xmlAction;
};

#endif // SOAPACTIONRESPONSE_H
