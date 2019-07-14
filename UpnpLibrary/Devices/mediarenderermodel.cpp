#include "mediarenderermodel.h"

MediaRendererModel::MediaRendererModel(QObject *parent) :
    ListModel(new MediaRenderer, parent)
{

}

void MediaRendererModel::addMediaRenderer(UpnpRootDevice *device)
{
    auto renderer = qobject_cast<MediaRenderer*>(find(device->uuid()));
    if (!renderer)
    {
        renderer = new MediaRenderer(device, this);
        renderer->setDlnaProfiles(m_dlnaProfiles);
        connect(renderer, SIGNAL(removeRenderer()), this, SLOT(removeRenderer()));
        appendRow(renderer);
    }
}

void MediaRendererModel::removeRenderer()
{
    auto renderer = qobject_cast<MediaRenderer*>(sender());

    QModelIndex index = indexFromItem(renderer);

    if (index.isValid())
    {
        removeRow(index.row());
        emit mediaRendererDestroyed(renderer->netWorkAddress());
    }
}

void MediaRendererModel::serving(const QString &ip, const QString &mediaName)
{
    MediaRenderer* renderer = rendererFromIp(ip);
    if (renderer)
    {
        if (mediaName.isEmpty())
            renderer->setData("standby", MediaRenderer::statusRole);
        else
            renderer->setData(QString("Serving %1").arg(mediaName), MediaRenderer::statusRole);
    }
    else
    {
        qCritical() << this << "unable to find renderer" << ip;
    }
}

MediaRenderer *MediaRendererModel::rendererFromIp(const QString &ip)
{
    for (int i=0;i<rowCount();++i)
    {
        auto renderer = qobject_cast<MediaRenderer*>(at(i));
        if (renderer->netWorkAddress() == ip)
            return renderer;
    }

    return Q_NULLPTR;
}

void MediaRendererModel::setDlnaProfiles(Protocol *profiles)
{
    m_dlnaProfiles = profiles;
}

Protocol *MediaRendererModel::dlnaProfiles() const
{
    return m_dlnaProfiles;
}
