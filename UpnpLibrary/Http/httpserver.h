#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "httprequest.h"
#include <QTimer>
#include "debuginfo.h"

class HttpServer : public QTcpServer
{
    Q_OBJECT

    Q_DISABLE_COPY_MOVE(HttpServer)

public:
    explicit HttpServer(QObject *parent = Q_NULLPTR);
    ~HttpServer() Q_DECL_OVERRIDE;

    QString serverName() const;
    void setServerName(const QString &name);

    QString deviceUuid() const;
    void setDeviceUuid(const QString &uuid);

private:
    void addRequestToReply(HttpRequest *request);

signals:
    void newRequest(HttpRequest *request);
    void requestCompleted(HttpRequest *request);

private slots:
    void newConnectionSlot();
    void incomingData();
    void runRequestReply();
    void requestHeaderSent();

private:
    QString m_serverName;
    QHash<qintptr, HttpRequest*> m_incomingRequest;
    QHash<qintptr, QList<HttpRequest*>> m_requestToReply;
    QString m_uuid;
};

#endif // HTTPSERVER_H
