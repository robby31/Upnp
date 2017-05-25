#include "httprequest.h"

const int HttpRequest::STREAMING_PERIOD = 1000;

HttpRequest::HttpRequest(QObject *parent):
    ListItem(parent),
    m_date(QDateTime::currentDateTime()),
    m_peerAddress(),
    m_client(Q_NULLPTR),
    m_request(),
    m_operation(QNetworkAccessManager::UnknownOperation),
    m_version(),
    m_headerCompleted(false),
    m_data(),
    m_replyHeaderSent(false),
    m_status("init"),
    m_networkStatus("init"),
    m_finished(false),
    m_streamWithErrors(false),
    m_streamingCompleted(false),
    m_maxBufferSize(-1),
    networkBytesSent(-1),
    lastNetBytesSent(-1),
    netStatusTimerEvent(0)
{
    initializeRoles();
}

HttpRequest::HttpRequest(QTcpSocket *client, QObject *parent):
    ListItem(parent),
    m_date(QDateTime::currentDateTime()),
    m_peerAddress(client->peerAddress()),
    m_client(client),
    m_request(),
    m_operation(QNetworkAccessManager::UnknownOperation),
    m_version(),
    m_headerCompleted(false),
    m_data(),
    m_replyHeaderSent(false),
    m_status("init"),
    m_networkStatus("init"),
    m_finished(false),
    m_streamWithErrors(false),
    m_streamingCompleted(false),
    m_maxBufferSize(-1),
    networkBytesSent(-1),
    lastNetBytesSent(-1),
    netStatusTimerEvent(0)
{    
    initializeRoles();

    if (m_client)
    {
        logMessage(QString("request created from %1, socket %2.").arg(m_peerAddress.toString()).arg(m_client->socketDescriptor()));

        connect(m_client, SIGNAL(destroyed(QObject*)), this, SLOT(clientDestroyed()));
        connect(m_client, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
        connect(m_client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(clientError(QAbstractSocket::SocketError)));
        connect(m_client, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));

        if (m_client->state() == QAbstractSocket::ConnectedState)
            setData("connected", networkStatusRole);
    }
    else
    {
        setError(QString("invalid client"));
    }
}

void HttpRequest::initializeRoles()
{
    m_roles[operationRole] = "operation";
    m_roles[urlRole] = "url";
    m_roles[hostRole] = "host";
    m_roles[peerAddressRole] = "peerAddress";
    m_roles[statusRole] = "status";
    m_roles[headerRole] = "header";
    m_roles[contentRole] = "content";
    m_roles[durationRole] = "duration";
    m_roles[dateRole] = "date";
    m_roles[closeDateRole] = "closeDate";
    m_roles[answerRole] = "answer";
    m_roles[networkStatusRole] = "network_status";
    m_roles[streamingStatusRole] = "streaming_status";
    m_roles[transcodeLogRole] = "transcode_log";
}

void HttpRequest::incomingData()
{
    if (m_client)
    {
        // read header
        while (!m_headerCompleted && m_client->canReadLine())
        {
            QString data = m_client->readLine().trimmed();
            qDebug() << "incomingData" << data;

            if (m_operation == QNetworkAccessManager::UnknownOperation)
                readOperation(data);
            else if (data.isEmpty())
                headerReadFinished();
            else
                readHeader(data);
        }

        if (m_headerCompleted && m_client->bytesAvailable() > 0)
        {
            // read data
            int contentLength = m_request.header(QNetworkRequest::ContentLengthHeader).toInt();

            if (contentLength != 0)
            {
                if (m_data.size() < contentLength)
                {
                    m_data.append(m_client->read(contentLength - m_data.size()));
                }
                else
                {
                    QByteArray data = m_client->readAll();
                    setError(QString("invalid data received (size exceeded), size received : %1, length expected : %2, bytes received : %3.").arg(m_data.size()).arg(contentLength).arg(data.size()));
                    qCritical() << data;
                }

                if (m_data.size() == contentLength)
                    requestCompleted();
            }
        }
    }
    else
    {
        setError("unable to read data, client destroyed");
        close();
    }
}

