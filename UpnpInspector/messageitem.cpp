#include "messageitem.h"

MessageItem::MessageItem(QObject *parent):
    ListItem(parent),
    m_roles(),
    m_date(QDateTime::currentDateTime()),
    m_host(),
    m_port(-1),
    m_message()
{
    m_roles[HostRole] = "host";
    m_roles[PortRole] = "port";
    m_roles[DateRole] = "date";
    m_roles[TypeRole] = "type";
    m_roles[MessageRole] = "message";
    m_roles[NtRole] = "nt";
    m_roles[NtsRole] = "nts";
}

QVariant MessageItem::data(int role) const
{
    switch (role) {
    case HostRole:
        return m_host.toString();
    case PortRole:
        return m_port;
    case DateRole:
        return m_date.toString("hh:mm:ss.zzz");
    case TypeRole:
        return m_message.startLine();
    case MessageRole:
        return m_message.toUtf8();
    case NtRole:
        return m_message.getHeader("NT");
    case NtsRole:
        return m_message.getHeader("NTS");
    default:
        return QVariant::Invalid;
    }

    return QVariant::Invalid;
}

bool MessageItem::setData(const QVariant &value, const int &role)
{
    qWarning() << "unable to set data" << value << role;
    return false;
}

void MessageItem::setHostAddress(QHostAddress host)
{
    m_host = host;
}

void MessageItem::setPort(int port)
{
    m_port = port;
}

void MessageItem::setMessage(SsdpMessage msg)
{
    m_message = msg;
}
