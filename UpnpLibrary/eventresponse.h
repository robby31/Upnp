#ifndef EVENTRESPONSE_H
#define EVENTRESPONSE_H

#include <QObject>
#include <QDebug>
#include <QDomDocument>

class EventResponse : public QObject
{

public:
    explicit EventResponse(const QByteArray& data, QObject *parent = Q_NULLPTR);

    bool isValid() const;

    QString toString() const;

    QHash<QString,QString> variables() const;
    QStringList variablesName() const;
    QString value(const QString &name) const;

private:
    bool m_valid;
    QDomDocument m_xml;
    QHash<QString,QString> m_variables;

};

#endif // EVENTRESPONSE_H