void HttpRequest::readOperation(const QString &data)
{
    if (m_operation == QNetworkAccessManager::UnknownOperation)
    {
        QRegularExpression pattern("^(GET|PUT|POST|HEAD|SUBSCRIBE|NOTIFY)\\s+(\\S+)\\s+(HTTP\\S+)");
        QRegularExpressionMatch match = pattern.match(data);
        if (match.hasMatch())
        {
            m_header << data;

            setData(match.captured(1), operationRole);
            setData(match.captured(2), urlRole);
            m_version = match.captured(3);

            qDebug() << socketDescriptor() << "Read operation" << data << match.capturedTexts();
        }
        else
        {
            setError(QString("%3 %2, invalid operation : %1.").arg(data).arg(socketDescriptor()).arg(peerAddress().toString()));
        }
    }
    else
    {
        setError(QString("%2, operation %1 already read : %2.").arg(operationString()).arg(data).arg(socketDescriptor()));
    }
}

void HttpRequest::readHeader(const QString &data)
{
    QRegularExpression pattern("([^:]+):(.+)");
    QRegularExpressionMatch match = pattern.match(data);
    if (match.hasMatch())
    {
        m_header << data;

        QString param = match.captured(1);
        QString value = match.captured(2);
        m_request.setRawHeader(param.toUtf8(), value.toUtf8());

        qDebug() << socketDescriptor() << "Read header" << match.capturedTexts() << param << value;
    }
    else
    {
        setError(QString("%2, invalid header data : %1.").arg(data).arg(socketDescriptor()));
    }
}

void HttpRequest::headerReadFinished()
{
    if (m_headerCompleted)
    {
        setError(QString("%1, header already finished").arg(socketDescriptor()));
    }
    else
    {
        m_headerCompleted = true;
        logMessage("header fully read.");
        setData("header read", statusRole);

        int contentLength = m_request.header(QNetworkRequest::ContentLengthHeader).toInt();
        if (contentLength == 0)
            requestCompleted();
    }
}

QNetworkAccessManager::Operation HttpRequest::operation() const
{
    return m_operation;
}

QString HttpRequest::operationString() const
{
    switch (m_operation)
    {
    case QNetworkAccessManager::HeadOperation:
    {
        return "HEAD";
    }
    case QNetworkAccessManager::GetOperation:
    {
        return "GET";
    }
    case QNetworkAccessManager::PutOperation:
    {
        return "PUT";
    }
    case QNetworkAccessManager::PostOperation:
    {
        return "POST";
    }
    case QNetworkAccessManager::CustomOperation:
    {
        return m_customOperation;
    }
    default:
    {
        return "UNKNOWN";
    }
    }
}

QUrl HttpRequest::url() const
{
    return m_request.url();
}

QString HttpRequest::version() const
{
    return m_version;
}

QString HttpRequest::header(const QString &param) const
{
    return m_request.rawHeader(param.toUtf8()).trimmed();
}

QByteArray HttpRequest::requestData() const
{
    return m_data;
}

QTcpSocket *HttpRequest::tcpSocket() const
{
    return m_client;
}

QHostAddress HttpRequest::peerAddress() const
{
    return m_peerAddress;
}

quint16 HttpRequest::peerPort() const
{
    if (m_client)
        return m_client->peerPort();
    else
        return -1;
}

qintptr HttpRequest::socketDescriptor() const
{
    if (m_client)
        return m_client->socketDescriptor();
    else
        return -1;
}

QString HttpRequest::serverName() const
{
    return m_serverName;
}

void HttpRequest::setServerName(const QString &name)
{
    m_serverName = name;
}

QHash<int, QByteArray> HttpRequest::roleNames() const
{
    return m_roles;
}

