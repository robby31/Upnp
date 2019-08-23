#ifndef MEDIARENDERERMODEL_H
#define MEDIARENDERERMODEL_H

#include <QDebug>
#include "mediarenderer.h"
#include "Models/listmodel.h"
#include <QHostAddress>
#include "dlna/protocol.h"

class MediaRendererModel : public ListModel
{
    Q_OBJECT

public:
    explicit MediaRendererModel(QObject *parent = Q_NULLPTR);

    MediaRenderer *rendererFromIp(const QString &ip);

    void setDlnaProfilesPath(const QString &path);

signals:
    void mediaRendererDestroyed(const QString &hostaddress);

public slots:
    void addMediaRenderer(UpnpRootDevice *device);
    void removeRenderer();

    // the renderer is serving a new media
    void serving(const QString &ip, const QString &mediaName);

private:
    QString m_profilesPath;
};

#endif // MEDIARENDERERMODEL_H
