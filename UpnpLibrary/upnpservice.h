#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include "upnpobject.h"
#include "upnperror.h"
#include "soapaction.h"
#include "Models/listmodel.h"
#include "statevariableitem.h"
#include "upnpservicedescription.h"

class UpnpService : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(QString serviceType READ serviceType NOTIFY serviceTypeChanged)
    Q_PROPERTY(QStringList actionsModel READ actionsModel NOTIFY actionsModelChanged)
    Q_PROPERTY(ListModel* stateVariablesModel READ stateVariablesModel NOTIFY stateVariablesModelChanged)

    enum Roles {
        ServiceTypeRole = Qt::UserRole+1,
        AvailableRole
    };

public:
    explicit UpnpService(QObject *parent = 0);
    explicit UpnpService(UpnpObject *upnpParent, QDomNode info, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString serviceType() const;
    QString serviceId() const;

    QUrl scpdUrl() const;
    QUrl controlUrl() const;
    QUrl eventSubUrl() const;

    virtual void searchForST(const QString &st, const QString &uuid);

    QStringList actionsModel() const;
    ListModel *stateVariablesModel();
    void updateStateVariables(QHash<QString,QString> data);
    void updateLastChange(QString data);


private:
    void initRoles();

    QString getInfo(const QString &param) const;

    QNetworkReply *sendAction(const SoapAction &action);

signals:
    void serviceTypeChanged();
    void actionsModelChanged();
    void stateVariablesModelChanged();
    void subscribeEventingSignal(const QNetworkRequest &request, const QString &serviceId);

public slots:
    void requestDescription();
    void sendAlive(const QString &uuid);
    void sendByeBye(const QString &uuid);
    void runAction(const int &index);
    void subscribeEventing();

private slots:
    void readActions();
    void readStateVariables();
    void itemAvailableChanged();
    void networkError(QNetworkReply::NetworkError error);
    void descriptionReceived();
    void actionFinished();

private:
    QDomNode m_info;
    QStringList m_actionsModel;
    ListModel m_stateVariablesModel;
};

#endif // UPNPSERVICE_H
