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
public:
    enum Roles {
        HostRole = Qt::UserRole+1,
        PortRole,
        DateRole,
        TypeRole,
        MessageRole,
        NtRole,
        NtsRole
    };

    explicit MessageItem(QObject* parent = 0);

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE { return m_roles; }
    virtual QVariant data(int role) const Q_DECL_OVERRIDE;
    virtual bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    void setHostAddress(QHostAddress host);
    void setPort(int port);
    void setMessage(SsdpMessage msg);

private:
    QHash<int, QByteArray> m_roles;
    QDateTime m_date;
    QHostAddress m_host;
    int m_port;
    SsdpMessage m_message;
};

#endif // MESSAGEITEM_H
