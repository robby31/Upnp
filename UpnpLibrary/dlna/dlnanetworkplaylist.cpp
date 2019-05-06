#include "dlnanetworkplaylist.h"

qint64 DlnaNetworkPlaylist::objectCounter = 0;

DlnaNetworkPlaylist::DlnaNetworkPlaylist(const QUrl &url, QObject *parent):
    DlnaStorageFolder(parent)
{
    ++objectCounter;

    QElapsedTimer timer;
    timer.start();
    MediaStreaming streaming;
    m_playlist = streaming.get_playlist(url);
    if (m_playlist)
    {
        m_playlist->waitReady(5000);
        qDebug() << "playlist" << url << "created in" << timer.elapsed() << m_playlist->mediaUrl().size() << "medias.";
    }
}

DlnaNetworkPlaylist::~DlnaNetworkPlaylist() {
    --objectCounter;
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
        return m_playlist->id();

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
        QElapsedTimer timer;
        timer.start();

        if (l_children.contains(index))
            return l_children[index];

        auto child = new DlnaNetworkVideo(this);

        child->setUrl(m_playlist->mediaUrl().at(index));
        child->setMaxVideoHeight(720);

        if (!child->waitUrl(5000))
            qWarning() << "network media not ready" << child;

        child->setId(QString("%1").arg(index+1));
        child->setDlnaParent(this);

        l_children[index] = child;

        qDebug() << "playlist, child created in" << timer.elapsed();
        return child;
    }

    return Q_NULLPTR;
}

