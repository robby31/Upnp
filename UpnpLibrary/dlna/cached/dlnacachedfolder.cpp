#include "dlnacachedfolder.h"

qint64 DlnaCachedFolder::objectCounter = 0;

DlnaCachedFolder::DlnaCachedFolder(MediaLibrary* library, const QSqlQuery &query, const QString& name, bool cacheEnabled, int maxSize, QObject *parent):
    DlnaStorageFolder(parent),
    library(library),
    name(name),
    query(query),
    nbChildren(-1),
    cacheEnabled(cacheEnabled),
    limitSizeMax(maxSize)
{
    ++objectCounter;

    needRefresh();
}

DlnaCachedFolder::~DlnaCachedFolder()
{
    --objectCounter;
}

int DlnaCachedFolder::getChildrenSize() const
{
    return nbChildren;
}

void DlnaCachedFolder::refreshContent()
{
    cache.clear();

    if (query.isSelect() && query.isActive() && library)
    {
        query = QSqlQuery(query.executedQuery(), library->database());

        if (cacheEnabled) {
            nbChildren = 0;
            while (query.next()) {
                ++nbChildren;
                cache.append(query.value("id").toInt());
                if (nbChildren==limitSizeMax)
                    break;
            }
        } else {
            if (query.last())
                nbChildren = query.at() + 1;
            else
                nbChildren = 0;
            if (limitSizeMax>0 && nbChildren>limitSizeMax)
                nbChildren = limitSizeMax;
        }
    }

    emit dlnaContentUpdated();
}

DlnaResource *DlnaCachedFolder::getChild(int index, QObject *parent) {
    DlnaResource* child = Q_NULLPTR;
    int id_media = -1;
    QString type_media;
    QString filename;

    if (cacheEnabled)
    {
        if (index>=0 && index<cache.count())
        {
            id_media = cache.at(index);
            QSqlQuery newQuery = library->getMedia(QString("media.id=%1").arg(id_media));
            if (newQuery.isActive() && newQuery.next())
            {
                type_media = newQuery.value("type_media").toString();
                filename = newQuery.value("filename").toString();
            }
        }
    } else {
        if (query.seek(index))
        {
            id_media = query.value("id").toInt();
            type_media = query.value("type_media").toString();
            filename = query.value("filename").toString();
        }
    }

    if (type_media == "audio")
    {
        child = new DlnaCachedMusicTrack(library, id_media,
                                         parent != Q_NULLPTR ? parent : this);

    } else if (type_media == "video")
    {
        if (library && !library->isLocalUrl(filename))
        {
            child= new DlnaCachedNetworkVideo(library, id_media,
                                              parent != Q_NULLPTR ? parent : this);
        }
        else
        {
            child = new DlnaCachedVideo(library, id_media,
                                        parent != Q_NULLPTR ? parent : this);
        }

    } else {
        qWarning() << QString("Unkwown format %1: %2").arg(type_media, filename);
    }


    if (child)
    {
        child->setId(QString("%1").arg(index+1));
        child->setDlnaParent(this);
    }

    return child;
}

