#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include "upnpobject.h"
#include "upnperror.h"
#include "soapaction.h"
#include "Models/listmodel.h"
#include "statevariableitem.h"
#include "upnpservicedescription.h"
#include "httprequest.h"
#include "xmlevent.h"
#include "upnpactionreply.h"

using T_SUBSCRIPTION = struct {
    QStringList urls;
    int eventKey = 0;
    int timeOut = 0;
    QDateTime timeOver;
};

using T_SENDEVENTS = struct {
    QNetworkReply *reply;
};

class UpnpService : public UpnpObject
{
    Q_OBJECT

    Q_PROPERTY(QString serviceType READ serviceType NOTIFY serviceTypeChanged)
    Q_PROPERTY(QStringList actionsModel READ actionsModel NOTIFY actionsModelChanged)
    Q_PROPERTY(ListModel* stateVariablesModel READ stateVariablesModel NOTIFY stateVariablesModelChanged)

public:

    enum Roles {
        ServiceTypeRole = Qt::UserRole+1,
        AvailableRole
    };

    explicit UpnpService(QObject *parent = Q_NULLPTR);
    explicit UpnpService(UpnpObject *upnpParent, QObject *parent = Q_NULLPTR);
    explicit UpnpService(UpnpObject *upnpParent, const QDomNode& info, QObject *parent = Q_NULLPTR);

    QString id() const Q_DECL_OVERRIDE;

    QVariant data(int role) const Q_DECL_OVERRIDE;

    bool setInfo(const QDomNode& info);

    QString serviceType() const;
    QString serviceId() const;

    QUrl scpdUrl() const;
    QUrl controlUrl() const;
    QUrl eventSubUrl() const;

    virtual void searchForST(const QHostAddress &host, const quint16 &port, const QString &st, const QString &uuid);

    QStringList actionsModel() const;
    ListModel *stateVariablesModel();
    StateVariableItem *findStateVariableByName(const QString &name);
    void updateStateVariable(const QString &name, const QString &value);
    void updateStateVariables(QHash<QString,QString> data);
    void updateLastChange(const QString& data);

    virtual bool replyRequest(HttpRequest *request);

private:
    void initRoles();

    QString getInfo(const QString &param) const;

    QNetworkReply *sendAction(const SoapAction &action);
    QDomNode getAction(const QString &actionName);

    bool replyNewSubscription(HttpRequest *request);
    bool replyRenewSubscription(HttpRequest *request);
    void startCheckSubscription();
    void stopCheckSubscription();

    void sendEvent(const QString &uuid);

protected:
    virtual bool replyAction(HttpRequest *request, const SoapAction &action);

    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

signals:
    void serviceTypeChanged();
    void actionsModelChanged();
    void stateVariablesModelChanged();
    void subscribeEventingSignal(const QNetworkRequest &request, const QString &serviceId);
    void errorOccured(const UpnpError &error);

public slots:
    void requestDescription();
    void sendAlive(const QString &uuid);
    void sendByeBye(const QString &uuid);
    UpnpActionReply *runAction(const int &index);
    UpnpActionReply *runAction(const QString &actionName, QVariantMap args = QVariantMap());
    UpnpActionReply *runAction(const SoapAction &action);
    void subscribeEventing();

private slots:
    void parseObject() Q_DECL_OVERRIDE; // parse service to read actions and state variables

    void readActions();
    void readStateVariables();
    void itemAvailableChanged();
    void networkError(QNetworkReply::NetworkError error);
    void descriptionReceived();

    void sendEventReply();

private:
    QDomNode m_info;
    QStringList m_actionsModel;
    ListModel m_stateVariablesModel;

    QHash<QString, T_SUBSCRIPTION> m_subscription;
    int m_timerCheckSubscription = -1;
    QHash<int, QString> m_sendEventTimer;
    QHash<int, T_SENDEVENTS> m_checkSendEvent;
};

#endif // UPNPSERVICE_H
