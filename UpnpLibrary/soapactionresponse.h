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

    Q_PROPERTY(QStringList arguments READ arguments NOTIFY argumentsChanged)

public:
    explicit SoapActionResponse(QString serviceType, QString actionName, QObject *parent = Q_NULLPTR);
    explicit SoapActionResponse(QByteArray data, QObject *parent = Q_NULLPTR);

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
