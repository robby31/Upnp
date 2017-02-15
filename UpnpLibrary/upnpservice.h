#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include "upnpobject.h"

class UpnpService : public UpnpObject
{
    Q_OBJECT

    enum Roles {
        ServiceTypeRole = Qt::UserRole+1,
        AvailableRole
    };

public:
    explicit UpnpService(QObject *parent = 0);
    explicit UpnpService(QHostAddress host, QDomNode info, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString getInfo(const QString &param) const;

    void getDescription();

private:
    void initRoles();

signals:

public slots:

private slots:
    void descriptionReceived();

private:
    QDomNode m_info;
};

#endif // UPNPSERVICE_H
