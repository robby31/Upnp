#include "statevariableitem.h"

StateVariableItem::StateVariableItem(QObject *parent):
    ListItem(parent)
{
    m_roles[NameRole] = "name";
    m_roles[ValueRole] = "value";
    m_roles[SendEventsRole] = "sendEvents";
    m_roles[MulticastRole] = "multicast";
    m_roles[DefaultValueRole] = "defaultValue";
}

QHash<int, QByteArray>  StateVariableItem::roleNames() const
{
    return m_roles;
}

QVariant StateVariableItem::data(int role) const
{
    switch (role) {
    case NameRole:
        return m_name;
    case ValueRole:
        return m_value;
    case SendEventsRole:
        return m_sendEnvents;
    case MulticastRole:
        return m_multicast;
    case DefaultValueRole:
        return m_defaultValue;
    default:
        break;
    }
    return QVariant::Invalid;
}

bool StateVariableItem::setData(const QVariant &value, const int &role)
{
    QVector<int> roles;

    switch (role)
    {
    case NameRole:
        m_name = value.toString();
        roles << role;
        emit itemChanged(roles);
        return true;

    case ValueRole:
        m_value = value.toString();
        roles << role;
        emit itemChanged(roles);
        return true;

    case SendEventsRole:
        m_sendEnvents = value.toBool();
        roles << role;
        emit itemChanged(roles);
        return true;

    case MulticastRole:
        m_multicast = value.toBool();
        roles << role;
        emit itemChanged(roles);
        return true;

    case DefaultValueRole:
        m_defaultValue = value.toString();
        roles << role;
        emit itemChanged(roles);
        return true;

    default:
        qCritical() << this << "invalid role" << role;
    }

    return false;
}
