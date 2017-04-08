#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include "upnpobject.h"
#include "upnperror.h"
#include "soapaction.h"

class UpnpService : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(QString serviceType READ serviceType NOTIFY serviceTypeChanged)
    Q_PROPERTY(QStringList actionsModel READ actionsModel NOTIFY actionsModelChanged)
    Q_PROPERTY(QStringList stateVariablesModel READ stateVariablesModel NOTIFY stateVariablesModelChanged)

    enum Roles {
        ServiceTypeRole = Qt::UserRole+1,
        AvailableRole
    };

public:
    explicit UpnpService(QObject *parent = 0);
    explicit UpnpService(UpnpObject *upnpParent, QDomNode info, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    QString getInfo(const QString &param) const;

    QString serviceType() const;
    QString serviceId() const;

    QUrl scpdUrl() const;
    QUrl controlUrl() const;
    QUrl eventUrl() const;

    virtual void searchForST(const QString &st, const QString &uuid);

    QStringList actionsModel() const;
    QStringList stateVariablesModel() const;


private:
    void initRoles();
    void readActions();
    void readStateVariables();

    QNetworkReply *sendAction(const SoapAction &action);

signals:
    void infoChanged();
    void serviceTypeChanged();
    void actionsModelChanged();
    void stateVariablesModelChanged();
    void subscribeEventingSignal(const QNetworkRequest &request, const QString &serviceId);

public slots:
    void sendAlive(const QString &uuid);
    void sendByeBye(const QString &uuid);
    void runAction(const int &index);
    void subscribeEventing();

private slots:
    void requestDescription();
    void itemAvailableChanged();
    void networkError(QNetworkReply::NetworkError error);
    void descriptionReceived();
    void actionFinished();

private:
    QDomNode m_info;
    QStringList m_actionsModel;
    QStringList m_stateVariablesModel;
};

#endif // UPNPSERVICE_H
