#ifndef MESSAGEITEM_H
#define MESSAGEITEM_H

#include <QHash>
#include <QVariant>
#include <QDebug>
#include <QHostAddress>
#include <QDateTime>
#include "Models/listitem.h"
#include "ssdpmessage.h"

class MessageItem : public ListItem
{
    Q_OBJECT

public:
    enum Roles {
        HostRole = Qt::UserRole+1,
        PortRole,
        DateRole,
        TypeRole,
        MessageRole,
        NtRole,
        NtsRole,
        ManRole,
        StRole
    };

    explicit MessageItem(QObject* parent = Q_NULLPTR);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE { return m_roles; }
    QVariant data(int role) const Q_DECL_OVERRIDE;
    bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    void setHostAddress(const QHostAddress& host);
    void setPort(const quint16& port);
    void setMessage(const SsdpMessage& msg);

private:
    QHash<int, QByteArray> m_roles;
    QDateTime m_date;
    QHostAddress m_host;
    quint16 m_port;
    SsdpMessage m_message;
};

#endif // MESSAGEITEM_H
