#include "httpserver.h"

HttpServer::HttpServer(QObject *parent):
    QTcpServer(parent)
{
    connect(this, SIGNAL(newConnection()), this, SLOT(newConnectionSlot()));
}

HttpServer::~HttpServer()
{
    if (m_request.size() != 0)
        qCritical() << "DESTROY HttpServer" << m_request.size() << "requests pending.";
}

void HttpServer::newConnectionSlot()
{
    while (hasPendingConnections())
    {
        QTcpSocket *clientConnection = nextPendingConnection();
        connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingData()));
        qDebug() << "new connection" << clientConnection;
    }
}

void HttpServer::incomingData()
{
    HttpRequest *request = 0;

    QTcpSocket *clientConnection = qobject_cast<QTcpSocket*>(sender());

    while (clientConnection->bytesAvailable() > 0)
    {
        qintptr socket = clientConnection->socketDescriptor();
        if (m_request.contains(socket))
        {
            qDebug() << "read request" << socket;
            request = m_request[socket];
            request->incomingData();
        }
        else
        {
            qDebug() << "new request" << socket;
            request = new HttpRequest(clientConnection, this);
            request->setDeviceUuid(deviceUuid());
            m_request[socket] = request;
            emit newRequest(request);

            request->incomingData();
        }

        if (request->isFinished())
        {
            qDebug() << "request finished" << request->socketDescriptor();
            m_request.remove(request->socketDescriptor());
            emit requestCompleted(request);
        }
        else
        {
            break;
        }
    }

    if (clientConnection->bytesAvailable() > 0)
        qCritical() << clientConnection->socketDescriptor() << "data are available :" << clientConnection->bytesAvailable() << "bytes.";
}

QString HttpServer::deviceUuid() const
{
    return m_uuid;
}

void HttpServer::setDeviceUuid(const QString &uuid)
{
    m_uuid = uuid;
}
