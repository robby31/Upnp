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


class HttpRequest : public ListItem
{
    Q_OBJECT

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
        HTTP_500_KO  // internal server error
    };

    explicit HttpRequest(QObject *parent = 0);
    explicit HttpRequest(QTcpSocket *client, QObject *parent = 0);

    virtual QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(int role) const;
    virtual bool setData(const QVariant &value, const int &role);

    QNetworkAccessManager::Operation operation() const;
    QString operationString() const;
    QUrl url() const;
    QString version() const;
    QString header(const QString &param) const;
    QByteArray requestData() const;

    QString serverName() const;
    void setServerName(const QString &name);

    QString deviceUuid() const;
    void setDeviceUuid(const QString &uuid);

    QString requestedResource() const;
    void setRequestedResource(const QString &name);

    QTcpSocket *tcpSocket() const;
    QHostAddress peerAddress() const;
    quint16 peerPort() const;
    qintptr socketDescriptor() const;

    QString errorString() const;

    void incomingData();

    bool isFinished() const;

    void logString(const QString &text);

    bool sendHeader(const QStringList &header, HttpStatus status = HTTP_200_OK);
    void replyData(const QByteArray &data, const QString &contentType = "text/xml; charset=\"utf-8\"");
    void replyFile(const QString &pathname);
    void replyError(const UpnpError &error);
    void replyAction(const SoapActionResponse &response);

    bool isClosed() const;
    void close();

    void setClockSending(const qint64 &msec);

    void setMaxBufferSize(const qint64 &size);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void initializeRoles();
    void readOperation(const QString &data);
    void readHeader(const QString &data);

    void headerReadFinished();
    void requestCompleted();



signals:
    void finished();

    void headerSent();

    void bytesSent(const qint64 &size, const qint64 &bytesToWrite);

    // emit signal to provide progress on serving media
    void servingSignal(QString filename, int playedDurationInMs);

    // emit signal when serving is finished
    //   status = 0 if serving finished successfully
    //   status = 1 if error occurs
    void servingFinishedSignal(QString ip, QString filename, int status);

    void networkPaused();


public slots:
    void setError(const QString &message);

    void logMessage(const QString &message);

    void streamOpened();

    bool sendPartialData(const QByteArray &data);

    void bytesWritten(const qint64 &size);


private slots:
    void clientDestroyed();
    void clientError(QAbstractSocket::SocketError error);
    void socketStateChanged(QAbstractSocket::SocketState state);

    void streamError(const QString &error);
    void streamingCompleted();
    void streamingStatus(const QString &status);

private:
    QHash<int, QByteArray> m_roles;
    QDateTime m_date;
    QDateTime m_closeDate;
    QHostAddress m_peerAddress;
    QTcpSocket *m_client;
    QNetworkRequest m_request;
    QNetworkAccessManager::Operation m_operation;
    QString m_version;
    bool m_headerCompleted;
    QStringList m_header;
    QByteArray m_data;

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
    qint64 m_maxBufferSize;

    long networkBytesSent;
    long lastNetBytesSent;
    int netStatusTimerEvent;
    ElapsedTimer clockSending;          // clock to mesure time taken to send streamed or transcoded data.
};

#endif // HTTPREQUEST_H
