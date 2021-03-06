#ifndef UPNPOBJECT_H
#define UPNPOBJECT_H

#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "ssdpmessage.h"
#include "Models/listitem.h"
#include "upnpdescription.h"
#include "mynetwork.h"
#include "debuginfo.h"
#include "httprequest.h"
#include "upnperror.h"
#include "soapactionresponse.h"

class UpnpObject : public ListItem
{
    Q_OBJECT

    Q_PROPERTY(UpnpObject *upnpParent READ upnpParent WRITE setUpnpParent NOTIFY upnpParentChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString description READ strDescription  NOTIFY descriptionChanged)
    Q_PROPERTY(bool available READ available WRITE setAvailable NOTIFY availableChanged)

public:
    enum TypeObject { Unknown, T_RootDevice, T_Device, T_Service };
    enum Status { Null, Loading, Ready, Error };

    explicit UpnpObject(QObject *parent = Q_NULLPTR);
    explicit UpnpObject(TypeObject type, UpnpObject *upnpParent, QObject *parent = Q_NULLPTR);

    TypeObject type() const;
    void setType(const TypeObject &type);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    void setRoles(const QHash<int, QByteArray> &roles);

    UpnpObject *upnpParent() const;
    void setUpnpParent(UpnpObject *parent);

    Status status() const;

    bool available() const;
    void setAvailable(bool flag);

    QDateTime timeoutDateTime() const;

    void update(const SsdpMessage &message);

    virtual QHostAddress host() const;

    virtual quint16 port() const;

    virtual QString serverName() const;

    virtual QUrl url() const;

    QUrl urlFromRelativePath(const QString &path) const;

    UpnpDescription *description() const;
    QString strDescription() const;
    bool setDescription(UpnpDescription *descr);

    QString valueFromDescription(const QString &param) const;

    QNetworkReply *get(QNetworkRequest request);
    QNetworkReply *post(QNetworkRequest request, const QByteArray &data);

    virtual QString generateUuid();

protected:
    void setStatus(const Status &status);

    void replyUpnpError(HttpRequest *request, const UpnpError::ErrorTypes &errorType);

signals:
    void upnpParentChanged();
    void statusChanged();
    void descriptionChanged();
    void availableChanged();

    void aliveMessage(const QString &uuid, const QString &nt);
    void byebyeMessage(const QString &uuid, const QString &nt);

    void searchResponse(const QHostAddress &address, const quint16 &port, const QString &st, const QString &usn);

private slots:
    void timeout();

    virtual void parseObject() = 0; // parse Upnp Object to read content

private:
    UpnpObject *m_upnpParent = Q_NULLPTR;
    Status m_status = Null;
    QHash<int, QByteArray> m_roles;
    TypeObject m_type = Unknown;
    QDateTime m_timeout;
    QTimer m_timer;
    bool m_available = false;
    UpnpDescription *m_description = Q_NULLPTR;
};

#endif // UPNPOBJECT_H
