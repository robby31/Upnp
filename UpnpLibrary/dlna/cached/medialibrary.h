#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include <QObject>
#include <QtSql>

#include "mysqldatabase.h"
//#include "acoustid.h"

class MediaLibrary : public QObject
{
    Q_OBJECT

    typedef QHash<QString, QHash<QString, QVariant> > StateType;

public:
    explicit MediaLibrary(QObject *parent = Q_NULLPTR);
    ~MediaLibrary() Q_DECL_OVERRIDE;

    bool isValid();

    QSqlDatabase database() const;
    QString databaseName() const;

    bool isLocalUrl(const QString &url);

    QSqlQuery getMediaType() const { return QSqlQuery("SELECT DISTINCT id, name FROM type", database()); }

    QSqlQuery getMedia(const QString &where, const QString &orderParam="media.id", const QString &sortOption="ASC") const;
    int countMedia(const QString &where) const;

    QSqlQuery getAllNetworkLinks() { return QSqlQuery("SELECT id, filename, title, artist, is_reachable from media WHERE filename like 'http%' or filename like 'francetv:%' or filename like 'wat:%'", database()); }
    QSqlQuery getAllPlaylists() { return QSqlQuery("SELECT * FROM playlists", database()); }

    QSqlQuery getDistinctMetaData(const int &typeMedia, const QString &tagName, const QString &where = QString()) const;
    int countDistinctMetaData(const int &typeMedia, const QString &tagName) const;

    QVariant getmetaData(const QString &tagName, const int &idMedia) const;
    QVariant getmetaDataAlbum(const QString &tagName, const int &idMedia) const;
    QVariant getmetaDataArtist(const QString &tagName, const int &idMedia) const;

    QHash<QString, double> volumeInfo(const int &idMedia);
    bool setVolumeInfo(const int& idMedia, const QHash<QString, double>& info);

//    void checkMetaData(const QFileInfo &fileinfo) const;

    bool contains(const QFileInfo &fileinfo) const;
    int add_media(QHash<QString, QVariant> data, const QHash<QString, QVariant>& data_album, const QHash<QString, QVariant>& data_artist);
    int add_album(QHash<QString, QVariant> data_album);
    int add_artist(QHash<QString, QVariant> data_artist);
    int add_playlist(const QString &name, const QUrl &url = QUrl());
    int add_media_to_playlist(const int &mediaId, const int &playlistId);
    bool updateFromFilename(const QString &filename, const QHash<QString, QVariant> &data);
    bool updateFromId(const int &id, const QHash<QString, QVariant> &data);
    bool incrementCounterPlayed(const QString &filename);

    bool resetLibrary(const QString &pathname);

private:
    bool initialize();

    int insertForeignKey(const QString &table, const QString &parameter, const QVariant &value);

    int insert(const QString &table, const QHash<QString, QVariant> &data);
    int update(const QString &table, const int &id, const QHash<QString, QVariant> &data);

    // export attributes not stored in media file such as last_played, counter_played, progress_played.
    StateType *exportMediaState() const;

signals:

private:
    QHash<QString, QHash<QString, QHash<QString, QString> > > foreignKeys;
    StateType *libraryState = Q_NULLPTR;

//    Acoustid m_acoustId;
};

#endif // MEDIALIBRARY_H
