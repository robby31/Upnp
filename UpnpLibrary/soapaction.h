#ifndef SOAPACTION_H
#define SOAPACTION_H

#include <QObject>
#include <QDebug>
#include <QHash>
#include <QDomDocument>

class SoapAction : public QObject
{
    Q_OBJECT
public:
    explicit SoapAction(QString serviceType, QString actionName, QObject *parent = 0);
    explicit SoapAction(QByteArray data, QObject *parent = 0);

    bool isValid() const;

    QString serviceType() const;
    QString actionName() const;
    QByteArray soapaction() const;

    bool addArgument(const QString &name, const QString &value);
    QString argumentValue(const QString &param) const;

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

#endif // SOAPACTION_H
