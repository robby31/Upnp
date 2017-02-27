#ifndef UPNPOBJECT_H
#define UPNPOBJECT_H

#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include "ssdpmessage.h"
#include "Models/listitem.h"

class UpnpObject : public ListItem
{
    Q_OBJECT

    Q_PROPERTY(UpnpObject *upnpParent READ upnpParent WRITE setUpnpParent NOTIFY upnpParentChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString description READ strDescription  NOTIFY descriptionChanged)
    Q_PROPERTY(bool available READ available WRITE setAvailable NOTIFY availableChanged)

public:
    enum TypeObject { RootDevice, Device, Service };
    enum Status { Null, Loading, Ready, Error };

    explicit UpnpObject(QObject *parent = 0);
    explicit UpnpObject(TypeObject type, UpnpObject *upnpParent, QObject *parent = 0);

    TypeObject type() const;
    void setType(const TypeObject &type);

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    void setRoles(QHash<int, QByteArray> roles);

    UpnpObject *upnpParent() const;

    virtual QNetworkAccessManager *networkManager() const;

    Status status() const;
    void setStatus(const Status &status);

    bool available() const;
    void setAvailable(bool flag);

    QDateTime timeoutDateTime() const;

    void update(const SsdpMessage &message);

    virtual QHostAddress host() const;

    virtual QString serverName() const;

    virtual QUrl url() const;

    QUrl urlFromRelativePath(QString path) const;

    QDomNode description() const;
    QString strDescription() const;
    void setDescription(QDomNode node);

    QString valueFromDescription(const QString &param) const;

    QNetworkReply *get(QNetworkRequest request);
    QNetworkReply *post(QNetworkRequest request, QByteArray data);

private:
    void setUpnpParent(UpnpObject *parent);

signals:
    void upnpParentChanged();
    void statusChanged();
    void descriptionChanged();
    void availableChanged();

    void aliveMessage(const QString &uuid, const QString &nt);
    void byebyeMessage(const QString &uuid, const QString &nt);

    void searchResponse(const QString &st, const QString &usn);

public slots:

private slots:
    void timeout();

private:
    UpnpObject *m_upnpParent;
    Status m_status;
    QHash<int, QByteArray> m_roles;
    TypeObject m_type;
    QDateTime m_timeout;
    QTimer m_timer;
    bool m_available;
    QDomNode m_description;
};

#endif // UPNPOBJECT_H
