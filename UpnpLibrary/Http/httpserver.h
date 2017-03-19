#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "httprequest.h"

class HttpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit HttpServer(QObject *parent = 0);
    virtual ~HttpServer() Q_DECL_OVERRIDE;

    QString deviceUuid() const;
    void setDeviceUuid(const QString &uuid);

signals:
    void newRequest(HttpRequest *request);
    void requestCompleted(HttpRequest *request);

private slots:
    void newConnectionSlot();
    void incomingData();

private:
    QHash<qintptr, HttpRequest*> m_request;
    QString m_uuid;
};

#endif // HTTPSERVER_H
