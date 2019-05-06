#include "dlnacachedrootfolder.h"

DlnaCachedRootFolder::DlnaCachedRootFolder(QObject *parent):
    DlnaRootFolder(parent),
    library(this),
    rootFolder(this)
{
    recentlyPlayedChild = new DlnaCachedFolder(&library,
                                               library.getMedia("last_played is not null", "last_played", "DESC"),
                                               "Recently Played", true, 50, this);
    recentlyPlayedChild->setDlnaParent(this);
    addChild(recentlyPlayedChild);

    resumeChild = new DlnaCachedFolder(&library,
                                       library.getMedia("progress_played>0", "last_played", "DESC"),
                                       "Resume", true, 50, this);
    resumeChild->setDlnaParent(this);
    addChild(resumeChild);

    lastAddedChild = new DlnaCachedFolder(&library,
                                          library.getMedia("addedDate is not null", "addedDate", "DESC"),
                                          "Last Added", true, 50, this);
    lastAddedChild->setDlnaParent(this);
    addChild(lastAddedChild);

    favoritesChild = new DlnaCachedFolder(&library,
                                          library.getMedia("counter_played>0", "counter_played", "DESC"),
                                          "Favorites", true, 50, this);
    favoritesChild->setDlnaParent(this);
    addChild(favoritesChild);

    playlists = new DlnaCachedPlaylists(&library, this);
    addChild(playlists);

    QSqlQuery query = library.getMediaType();
    while (query.next()) {
        int id_type = query.value(0).toInt();
        QString typeMedia = query.value(1).toString();

        if (typeMedia == "video")
        {
            QString where = QString("media.type=%1 and is_reachable=1 and filename like '%youtube%'").arg(id_type);
            youtube = new DlnaCachedGroupedFolderMetaData(&library, "YOUTUBE", this);
            youtube->setDlnaParent(this);
            youtube->addFolder("SELECT DISTINCT artist.id, artist.name FROM media LEFT OUTER JOIN artist ON media.artist=artist.id WHERE " + where + " ORDER BY artist.name",
                               "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                               "from media "
                               "LEFT OUTER JOIN type ON media.type=type.id "
                               "WHERE " + where + " and artist%1 "
                               "ORDER BY title ASC",
                               "Artist");
            youtube->addFolder("SELECT DISTINCT album.id, album.name FROM media LEFT OUTER JOIN album ON media.album=album.id WHERE " + where + " ORDER BY album.name",
                               "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                               "from media "
                               "LEFT OUTER JOIN type ON media.type=type.id "
                               "WHERE " + where + " and album%1 "
                               "ORDER BY title ASC",
                               "Album");
            youtube->addFolder("SELECT DISTINCT genre.id, genre.name FROM media LEFT OUTER JOIN genre ON media.genre=genre.id WHERE " + where + "ORDER BY genre.name",
                               "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                               "from media "
                               "LEFT OUTER JOIN type ON media.type=type.id "
                               "WHERE " + where + " and genre%1 "
                               "ORDER BY title ASC",
                               "Genre");
            addChild(youtube);
        }

        if (typeMedia == "audio")
        {
            DlnaCachedGroupedFolderMetaData* child;
            child = new DlnaCachedGroupedFolderMetaData(&library, "Music", this);
            child->setDlnaParent(this);
            QString where = QString("media.type=%1 and is_reachable=1").arg(id_type);

            child->addFolder("SELECT DISTINCT artist.id, artist.name FROM media LEFT OUTER JOIN artist ON media.artist=artist.id WHERE " + where + " ORDER BY artist.name",
                             "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                             "from media "
                             "LEFT OUTER JOIN type ON media.type=type.id "
                             "WHERE " + where + " and artist%1 "
                             "ORDER BY album, disc, trackposition ASC",
                             "Artist");

            child->addFolder("SELECT DISTINCT album.id, album.name FROM media LEFT OUTER JOIN album ON media.album=album.id WHERE " + where + " ORDER BY album.name",
                             "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                             "from media "
                             "LEFT OUTER JOIN type ON media.type=type.id "
                             "WHERE " + where + " and album%1 "
                             "ORDER BY disc, trackposition ASC",
                             "Album");

            child->addFolder("SELECT DISTINCT genre.id, genre.name FROM media LEFT OUTER JOIN genre ON media.genre=genre.id WHERE " + where + " ORDER BY genre.name",
                             "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                             "from media "
                             "LEFT OUTER JOIN type ON media.type=type.id "
                             "WHERE " + where + " and genre%1 "
                             "ORDER BY album, disc, trackposition ASC",
                             "Genre");

            child->addFolder("SELECT DISTINCT artist.id, artist.name FROM media LEFT OUTER JOIN album ON media.album=album.id LEFT OUTER JOIN artist ON album.artist=artist.id WHERE " + where + " ORDER BY artist.name",
                             "SELECT media.id, media.filename, type.name AS type_media, media.last_modified, media.counter_played "
                             "from media "
                             "LEFT OUTER JOIN type ON media.type=type.id "
                             "LEFT OUTER JOIN album ON media.album=album.id "
                             "LEFT OUTER JOIN artist ON album.artist=artist.id "
                             "WHERE " + where + " and album.artist%1 "
                             "ORDER BY album, disc, trackposition ASC",
                             "Album Artist");
            addChild(child);
        } else {
            DlnaCachedFolder* child = new DlnaCachedFolder(&library,
                                                           library.getMedia(QString("type='%1'").arg(id_type), "title", "ASC"),
                                                           typeMedia, false, -1, this);
            child->setDlnaParent(this);
            addChild(child);
        }
    }

    addChild(&rootFolder);
}