QVariant HttpRequest::data(int role) const
{
    switch (role) {

    case operationRole:
    {
        return operationString();
    }

    case urlRole:
    {
        return m_request.url();
    }

    case hostRole:
    {
        return QString("%1 (%2)").arg(m_request.url().host()).arg(socketDescriptor());
    }

    case peerAddressRole:
    {
        return m_peerAddress.toString();
    }

    case statusRole:
    {
        return m_status;
    }

    case dateRole:
    {
        return m_date.toString("dd MMM yyyy hh:mm:ss,zzz");
    }

    case closeDateRole:
    {
        return m_closeDate.toString("dd MMM yyyy hh:mm:ss,zzz");
    }

    case networkStatusRole:
    {
        return m_networkStatus;
    }

    case streamingStatusRole:
    {
        return m_streamingStatus;
    }

    case durationRole:
    {
        if (isClosed())
            return QTime(0, 0).addMSecs(m_date.msecsTo(m_closeDate)).toString("hh:mm:ss.zzz");
        else
            return QString();
    }

    case headerRole:
    {
        return m_header.join("\r\n");
    }

    case contentRole:
    {
        return m_data;
    }

    case answerRole:
    {
        QString result;

        result = m_replyHeader.join("\r\n");

        result.append(QString("Data sent (%1 bytes)\r\n").arg(m_replyData.size()));
        result.append(m_replyData);

        return result;
    }

    case transcodeLogRole:
    {
        return m_log;
    }

    default:
    {
        return QVariant::Invalid;
    }
    }

    return QVariant::Invalid;
}

