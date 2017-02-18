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

    Q_PROPERTY(QString description READ strDescription  NOTIFY descriptionChanged)
    Q_PROPERTY(bool available READ available WRITE setAvailable NOTIFY availableChanged)

public:
    enum TypeObject { RootDevice, Device, Service };

    explicit UpnpObject(QObject *parent = 0);
    explicit UpnpObject(TypeObject type, QHostAddress host, QObject *parent = 0);

    TypeObject type() const;

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    void setRoles(QHash<int, QByteArray> roles);

    bool available() const;
    void setAvailable(bool flag);

    QDateTime timeoutDateTime() const;

    void update(const SsdpMessage &message);

    QNetworkAccessManager *getNetworkManager() const;
    void setNetworkManager(QNetworkAccessManager *nam);

    QHostAddress host() const;

    QUrl url() const;
    void setUrl(QUrl url);

    QUrl urlFromRelativePath(QString path);

    QDomDocument description() const;
    QString strDescription() const;
    void setDescription(QByteArray data);
    void setDescription(QDomNode node);

    QString valueFromDescription(const QString &param) const;

    QNetworkReply *get(const QString &location);

signals:
    void descriptionChanged();
    void availableChanged();

public slots:

private slots:
    void timeout();

private:
    QHash<int, QByteArray> m_roles;
    TypeObject m_type;
    QDateTime m_timeout;
    QTimer m_timer;
    bool m_available;
    QHostAddress m_host;
    QUrl m_url;
    QDomDocument m_description;

    QNetworkAccessManager *netManager;
};

#endif // UPNPOBJECT_H
