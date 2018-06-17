#ifndef STATEVARIABLEITEM_H
#define STATEVARIABLEITEM_H

#include "Models/listitem.h"
#include <QVariant>
#include <QDebug>

class StateVariableItem : public ListItem
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole+1,
        ValueRole,
        SendEventsRole,
        MulticastRole,
        DefaultValueRole
    };

    explicit StateVariableItem(QObject *parent = 0);

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual QVariant data(int role) const Q_DECL_OVERRIDE;
    virtual bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

signals:

public slots:

private:
    QHash<int, QByteArray> m_roles;
    QString m_name;
    QString m_value;
    bool m_sendEnvents = false;
    bool m_multicast = false;
    QString m_defaultValue;
};

#endif // STATEVARIABLEITEM_H