bool DlnaCachedRootFolder::addFolderSlot(const QString &path)
{
    if (QFileInfo(path).isDir())
    {
        // scan the folder in background
        emit scanFolder(path);

        rootFolder.addFolder(path);

        emit folderAddedSignal(path);
        return true;
    }

    emit error_addFolder(path);
    return false;
}

void DlnaCachedRootFolder::addNetworkLink(const QString &url)
{
    auto playlist = new DlnaNetworkPlaylist(QUrl(url), this);
    if (playlist->isValid())
    {
        addPlaylist(playlist);
    }
    else
    {
        queueResource(QUrl(url));
    }

    playlist->deleteLater();

    if (!url_inProgress)
        addNextResource();
}

void DlnaCachedRootFolder::addPlaylist(DlnaNetworkPlaylist *playlist)
{
    if (!playlist)
    {
        emit error_addNetworkLink("playlist invalid");
        return;
    }

    int id_playlist = library.add_playlist(playlist->getName(), playlist->url());
    if (id_playlist == -1)
    {
        emit error_addNetworkLink("unable to create playlist");
        return;
    }

    for (int index=0;index<playlist->getChildrenSize();index++)
    {
        const AbstractPlaylist::T_URL *info = playlist->getMediaInfo(index);
        if (info)
            queueResource(info->url, info->title, id_playlist);
        else
            qCritical() << "invalid media in playlist, index =" << index;
    }

    playlists->needRefresh();

    emit linkAdded(QString("Playlist %1").arg(playlist->getName()));
}

void DlnaCachedRootFolder::addResource(const QUrl &url, const int &playlistId, const QString &title)
{
    url_inProgress = true;

    auto movie = new DlnaNetworkVideo(this);

    movie->setDlnaParent(this);
    connect(movie, &DlnaNetworkVideo::destroyed, this, &DlnaCachedRootFolder::addNextResource);
    connect(movie, &DlnaNetworkVideo::streamUrlDefined, this, &DlnaCachedRootFolder::networkLinkAnalyzed);
    connect(movie, &DlnaNetworkVideo::videoUrlErrorSignal, this, &DlnaCachedRootFolder::networkLinkError);
    if (movie->setUrl(url.url()))
    {
        movie->setMaxVideoHeight(720);

        if (playlistId != -1)
            movie->setProperty("playlistId", playlistId);

        if (!title.isNull())
            movie->setProperty("title", title);
    }
    else
    {
        qCritical() << "invalid url" << url;
        movie->deleteLater();
    }
}


