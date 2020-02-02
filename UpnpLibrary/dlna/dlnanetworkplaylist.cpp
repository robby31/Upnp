#include "dlnanetworkplaylist.h"

DlnaNetworkPlaylist::DlnaNetworkPlaylist(const QUrl &url, QObject *parent):
    DlnaStorageFolder(parent)
{
#if !defined(QT_NO_DEBUG_OUTPUT)
    QElapsedTimer timer;
    timer.start();
#endif

    MediaStreaming streaming;
    m_playlist = streaming.get_playlist(url);
    if (m_playlist)
    {
        m_playlist->setParent(this);
        m_playlist->waitReady(5000);

#if !defined(QT_NO_DEBUG_OUTPUT)
        qDebug() << "playlist" << url << "created in" << timer.elapsed() << "ms," << m_playlist->mediaUrl().size() << "medias.";
#endif
    }
}

bool DlnaNetworkPlaylist::isValid() const
{
    return m_playlist != Q_NULLPTR && m_playlist->isValid();
}

QUrl DlnaNetworkPlaylist::url() const
{
    if (m_playlist)
        return m_playlist->url();

    return QUrl();
}

QString DlnaNetworkPlaylist::getName() const
{
    if (m_playlist)
        return m_playlist->title();

    return QString("Playlist");
}

QString DlnaNetworkPlaylist::getSystemName() const
{
    return getName();
}

QString DlnaNetworkPlaylist::getDisplayName() const
{
    return getName();
}

int DlnaNetworkPlaylist::getChildrenSize() const
{
    if (m_playlist)
        return m_playlist->mediaUrl().size();

    return 0;
}

const AbstractPlaylist::T_URL *DlnaNetworkPlaylist::getMediaInfo(const int &index)
{
    if (m_playlist && index >=0 && index < getChildrenSize())
        return m_playlist->media(index);

    return Q_NULLPTR;
}

DlnaResource *DlnaNetworkPlaylist::getChild(int index, QObject *parent)
{
    Q_UNUSED(parent)

    if (m_playlist && index >=0 && index < getChildrenSize())
    {
#if !defined(QT_NO_DEBUG_OUTPUT)
        QElapsedTimer timer;
        timer.start();
#endif

        if (l_children.contains(index))
            return l_children[index];

        auto child = new DlnaNetworkVideo(this);

        child->setUrl(m_playlist->mediaUrl().at(index));
        child->setMaxVideoHeight(720);

        if (!child->waitUrl(5000))
            qWarning() << "network media not ready" << child;

        if (!child->isValid())
            qWarning() << "invalid media" << child->getSystemName() << child->error();

        child->setId(QString("%1").arg(index+1));
        child->setDlnaParent(this);

        l_children[index] = child;

#if !defined(QT_NO_DEBUG_OUTPUT)
        qDebug() << "playlist, child created in" << timer.elapsed();
#endif
        return child;
    }

    return Q_NULLPTR;
}

