#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include "upnpobject.h"
#include "upnperror.h"
#include "soapaction.h"
#include "Models/listmodel.h"
#include "statevariableitem.h"
#include "upnpservicedescription.h"
#include "Http/httprequest.h"
#include "xmlevent.h"

typedef struct {
    QStringList urls;
    int eventKey = 0;
} T_SUBSCRIPTION;

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

    explicit UpnpService(QObject *parent = 0);
    explicit UpnpService(UpnpObject *upnpParent, QObject *parent = 0);
    explicit UpnpService(UpnpObject *upnpParent, QDomNode info, QObject *parent = 0);

    virtual QString id() const Q_DECL_OVERRIDE;

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;

    bool setInfo(QDomNode info);

    QString serviceType() const;
    QString serviceId() const;

    QUrl scpdUrl() const;
    QUrl controlUrl() const;
    QUrl eventSubUrl() const;

    virtual void searchForST(const QHostAddress &host, const int &port, const QString &st, const QString &uuid);

    QStringList actionsModel() const;
    ListModel *stateVariablesModel();
    StateVariableItem *findStateVariableByName(const QString &name);
    void updateStateVariable(const QString &name, const QString &value);
    void updateStateVariables(QHash<QString,QString> data);
    void updateLastChange(QString data);

    virtual bool replyRequest(HttpRequest *request);

private:
    void initRoles();

    QString getInfo(const QString &param) const;

    QNetworkReply *sendAction(const SoapAction &action);

    bool replyNewSubscription(HttpRequest *request);
    bool replyRenewSubscription(HttpRequest *request);

    void sendEvent(const QString &uuid);

protected:
    virtual bool replyAction(HttpRequest *request, const SoapAction &action);

    virtual void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

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
    virtual void parseObject() Q_DECL_OVERRIDE; // parse service to read actions and state variables

    void readActions();
    void readStateVariables();
    void itemAvailableChanged();
    void networkError(QNetworkReply::NetworkError error);
    void descriptionReceived();
    void actionFinished();

    void sendEventReply();

private:
    QDomNode m_info;
    QStringList m_actionsModel;
    ListModel m_stateVariablesModel;

    QHash<QString, T_SUBSCRIPTION> m_subscription;
    QHash<int, QString> m_sendEventTimer;
};

#endif // UPNPSERVICE_H