void DlnaCachedRootFolder::networkLinkAnalyzed(const QList<QUrl> &urls)
{
    Q_UNUSED(urls)

    url_inProgress = false;

    auto movie = qobject_cast<DlnaNetworkVideo*>(sender());

    if (movie)
    {
        QHash<QString, QVariant> data;
        QHash<QString, QVariant> data_album;
        QHash<QString, QVariant> data_artist;

        data.insert("filename", movie->url());
        data.insert("type", "video");

        QString title = movie->metaDataTitle();
        if (movie->property("title").isValid() && movie->property("title").toString().size() > title.size())
            title = movie->property("title").toString();

        data.insert("title", title);
        data.insert("duration", movie->metaDataDuration());
        data.insert("resolution", movie->resolution());
        data.insert("samplerate", movie->samplerate());
        data.insert("channelcount", movie->channelCount());
        data.insert("audiolanguages", movie->audioLanguages().join(","));
        data.insert("subtitlelanguages", movie->subtitleLanguages().join(","));
        data.insert("framerate", movie->framerate());
        data.insert("bitrate", movie->metaDataBitrate());
        data.insert("format", movie->sourceContainer());
        data.insert("mime_type", movie->sourceMimeType());

        if (title.isEmpty())
        {
            qCritical() << QString("unable to add resource %1, title is empty").arg(movie->url().toString());
            emit error_addNetworkLink(movie->url().toString());
        }
        else
        {
            if (movie->metaDataDuration()<=0)
                qCritical() << QString("invalid duration %3 for %1 (%2).").arg(title, movie->url().toString()).arg(movie->metaDataDuration());
            if (movie->resolution().isEmpty())
                qCritical() << QString("invalid resolution %3 for %1 (%2).").arg(title, movie->url().toString(), movie->resolution());

            if (!data.isEmpty())
            {
                qDebug() << QString("Resource to add: %1").arg(title);
                int id_media = library.add_media(data, data_album, data_artist);
                if (id_media == -1)
                {
                    qCritical() << QString("unable to add or update resource %1 (%2)").arg(movie->url().toString(), "video");
                    emit error_addNetworkLink(title);
                }
                else
                {
                    lastAddedChild->needRefresh();

                    if (!movie->sourceAudioFormat().isEmpty())
                    {
                        if (!library.add_param(id_media, "audio_format", movie->sourceAudioFormat()))
                            qCritical() << "unable to add audio format" << id_media << movie->sourceAudioFormat();
                    }

                    if (!movie->sourceVideoFormat().isEmpty())
                    {
                        if (!library.add_param(id_media, "video_format", movie->sourceVideoFormat()))
                            qCritical() << "unable to add video format" << id_media << movie->sourceVideoFormat();
                    }

                    if (movie->thumbnailUrl().isValid())
                    {
                        // add thumbnail url
                        if (!library.add_param(id_media, "thumbnailUrl", movie->thumbnailUrl()))
                            qCritical() << "unable to add url picture" << id_media << movie->thumbnailUrl();
                    }

                    int playlistId = -1;
                    if (movie->property("playlistId").isValid())
                        playlistId = movie->property("playlistId").toInt();
                    if (playlistId != -1)
                    {
                        if (library.add_media_to_playlist(id_media, playlistId) == -1)
                            emit error_addNetworkLink("unable to add media to playlist");
                        else
                            emit linkAdded(title);
                    }
                    else
                    {
                        emit linkAdded(title);
                    }
                }
            }
        }

        movie->deleteLater();
    }
    else
    {
        qCritical() << "invalid sender" << sender();
        addNextResource();
    }
}

void DlnaCachedRootFolder::addNextResource()
{    
    if (!urlToAdd.isEmpty())
    {
        T_URL new_url = urlToAdd.takeFirst();
        addResource(new_url.url, new_url.playListId, new_url.title);
    }
    else
    {
        qInfo() << "no more resource to add";
    }
}

