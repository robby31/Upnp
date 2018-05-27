#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "Models/listitem.h"
#include <QDateTime>
#include <QFile>
#include <QMimeDatabase>
#include "upnperror.h"
#include "soapactionresponse.h"
#include "elapsedtimer.h"
#include <QThread>
#include <QTimerEvent>
#include "httprange.h"


class HttpRequest : public ListItem
{
    Q_OBJECT

    Q_PROPERTY(long bytesSent READ bytesSent NOTIFY bytesSentChanged)

public:
    enum Roles {
        operationRole = Qt::UserRole+1,
        urlRole,
        hostRole,
        peerAddressRole,
        statusRole,
        headerRole,
        contentRole,
        durationRole,
        dateRole,
        closeDateRole,
        answerRole,
        networkStatusRole,
        streamingStatusRole,
        transcodeLogRole
    };

    enum HttpStatus {
        HTTP_200_OK,
        HTTP_400_KO, // Bad Request
        HTTP_412_KO, // Precondition Failed
        HTTP_500_KO  // internal server error
    };

    explicit HttpRequest(QObject *parent = 0);
    explicit HttpRequest(QTcpSocket *client, QObject *parent = 0);

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual QVariant data(int role) const Q_DECL_OVERRIDE;
    virtual bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    QNetworkAccessManager::Operation operation() const;
    QString operationString() const;
    QUrl url() const;
    QString version() const;
    QString header(const QString &param) const;
    QByteArray requestData() const;

    HttpRange *range(qint64 size);

    QString serverName() const;
    void setServerName(const QString &name);

    QString deviceUuid() const;
    void setDeviceUuid(const QString &uuid);

    QString requestedResource() const;
    void setRequestedResource(const QString &name);

    QString requestedDisplayName() const;
    void setRequestedDisplayName(const QString &name);

    QTcpSocket *tcpSocket() const;
    QHostAddress peerAddress() const;
    quint16 peerPort() const;
    qintptr socketDescriptor() const;

    long bytesSent() const;

    QString errorString() const;

    void incomingData();

    bool isFinished() const;

    void logString(const QString &text);

    bool sendHeader(const QStringList &header, HttpStatus status = HTTP_200_OK);
    void replyData(const QByteArray &data, const QString &contentType = "text/xml; charset=\"utf-8\"");
    void replyFile(const QString &pathname);
    void replyError(const UpnpError &error);
    void replyError(const HttpStatus &error);
    void replyAction(const SoapActionResponse &response);

    bool isClosed() const;

    void setClockSending(const qint64 &msec);

    void setMaxBufferSize(const qint64 &size);

protected:
    virtual void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private:
    void initializeRoles();
    void readOperation(const QString &data);
    void readHeader(const QString &data);

    void headerReadFinished();
    void requestCompleted();



signals:
    void finished();

    void headerSent();

    void requestStreamingData(qint64 maxlen);

    void servingRendererSignal(QString ip, const QString &mediaName);

    // emit signal to provide progress on serving media
    void servingSignal(QString filename, int playedDurationInMs);

    // emit signal when serving is finished
    //   status = 0 if serving finished successfully
    //   status = 1 if error occurs
    void servingFinishedSignal(QString ip, QString filename, int status);

    void networkPaused();

    void bytesSentChanged();


public slots:
    void setError(const QString &message);

    void logMessage(const QString &message);

    void streamOpened();

    bool sendPartialData(const QByteArray &data);

    void bytesWritten(const qint64 &size);

    void close();

private slots:
    void clientDestroyed();
    void clientError(QAbstractSocket::SocketError error);
    void socketStateChanged(QAbstractSocket::SocketState state);

    void streamDataAvailable();
    void streamError(const QString &error);
    void streamingCompleted();
    void streamingStatus(const QString &status);
    void streamClosed();

private:
    static const int STREAMING_PERIOD;

    QHash<int, QByteArray> m_roles;
    QDateTime m_date;
    QDateTime m_closeDate;
    QHostAddress m_peerAddress;
    QTcpSocket *m_client;
    QNetworkRequest m_request;
    QNetworkAccessManager::Operation m_operation;
    QString m_customOperation;
    QString m_version;
    bool m_headerCompleted;
    QStringList m_header;
    QByteArray m_data;

    bool m_replyHeaderSent;
    QStringList m_replyHeader;
    QByteArray m_replyData;

    QString m_serverName;
    QString m_uuid;
    QString m_status;
    QString m_networkStatus;

    QString m_error;

    bool m_finished;

    QString m_log;

    QString m_streamingStatus;
    bool m_streamWithErrors;
    bool m_streamingCompleted;
    QString m_requestedResource;
    QString m_requestedDisplayName;
    qint64 m_maxBufferSize;

    long networkBytesSent;
    long lastNetBytesSent;
    int netStatusTimerEvent;
    ElapsedTimer clockSending;          // clock to mesure time taken to send streamed or transcoded data.
};

#endif // HTTPREQUEST_H
