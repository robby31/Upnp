#include "dlnacachedplaylists.h"

qint64 DlnaCachedPlaylists::objectCounter = 0;

DlnaCachedPlaylists::DlnaCachedPlaylists(MediaLibrary* library, QObject *parent):
    DlnaStorageFolder(parent),
    library(library)
{
    ++objectCounter;

    needRefresh();

    if (library)
    {
        query = QSqlQuery(library->database());
        query.prepare("SELECT DISTINCT playlists.id, playlists.name FROM playlists "
                      "LEFT OUTER JOIN media_in_playlists ON media_in_playlists.playlist=playlists.id "
                      "LEFT OUTER JOIN media ON media.id=media_in_playlists.media "
                      "LEFT OUTER JOIN type ON media.type=type.id "
                      "WHERE media.is_reachable=1");
    }
}

DlnaCachedPlaylists::~DlnaCachedPlaylists()
{
    --objectCounter;
}

QString DlnaCachedPlaylists::getName() const
{
    return QString("Playlists");
}

QString DlnaCachedPlaylists::getDisplayName() const
{
    return getName();
}

QString DlnaCachedPlaylists::getSystemName() const
{
    return getName();
}

int DlnaCachedPlaylists::getChildrenSize() const
{
    return nbChildren;
}

void DlnaCachedPlaylists::refreshContent()
{

    if (!query.exec())
    {
        qCritical() << "unable to refresh" << getName();
        return;
    }

    if (query.last())
        nbChildren = query.at() + 1;
    else
        nbChildren = 0;

    emit dlnaContentUpdated();
}

DlnaResource *DlnaCachedPlaylists::getChild(int index, QObject *parent)
{
    DlnaResource* child = Q_NULLPTR;

    if (query.seek(index))
    {
        QString name = query.value("name").toString();

        QSqlQuery newQuery(QString("SELECT media.id, media.filename, type.name AS type_media FROM playlists "
                                   "LEFT OUTER JOIN media_in_playlists ON media_in_playlists.playlist=playlists.id "
                                   "LEFT OUTER JOIN media ON media.id=media_in_playlists.media "
                                   "LEFT OUTER JOIN type ON media.type=type.id "
                                   "WHERE playlists.name='%1' and media.is_reachable=1").arg(name),
                           library->database());

        child = new DlnaCachedFolder(library, newQuery, name,
                                     false, -1, parent != Q_NULLPTR ? parent : this);

        child->setId(QString("%1").arg(index+1));
        child->setDlnaParent(this);
    }

    return child;
}
