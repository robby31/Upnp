#include "httpserver.h"

HttpServer::HttpServer(QObject *parent):
    QTcpServer(parent)
{
    DebugInfo::add_object(this);

    connect(this, SIGNAL(newConnection()), this, SLOT(newConnectionSlot()));

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &HttpServer::runRequestReply);
    timer->start(100);
}

HttpServer::~HttpServer()
{
    if (!m_incomingRequest.isEmpty())
        qCritical() << "DESTROY HttpServer" << m_incomingRequest.size() << "incoming requests pending.";

    QHash<qintptr, QList<HttpRequest*>>::iterator i;
    for (i = m_requestToReply.begin(); i != m_requestToReply.end(); ++i)
        qCritical() << "DESTROY HttpServer, socket" << i.key() << "," << i.value().size() << "requests to reply.";
}

void HttpServer::newConnectionSlot()
{
    while (hasPendingConnections())
    {
        QTcpSocket *clientConnection = nextPendingConnection();
        DebugInfo::add_object(clientConnection);
        connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(incomingData()));
        qDebug() << "new connection" << clientConnection;
    }
}

void HttpServer::incomingData()
{
    HttpRequest *request = Q_NULLPTR;

    auto clientConnection = qobject_cast<QTcpSocket*>(sender());
    while (clientConnection->bytesAvailable() > 0)
    {
        qintptr socket = clientConnection->socketDescriptor();
        if (m_incomingRequest.contains(socket))
        {
            qDebug() << clientConnection << "read request" << socket;
            request = m_incomingRequest[socket];
            if (!request->isFinished())
            {
                request->incomingData();
            }
            else
            {
                qCritical() << request << "incoming data but previous request not yet completed.";
                return;
            }
        }
        else
        {
            request = new HttpRequest(clientConnection, this);
            connect(request, &HttpRequest::headerSent, this, &HttpServer::requestHeaderSent);
            connect(request, &HttpRequest::closed, this, &HttpServer::requestHeaderSent);
            connect(request, &HttpRequest::destroyed, this, &HttpServer::requestHeaderSent);
            qDebug() << QThread::currentThread() << clientConnection << "new request" << socket << clientConnection->peerAddress().toString() << request;
            request->setDeviceUuid(deviceUuid());
            request->setServerName(m_serverName);
            request->incomingData();

            if (request->operation() != QNetworkAccessManager::UnknownOperation)
            {
                m_incomingRequest[socket] = request;
                emit newRequest(request);
            }
            else
            {
                // invalid operation
                request->deleteLater();
                return;
            }
        }

        if (request->isFinished())
        {
            qDebug() << clientConnection << "request finished" << request->socketDescriptor();
            m_incomingRequest.remove(request->socketDescriptor());
            addRequestToReply(request);
        }
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

void HttpServer::addRequestToReply(HttpRequest *request)
{
    if (request)
    {
        qintptr socket = request->socketDescriptor();
        if (!m_requestToReply.contains(socket))
        {
            QList<HttpRequest *> list;
            m_requestToReply[socket] = list;
        }

        m_requestToReply[socket].append(request);
    }
}

void HttpServer::runRequestReply()
{
    QHash<qintptr, QList<HttpRequest*>>::iterator i = m_requestToReply.begin();
    while (i != m_requestToReply.end())
    {
        if (!i.value().isEmpty())
        {
            HttpRequest *request = i.value().at(0);

            qDebug() << "run reply for request" << request << request->socketDescriptor() << i.value();

            emit requestCompleted(request);
            break;
        }

        i = m_requestToReply.erase(i);
    }
}

void HttpServer::requestHeaderSent()
{
    // reply is done or in progress

    auto request = qobject_cast<HttpRequest*>(sender());
    if (!request)
    {
        qCritical() << "invalid sender" << request;
    }
    else
    {
        if (!m_requestToReply.contains(request->socketDescriptor()))
        {
            bool requestFound = false;
            QHash<qintptr, QList<HttpRequest*>>::iterator i;
            for (i = m_requestToReply.begin(); i != m_requestToReply.end(); ++i)
            {
                if (i.value().contains(request))
                {
                    requestFound = true;
                    disconnect(request);
                    request->disconnect(this);
                    i.value().removeAll(request);
                    qDebug() << "remove request" << request << request->socketDescriptor();
                }
            }

            if (!requestFound)
                qWarning() << "request not found" << request->socketDescriptor() << request;
        }
        else if (!m_requestToReply[request->socketDescriptor()].contains(request))
        {
            qWarning() << "request not found" << request->socketDescriptor() << request;
        }
        else
        {
            qDebug() << "remove request" << request << request->socketDescriptor();
            disconnect(request);
            request->disconnect(this);
            m_requestToReply[request->socketDescriptor()].removeAll(request);
        }
    }
}
