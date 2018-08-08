#ifndef UPNPACTIONREPLY_H
#define UPNPACTIONREPLY_H

#include <QObject>
#include <QNetworkReply>
#include "upnperror.h"
#include "soapactionresponse.h"

class UpnpActionReply : public QObject
{
    Q_OBJECT

public:
    explicit UpnpActionReply(QNetworkReply *reply);

    QNetworkReply::NetworkError error() const;
    QByteArray data() const;
    SoapActionResponse *response() const;

signals:
    void errorOccured(const UpnpError &error);
    void finished();

private slots:
    void replyReceived();
    void networkError(QNetworkReply::NetworkError error);
    void replyDestroyed();

private:
    QNetworkReply *m_reply = Q_NULLPTR;
    QNetworkReply::NetworkError m_error = QNetworkReply::NoError;
    QString m_actionName;
    SoapActionResponse *m_response = Q_NULLPTR;
};

#endif // UPNPACTIONREPLY_H