bool HttpRequest::setData(const QVariant &value, const int &role)
{
    QVector<int> roles;
    roles << role;

    if (roles.contains(urlRole))
        roles << hostRole;

    if (roles.contains(closeDateRole))
        roles << durationRole;

    switch(role)
    {
    case networkStatusRole:
    {
        if (value.toString() != m_networkStatus)
        {
            m_networkStatus = value.toString();
            emit itemChanged(roles);
            return true;
        }
        else
        {
            return false;
        }
    }

    case streamingStatusRole:
    {
        if (value.toString() != m_streamingStatus)
        {
            m_streamingStatus = value.toString();
            emit itemChanged(roles);
            return true;
        }
        else
        {
            return false;
        }
    }

    case statusRole:
    {
        if (value.toString() != m_status)
        {
            if (m_status != "KO")
            {
                m_status = value.toString();
                emit itemChanged(roles);
                return true;
            }
            else
            {
                qCritical() << "cannot update status when error occurs" << value.toString();
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    case urlRole:
    {
        if (value.toString() != m_request.url().toString())
        {
            m_request.setUrl(value.toString());
            emit itemChanged(roles);
            return true;
        }
        else
        {
            return false;
        }
    }

    case operationRole:
    {
        if (value.toString() == "GET")
        {
            m_operation = QNetworkAccessManager::GetOperation;
            emit itemChanged(roles);
            return true;
        }
        else if (value.toString() == "POST")
        {
            m_operation = QNetworkAccessManager::PostOperation;
            emit itemChanged(roles);
            return true;
        }
        else if (value.toString() == "PUT")
        {
            m_operation = QNetworkAccessManager::PutOperation;
            emit itemChanged(roles);
            return true;
        }
        else if (value.toString() == "HEAD")
        {
            m_operation = QNetworkAccessManager::HeadOperation;
            emit itemChanged(roles);
            return true;
        }
        else if (value.toString() == "SUBSCRIBE" || value.toString() == "NOTIFY")
        {
            m_operation = QNetworkAccessManager::CustomOperation;
            m_customOperation = value.toString();
            emit itemChanged(roles);
            return true;
        }
        else
        {
            setError(QString("invalid operation : %1.").arg(value.toString()));
            return false;
        }
    }

    case closeDateRole:
    {
        m_closeDate = value.toDateTime();
        emit itemChanged(roles);
        return true;
    }

    default:
    {
        qCritical() << "unable to set data" << value << role << this;
        return false;
    }
    }
}

void HttpRequest::clientDestroyed()
{
    m_client = Q_NULLPTR;
    setData("destroyed", networkStatusRole);
    logMessage("client destroyed.");

    if (!isClosed())
        close();
}

void HttpRequest::clientError(QAbstractSocket::SocketError error)
{
    QString msg;

    if (m_client)
        msg = QString("network error: %1.").arg(m_client->errorString());
    else
        msg = QString("network error : %1.").arg(error);

    logMessage(msg);
}

void HttpRequest::socketStateChanged(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::ClosingState)
    {
        setData("closing", networkStatusRole);
        logMessage(QString("network state changed : Closing."));
    }
    else if (state == QAbstractSocket::UnconnectedState)
    {
        setData("disconnected", networkStatusRole);
        logMessage(QString("network state changed : Disconnected."));

        logMessage(QString("network disonnected : %1 bytes sent").arg(networkBytesSent));

        if (m_client)
        {
            if (m_client->bytesAvailable() > 0)
            {
                logMessage(QString("client disconnected and remaining data to read (%1 bytes).").arg(m_client->bytesAvailable()));
                qCritical() << m_client->socketDescriptor() << "client disconnected and data not read" << m_client->bytesAvailable();
                qWarning() << m_client->readAll();
            }

            if (m_client->bytesToWrite() > 0)
            {
                m_streamingCompleted = false;
                logMessage(QString("client disconnected and remaining data to send (%1 bytes).").arg(m_client->bytesToWrite()));
            }
        }
    }
    else
    {
        logMessage(QString("network state changed : %1.").arg(state));
        qCritical() << "unknown socket state value" << state;
    }
}

bool HttpRequest::isClosed() const
{
    return m_closeDate.isValid();
}

void HttpRequest::close()
{
    if (thread() != QThread::currentThread())
        qWarning() << "HttpRequest::Close" << thread() << QThread::currentThread();

    if (!isClosed())
    {
        logMessage("close request.");

        if (netStatusTimerEvent != 0)
        {
            killTimer(netStatusTimerEvent);
            netStatusTimerEvent = 0;
        }

        if (m_client)
        {            
            if (m_version == "HTTP/1.0" || header("Connection").toLower() == "close")
            {
                logMessage("close socket.");
                m_client->disconnectFromHost();
            }
        }

        if (!m_requestedResource.isEmpty())
        {
            setData(QString(), streamingStatusRole);

            if (m_streamingCompleted && !m_streamWithErrors)
            {
                setData("Streaming finished.", statusRole);
                emit servingFinishedSignal(m_peerAddress.toString(), m_requestedResource, 0);
                logMessage(QString("Streaming finished."));
            }
            else
            {
                if (!m_streamingCompleted)
                    logMessage("Streaming not completed.");

                if (m_streamWithErrors)
                    logMessage("Streaming with errors.");

                setData("Streaming aborted.", statusRole);
                emit servingFinishedSignal(m_peerAddress.toString(), m_requestedResource, 1);
                logMessage(QString("Streaming aborted."));
            }

            if (clockSending.pausedCounter() > 0)
                qWarning() << "low speed network, network paused" << clockSending.pausedCounter() << "times when streaming" << m_requestedDisplayName;
        }

        setData(QDateTime::currentDateTime(), closeDateRole);
    }
    else
    {
        setError("request already closed.");
    }
}

bool HttpRequest::sendHeader(const QStringList &header, HttpStatus status)
{
    if (thread() != QThread::currentThread())
        qWarning() << "HttpRequest::sendHeader" << thread() << QThread::currentThread();

    if (m_replyHeaderSent)
    {
        setError(QString("header already sent"));
        close();
        return false;
    }
    else if (m_client && m_status == "request ready")
    {
        if (!m_client->isValid() || !m_client->isWritable() || !m_client->isOpen())
        {
            setError(QString("unable to send data, client not ready"));
            close();
            return false;
        }
        else
        {
            if (status == HTTP_200_OK)
            {
                m_replyHeader << QString("%1 200 OK").arg(m_version);
            }
            else if (HTTP_500_KO)
            {
                m_replyHeader << QString("%1 500 Internal Server Error").arg(m_version);
            }
            else
            {
                setError(QString("unable to send data, invalid http status"));
                close();
                return false;
            }

            m_replyHeader << header;
            m_replyHeader << "";
            m_replyHeader << "";

            if (operation() == QNetworkAccessManager::HeadOperation)
            {
                // no data expected so header is sent immediately
                if (m_client->write(m_replyHeader.join("\r\n").toUtf8()) == -1)
                {
                    setError(QString("unable to send header data to client"));
                    close();
                    return false;
                }
                else
                {
                    logMessage(QString("header reply sent (%1 bytes).").arg(m_replyHeader.size()));
                    emit headerSent();

                    setData("OK", statusRole);
                    close();
                }

                m_replyHeaderSent = true;
            }
            else
            {
                setData("header set", statusRole);
            }

            return true;
        }
    }
    else
    {
        setError(QString("unable to send header data to client (status is wrong or client invalid)."));
        close();
        return false;
    }
}

bool HttpRequest::sendPartialData(const QByteArray &data)
{
    if (thread() != QThread::currentThread())
        qWarning() << "HttpRequest::sendPartialData" << thread() << QThread::currentThread();

    if (operation() != QNetworkAccessManager::HeadOperation && m_client && m_status == "header set")
    {
        if (!m_client->isValid() || !m_client->isWritable() || !m_client->isOpen())
        {
            logMessage(QString("unable to send data, client not ready"));
            if (!isClosed())
                close();
            return false;
        }
        else
        {
            QByteArray tmpData;

            if (!m_replyHeaderSent)
                tmpData.append(m_replyHeader.join("\r\n").toUtf8());

            tmpData.append(data);

            qint64 bytesWritten = m_client->write(tmpData);
            if (bytesWritten == -1)
            {
                logMessage(QString("unable to send data to client : %1").arg(m_client->errorString()));
                if (!isClosed())
                    close();
                return false;
            }
            else
            {
                if (!m_replyHeaderSent)
                {
                    logMessage(QString("header reply sent (%1 bytes).").arg(m_replyHeader.size()));
                    m_replyHeaderSent = true;
                    emit headerSent();
                }

                qDebug() << "sendPartialData, data sent" << bytesWritten << m_client->bytesToWrite();
#if !defined(QT_NO_DEBUG_OUTPUT)
                logMessage(QString("sendPartialData, data written : %1, pending bytes to write %2").arg(bytesWritten).arg(m_client->bytesToWrite()));
#endif
                return true;
            }
        }
    }
    else
    {
        logMessage(QString("cannot send reply, operation(%1) or client (%2) or status (%3) are invalid.").arg(operationString()).arg((qintptr)m_client).arg(m_status));
        if (!isClosed())
            close();
        return false;
    }
}

void HttpRequest::replyData(const QByteArray &data, const QString &contentType)
{    
    if (m_client && m_status == "request ready")
    {
        QStringList header;
        header << QString("Content-Type: %1").arg(contentType);
        header << QString("Content-Length: %1").arg(data.size());

        if (!sendHeader(header))
        {
            setError(QString("unable to send header to client"));
        }
        else if (operation() == QNetworkAccessManager::GetOperation)
        {
            m_replyData = data;

            if (!sendPartialData(m_replyData))
            {
                setError(QString("unable to send data to client"));
            }
            else
            {
                qDebug() << "replyData, data sent";
                qDebug() << m_replyHeader.join("\r\n").toUtf8();
                qDebug() << "data sent" << m_replyData.size() << "bytes";

                setData("OK", statusRole);
            }
        }
        else if (operation() == QNetworkAccessManager::HeadOperation)
        {
            qDebug() << "replyData, data sent";
            qDebug() << m_replyHeader.join("\r\n").toUtf8();

            setData("OK", statusRole);
        }
    }
    else
    {
        setError(QString("cannot send reply, client (%1) or status (%2) are invalid.").arg((qintptr)m_client).arg(m_status));
    }

    close();
}

void HttpRequest::replyFile(const QString &pathname)
{
    if (m_client && m_status == "request ready")
    {
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(pathname);

        if (mime.isValid())
        {
            QFile inputStream(pathname);
            if (inputStream.open(QFile::ReadOnly))
            {
                QStringList header;
                if (mime.inherits("text/xml"))
                    header << QString("Content-Type: text/xml; charset=\"utf-8\"");
                else
                    header << QString("Content-Type: %1").arg(mime.name());

                header << QString("Content-Length: %1").arg(inputStream.size());

                if (!sendHeader(header))
                {
                    setError(QString("unable to send header data to client"));
                }
                else if (operation() == QNetworkAccessManager::GetOperation)
                {
                    m_replyData = inputStream.readAll();

                    if (!sendPartialData(m_replyData))
                    {
                        setError(QString("unable to send data to client"));
                    }
                    else
                    {
                        qDebug() << "replyData, data sent";
                        qDebug() << m_replyHeader.join("\r\n").toUtf8();
                        qDebug() << "data sent" << m_replyData.size() << "bytes";

                        setData("OK", statusRole);
                    }
                }
                else if (operation() == QNetworkAccessManager::HeadOperation)
                {
                    qDebug() << "replyData, data sent";
                    qDebug() << m_replyHeader.join("\r\n").toUtf8();

                    setData("OK", statusRole);
                }

                inputStream.close();
            }
            else
            {
                setError(QString("unable to read file : %1.").arg(pathname));
            }
        }
        else
        {
            setError(QString("invalid file %1, mimetype is %2.").arg(pathname).arg(mime.name()));
        }
    }
    else
    {
        setError(QString("cannot send reply, client (%1) or status (%2) are invalid.").arg((qintptr)m_client).arg(m_status));
    }

    close();
}

void HttpRequest::replyError(const UpnpError &error)
{
    if (m_client && m_status == "request ready" && m_replyHeader.isEmpty())
    {
        QByteArray data = error.toByteArray(-1);

        QDateTime sdf;

        QStringList header;
        header << QString("Content-Type: text/xml; charset=\"utf-8\"");
        header << QString("Content-Length: %1").arg(data.size());
        header << QString("DATE: %1").arg(sdf.currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
        header << QString("EXT:");
        if (!serverName().isEmpty())
            header << QString("SERVER: %1").arg(serverName());

        if (!sendHeader(header, HTTP_500_KO))
        {
            setError(QString("unable to send header data to client"));
        }
        else if (!sendPartialData(data))
        {
            setError(QString("unable to send data to client"));
        }
        else
        {
            m_replyData = data;

            qDebug() << "replyError, data sent" << error.code() << error.description();
            qDebug() << m_replyHeader.join("\r\n").toUtf8();
            qDebug() << m_replyData;

            setData("KO", statusRole);
        }
    }
    else
    {
        setError(QString("cannot send reply, client (%1) or status (%2) or reply header (%3) are invalid.").arg((qintptr)m_client).arg(m_status).arg(m_replyHeader.isEmpty()));
    }

    close();
}

void HttpRequest::replyAction(const SoapActionResponse &response)
{
    if (m_client && m_status == "request ready" && m_replyHeader.isEmpty())
    {
        QByteArray data = response.toByteArray();

        QDateTime sdf;

        QStringList header;
        header << QString("Content-Type: text/xml; charset=\"utf-8\"");
        header << QString("Content-Length: %1").arg(data.size());
        header << QString("DATE: %1").arg(sdf.currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");
        header << QString("EXT:");
        if (!serverName().isEmpty())
            header << QString("SERVER: %1").arg(serverName());

        if (!sendHeader(header))
        {
            setError("unable to send header data to client");
        }
        else if (!sendPartialData(data))
        {
            setError("unable to send data to client");
        }
        else
        {
            m_replyData = data;

            qDebug() << "replyAction, data sent" << response.actionName();
            qDebug() << m_replyHeader.join("\r\n").toUtf8();
            qDebug() << m_replyData;
            setData("OK", statusRole);
        }
    }
    else
    {
        setError(QString("cannot send reply, client (%1) or status (%2) or reply header (%3) are invalid.").arg((qintptr)m_client).arg(m_status).arg(m_replyHeader.isEmpty()));
    }

    close();
}

void HttpRequest::requestCompleted()
{
    qDebug() << socketDescriptor() << "COMPLETED" << operationString() << url() << m_data.size();
    logMessage("Request fully received.");

    int contentLength = m_request.header(QNetworkRequest::ContentLengthHeader).toInt();
    if (m_data.size() != contentLength)
    {
        setError(QString("Invalid data size %1 != %2.").arg(contentLength).arg(m_data.size()));
    }
    else
    {
        if (m_status == "header read")
        {
            setData("request ready", statusRole);
            m_finished = true;
            emit finished();
        }
        else
        {
            setError(QString("invalid status (%1) when request is completed.").arg(m_status));
        }
    }
}

QString HttpRequest::errorString() const
{
    return m_error;
}

void HttpRequest::setError(const QString &message)
{
    qCritical() << message;
    logMessage(QString("ERROR : %1.").arg(message));
    m_error = message;
    setData("KO", statusRole);
}

bool HttpRequest::isFinished() const
{
    return m_finished;
}

void HttpRequest::logString(const QString &text)
{
    m_log.append(text);

    QVector<int> roles;
    roles << transcodeLogRole;
    emit itemChanged(roles);
}

void HttpRequest::logMessage(const QString &message)
{
    logString(QString("%1: %2\r\n").arg(QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss,zzz")).arg(message));
}

void HttpRequest::bytesWritten(const qint64 &size)
{
    networkBytesSent += size;
    emit bytesSentChanged();

    if (m_client)
    {
        qint64 bytesToWrite = m_client->bytesToWrite();

        qDebug() << QString("%1: %2 bytes sent, %4 total bytes sent, %3 bytes to write.").arg(QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss,zzz")).arg(size).arg(bytesToWrite).arg(networkBytesSent);
#if !defined(QT_NO_DEBUG_OUTPUT)
        logMessage(QString("data written to client (%1 bytes), %2 bytes to write, %3 bytes sent.").arg(size).arg(bytesToWrite).arg(networkBytesSent));
#endif

        if (!m_requestedResource.isEmpty() && (m_maxBufferSize - bytesToWrite) > (m_maxBufferSize/2))
            emit requestStreamingData(m_maxBufferSize - bytesToWrite);
    }
    else
    {
        qCritical() << "HttpRequest::bytesWritten invalid client";
    }
}

QString HttpRequest::deviceUuid() const
{
    return m_uuid;
}

void HttpRequest::setDeviceUuid(const QString &uuid)
{
    m_uuid = uuid;
}

void HttpRequest::streamError(const QString &error)
{
    m_streamWithErrors = true;
    logMessage(QString("streaming ERROR: %1").arg(error));
    setError(error);
}

QString HttpRequest::requestedResource() const
{
    return m_requestedResource;
}

void HttpRequest::setRequestedResource(const QString &name)
{
    m_requestedResource = name;
}

void HttpRequest::streamingCompleted()
{
    m_streamingCompleted = true;
    logMessage("streaming reached end.");
    qDebug() << "streaming reached end" << m_requestedResource;
}

void HttpRequest::streamingStatus(const QString &status)
{
    if (!status.contains("%"))
        logMessage(QString("streaming status changed: %1").arg(status));

    qDebug() << "streaming status changed" << status;

    if (status != m_streamingStatus)
        setData(status, streamingStatusRole);
}

void HttpRequest::streamOpened()
{
    if (!m_requestedResource.isEmpty() && netStatusTimerEvent == 0)
    {
        logMessage("stream open, start timer");
        netStatusTimerEvent = startTimer(STREAMING_PERIOD);
        if (netStatusTimerEvent == 0)
            qCritical() << "unable to start timer";

        if (!clockSending.isValid())
            clockSending.start();   // start clock to measure time taken for streaming

        emit servingRendererSignal(m_peerAddress.toString(), m_requestedDisplayName);

        if (m_client && m_maxBufferSize - m_client->bytesToWrite() > 0)
            emit requestStreamingData(m_maxBufferSize - m_client->bytesToWrite());
    }
    else
    {
        if (m_requestedResource.isEmpty())
            qCritical() << "stream opened but no resource name defined";
        if (netStatusTimerEvent != 0)
            qCritical() << "timer already started";
    }
}

void HttpRequest::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == netStatusTimerEvent)
    {
        if (!m_requestedResource.isEmpty())
        {
            if (clockSending.isValid())
            {
                // emit progress every 10 seconds
                if (clockSending.elapsedFromBeginning()/1000 % 10 == 0)
                    emit servingSignal(m_requestedResource, clockSending.elapsedFromBeginning());
            }
            else
            {
                emit servingSignal(m_requestedResource, 0);
            }

            if (m_client)
            {
                if (m_maxBufferSize - m_client->bytesToWrite() > 0)
                    emit requestStreamingData(m_maxBufferSize - m_client->bytesToWrite());

                if (m_streamingCompleted && m_client->bytesToWrite() == 0 && !isClosed())
                    close();
            }

            if (clockSending.isValid())
            {
                qint64 bytesToWrite = 0;
                if (m_client)
                    bytesToWrite = m_client->bytesToWrite();

                // display the network buffer and network speed
                int networkSpeed = int((double(networkBytesSent)/1024.0)/(double(clockSending.elapsed())/1000.0));

                if (m_maxBufferSize != 0)
                {
                    int bufferTime = 0;
                    if (networkSpeed != 0)
                        bufferTime = m_maxBufferSize/(networkSpeed*1024);

                    QString netStatus = QString("Time: %5 Buffer: %1% (%4 KB - %3 seconds), Speed: %2 KB/s").arg(int(100.0*double(bytesToWrite)/double(m_maxBufferSize))).arg(networkSpeed).arg(bufferTime).arg(m_maxBufferSize/1024).arg(QTime(0, 0).addMSecs(clockSending.elapsedFromBeginning()).toString("hh:mm:ss"));
                    setData(netStatus, networkStatusRole);
                }
            }

            if (lastNetBytesSent!=-1 && lastNetBytesSent==networkBytesSent)
            {
                // no data sent since last function call

                if (!clockSending.isStatePaused())
                {
                    clockSending.pause();
                    logMessage("PAUSE network");
                }

                emit networkPaused();
            }
            else if (networkBytesSent > 0 && clockSending.isStatePaused())
            {
                clockSending.start();
                logMessage(QString("RESTART network, %1 bytes sent.").arg(networkBytesSent-lastNetBytesSent));
            }

            lastNetBytesSent = networkBytesSent;
        }
    }
    else
    {
        qCritical() << "invalid timer event" << event->timerId();
    }
}

void HttpRequest::setClockSending(const qint64 &msec)
{
    clockSending.addMSec(msec);
}

void HttpRequest::setMaxBufferSize(const qint64 &size)
{
    m_maxBufferSize = size;
}

QString HttpRequest::requestedDisplayName() const
{
    return m_requestedDisplayName;
}

void HttpRequest::setRequestedDisplayName(const QString &name)
{
    m_requestedDisplayName = name;
}

HttpRange *HttpRequest::range(qint64 size)
{
    HttpRange *res = 0;

    if (!header("RANGE").isEmpty())
    {
        res = new HttpRange(QString("RANGE: %1").arg(header("RANGE")));
        if (res->isNull())
        {
            // invalid range, ignore it
            delete res;
            res = 0;
            qCritical() << "Invalid range in request:" << header("RANGE");
        }
        else
        {
            res->setSize(size);
        }

        return res;
    }
    else
    {
        return res;
    }
}

void HttpRequest::streamClosed()
{
    logMessage(QString("Streaming closed."));

    QObject * obj = sender();
    if (obj)
    {
        // disconnect all signal between stream object and request object
        obj->disconnect(this);
        disconnect(obj);
    }
}

long HttpRequest::bytesSent() const
{
    return networkBytesSent;
}

void HttpRequest::streamDataAvailable()
{
    if (!clockSending.isValid())
    {
        qCritical() << "stream not open";
    }
    else
    {
        if (m_client && m_client->bytesToWrite() < m_maxBufferSize*0.5)
            emit requestStreamingData(m_maxBufferSize - m_client->bytesToWrite());
    }
}
