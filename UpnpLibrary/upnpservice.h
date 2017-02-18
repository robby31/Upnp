#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include "upnpobject.h"

class UpnpService : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(QString serviceType READ serviceType NOTIFY serviceTypeChanged)
    Q_PROPERTY(QStringList actionsModel READ actionsModel NOTIFY actionsModelChanged)

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

    QString serviceType() const;
    QString serviceId() const;

    void getDescription();

    QStringList actionsModel() const;

    Q_INVOKABLE void runAction(const int &index);

private:
    void initRoles();
    void readActions();

signals:
    void serviceTypeChanged();
    void actionsModelChanged();

public slots:

private slots:
    void availableSlotChanged();
    void descriptionReceived();

private:
    QDomNode m_info;
    QStringList m_actionsModel;
};

#endif // UPNPSERVICE_H
