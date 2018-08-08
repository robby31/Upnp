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
    HttpRequest *request = Q_NULLPTR;

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
            qDebug() << "new request" << socket << clientConnection->peerAddress().toString();
            request = new HttpRequest(clientConnection, this);
            request->setDeviceUuid(deviceUuid());
            request->setServerName(m_serverName);
            request->incomingData();

            if (request->operation() != QNetworkAccessManager::UnknownOperation)
            {
                m_request[socket] = request;
                emit newRequest(request);
            }
            else
            {
                // invalid operation
                request->deleteLater();
            }
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
    {
        qCritical() << clientConnection->socketDescriptor() << "data are available :" << clientConnection->bytesAvailable() << "bytes.";
        qDebug() << clientConnection->readAll();
    }
}

QString HttpServer::deviceUuid() const
{
    return m_uuid;
}

void HttpServer::setDeviceUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString HttpServer::serverName() const
{
    return m_serverName;
}

void HttpServer::setServerName(const QString &name)
{
    m_serverName = name;
}