void DlnaCachedRootFolder::queueResource(const QUrl &url, const int &playlistId)
{
    T_URL new_url;
    new_url.url = url;
    new_url.playListId = playlistId;
    urlToAdd << new_url;
}

void DlnaCachedRootFolder::queueResource(const QUrl &url, const QString &title, const int &playlistId)
{
    T_URL new_url;
    new_url.url = url;
    new_url.title = title;
    new_url.playListId = playlistId;
    urlToAdd << new_url;
}

void DlnaCachedRootFolder::networkLinkError(const QString &message)
{
    url_inProgress = false;

    auto movie = qobject_cast<DlnaNetworkVideo*>(sender());
    if (movie)
    {
        qCritical() << "ERROR, link not added" << movie->url() << message;

        emit error_addNetworkLink(movie->url().toString());

        movie->deleteLater();
    }
    else
    {
        qCritical() << "ERROR, link not added" << message;
        qCritical() << "invalid sender" << sender();
        addNextResource();
    }
}

void DlnaCachedRootFolder::addResource(const QFileInfo &fileinfo) {
//    // check meta data
//    library.checkMetaData(fileinfo);

    if (!library.contains(fileinfo))
    {
        QString mime_type = mimeDb.mimeTypeForFile(fileinfo).name();

        QHash<QString, QVariant> data;
        QHash<QString, QVariant> data_album;
        QHash<QString, QVariant> data_artist;
        QString srcAudioFormat;
        QString srcVideoFormat;

        data.insert("filename", fileinfo.absoluteFilePath());
        data.insert("type", mime_type.split("/").at(0));
        data.insert("mime_type", mime_type);
        data.insert("last_modified", fileinfo.lastModified());
        data.insert("is_reachable", 1);

        if (mime_type.startsWith("audio/"))
        {
            DlnaMusicTrackFile track(fileinfo.absoluteFilePath());
            track.setDlnaParent(this);

            data.insert("title", track.metaDataTitle());
            data.insert("genre", track.metaDataGenre());
            data.insert("trackposition", track.metaDataTrackPosition());
            data.insert("disc", track.metaDataDisc());
            data.insert("duration", track.metaDataDuration());
            data.insert("samplerate", track.samplerate());
            data.insert("channelcount", track.channelCount());
            data.insert("picture", track.getByteAlbumArt());
            data.insert("format", track.sourceContainer());
            data.insert("bitrate", track.metaDataBitrate());

            srcAudioFormat = track.sourceAudioFormat();
            srcVideoFormat = track.sourceVideoFormat();

            if (!track.metaDataAlbum().isEmpty())
            {
                data_album["name"] = track.metaDataAlbum();
                data_album["artist"] = track.metaDataAlbumArtist();
                int year = track.metaDataYear();
                if (year != -1)
                    data_album["year"] = year;
            }

            if (!track.metaDataPerformer().isEmpty())
            {
                data_artist["name"] = track.metaDataPerformer();
                if (!track.metaDataPerformerSort().isEmpty())
                    data_artist["sortname"] = track.metaDataPerformerSort();
            }
        }
        else if (mime_type.startsWith("video/"))
        {
            DlnaVideoFile movie(fileinfo.absoluteFilePath());
            movie.setDlnaParent(this);

            data.insert("duration", movie.metaDataDuration());
            data.insert("resolution", movie.resolution());
            data.insert("samplerate", movie.samplerate());
            data.insert("channelcount", movie.channelCount());
            data.insert("audiolanguages", movie.audioLanguages().join(","));
            data.insert("subtitlelanguages", movie.subtitleLanguages().join(","));
            data.insert("framerate", movie.framerate());
            data.insert("bitrate", movie.metaDataBitrate());
            data.insert("format", movie.sourceContainer());

            srcAudioFormat = movie.sourceAudioFormat();
            srcVideoFormat = movie.sourceVideoFormat();

            if (!movie.metaDataAlbum().isEmpty())
            {
                data_album["name"] = movie.metaDataAlbum();
                data_album["artist"] = movie.metaDataAlbumArtist();
                int year = movie.metaDataYear();
                if (year != -1)
                    data_album["year"] = year;
            }

            if (!movie.metaDataPerformer().isEmpty())
            {
                data_artist["name"] = movie.metaDataPerformer();
                if (!movie.metaDataPerformerSort().isEmpty())
                    data_artist["sortname"] = movie.metaDataPerformerSort();
            }
        }
        else
        {
            qDebug() << "resource not added to library: " << mime_type << ", " << fileinfo.absoluteFilePath();
            data.clear();
        }

        if (!data.isEmpty())
        {
            int id_media = library.add_media(data, data_album, data_artist);
            if (id_media == -1)
            {
                qCritical() << QString("unable to add or update resource %1 (%2)").arg(fileinfo.absoluteFilePath(), mime_type);
            }
            else
            {
                if (!srcAudioFormat.isEmpty())
                {
                    if (!library.add_param(id_media, "audio_format", srcAudioFormat))
                        qCritical() << "unable to add audio format" << id_media << srcAudioFormat;
                }

                if (!srcVideoFormat.isEmpty())
                {
                    if (!library.add_param(id_media, "video_format", srcVideoFormat))
                        qCritical() << "unable to add video format" << id_media << srcVideoFormat;
                }
            }
        }
    }
}

