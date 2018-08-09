#include "upnpactionreply.h"

UpnpActionReply::UpnpActionReply(QNetworkReply *reply):
    QObject(reply),
    m_reply(reply)
{
    connect(reply, &QNetworkReply::finished, this, &UpnpActionReply::replyReceived);
    connect(reply, &QNetworkReply::destroyed, this, &UpnpActionReply::replyDestroyed);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));
}

void UpnpActionReply::networkError(QNetworkReply::NetworkError error)
{
    m_error = error;

    auto reply = qobject_cast<QNetworkReply*>(sender());

    UpnpError upnpError(error, reply->readAll());
    emit errorOccured(upnpError);

    qCritical() << "Network Error" << upnpError.netError() << reply->request().url() << upnpError.code() << upnpError.description();

    reply->deleteLater();

    emit finished();
}

void UpnpActionReply::replyReceived()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());

    m_error = reply->error();

    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "action done" << this << reply->request().rawHeader("SOAPACTION");
        m_actionName = reply->request().rawHeader("SOAPACTION");
        m_response = new SoapActionResponse(reply->readAll(), this);
        emit responseChanged();
        emit finished();
    }
    else
    {
        qCritical() << reply->errorString();
    }

    reply->deleteLater();
}

void UpnpActionReply::replyDestroyed()
{
    m_reply = Q_NULLPTR;
}

QByteArray UpnpActionReply::data() const
{
    if (m_response)
        return m_response->toByteArray();

    return QByteArray();
}

SoapActionResponse *UpnpActionReply::response() const
{
    return m_response;
}
