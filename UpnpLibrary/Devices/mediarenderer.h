#ifndef MEDIARENDERER_H
#define MEDIARENDERER_H

#include "Models/listmodel.h"
#include "upnprootdevice.h"
#include "dlna/protocol.h"


class MediaRenderer : public ListItem
{
    Q_OBJECT

    Q_DISABLE_COPY_MOVE(MediaRenderer)

public:
    explicit MediaRenderer(QObject *parent = Q_NULLPTR);
    explicit MediaRenderer(UpnpRootDevice *device, QObject *parent = Q_NULLPTR);
    explicit MediaRenderer(UpnpRootDevice *device, const QString &profilesPath, QObject *parent = Q_NULLPTR);

    enum Roles {
        statusRole = Qt::UserRole+1,
        nameRole,
        networkAddressRole,
        iconUrlRole,
        availableRole,
        sinkProtocolRole
    };

    QString id() const Q_DECL_OVERRIDE;

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QVariant data(int role) const Q_DECL_OVERRIDE;
    bool setData(const QVariant &value, const int &role) Q_DECL_OVERRIDE;

    QString netWorkAddress() const;

    Protocol *dlnaProfiles() const;
    QStringList sinkProtocols() const;

signals:
    void removeRenderer();

private slots:
    void deviceItemChanged(const QVector<int> &roles);
    void deviceDestroyed(QObject *obj);

    void serviceStatusChanged();
    void stateVarChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

private:
    QHash<int, QByteArray> m_roles;

    UpnpRootDevice *m_device = Q_NULLPTR;
    QString status;

    QStringList m_sinkProtocol;
    Protocol *m_dlnaProfiles = Q_NULLPTR;
};

#endif // MEDIARENDERER_H