void DlnaCachedRootFolder::readDirectory(const QDir &folder)
{
    QFileInfoList files = folder.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);

    foreach(const QFileInfo &fileinfo, files) {
        QString currentPath = fileinfo.absoluteFilePath();

        if(fileinfo.isDir()) {
            readDirectory(currentPath);
        } else if (fileinfo.isFile()) {
            addResource(fileinfo);
        }
    }

    lastAddedChild->needRefresh();
}

void DlnaCachedRootFolder::updateLibrary(const QString &filename, const QHash<QString, QVariant> &data)
{
    if (!library.updateFromFilename(filename, data))
    {
        qCritical() << "unable to update library" << filename << data;
    }
    recentlyPlayedChild->needRefresh();
    resumeChild->needRefresh();
    favoritesChild->needRefresh();
}

void DlnaCachedRootFolder::updateLibraryFromId(const int &id, const QHash<QString, QVariant> &data)
{
    if (!library.updateFromId(id, data))
        qCritical() << QString("Unable to update library: media id %1").arg(id);

    recentlyPlayedChild->needRefresh();
    resumeChild->needRefresh();
    favoritesChild->needRefresh();

    if (data.contains("filename"))
    {
        // refresh the media
        if (!library.isLocalUrl(data["filename"].toString()))
            addResource(QUrl(data["filename"].toString()));
        else
            addResource(QFileInfo(data["filename"].toString()));
    }
}

void DlnaCachedRootFolder::incrementCounterPlayed(const QString &filename)
{
    if (!library.incrementCounterPlayed(filename))
        qCritical() << QString("Unable to update counter played: %1").arg(filename);
    recentlyPlayedChild->needRefresh();
    resumeChild->needRefresh();
    favoritesChild->needRefresh();
}

void DlnaCachedRootFolder::reloadLibrary(const QStringList &localFolder)
{
    // save network media
    QList<QString> networkMedia;
    QSqlQuery query(library.database());
    if (query.exec("SELECT filename from media WHERE filename like 'http%' and is_reachable=1")) {
        while (query.next())
            networkMedia.append(query.value("filename").toString());
    } else {
        qCritical() << QString("Unable to load network media: %1").arg(query.lastError().text());
    }

    QString newDatabaseName = QString("%1.new").arg(library.databaseName());
    qWarning() << "RELOAD" << newDatabaseName;

    if (library.resetLibrary(newDatabaseName))
    {
        bool res = true;

        // load local folder
        foreach (const QString &folder, localFolder)
            if (!addFolderSlot(folder))
                res = false;

        // load network media
//        foreach (const QString &url, networkMedia)
//            if (!addNetworkLink(url))
//                res = false;

        if (!res)
            qCritical() << "Library reloaded with errors.";
        else
            qInfo() << "Library reloaded.";
    }
    else
    {
        qCritical() << "Unable to reload library";
    }
}
