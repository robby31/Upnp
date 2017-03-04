#ifndef UPNPERROR_H
#define UPNPERROR_H

#include <QObject>
#include <QDebug>
#include <QDomDocument>
#include <QNetworkReply>

class UpnpError : public QObject
{
    Q_OBJECT

public:
    explicit UpnpError(QNetworkReply::NetworkError netError, QByteArray data, QObject *parent = 0);

    QNetworkReply::NetworkError netError() const;

    QString faultCode() const;
    QString faultString() const;

    int code() const;
    QString description() const;

signals:

public slots:

private:
    QNetworkReply::NetworkError m_netError;
    QDomDocument m_data;
    QDomElement m_fault;
    QDomElement m_detail;
};

#endif // UPNPERROR_H
