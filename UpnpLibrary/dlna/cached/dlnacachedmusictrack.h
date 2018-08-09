#ifndef DLNACACHEDMUSICTRACK_H
#define DLNACACHEDMUSICTRACK_H

#include "../dlnamusictrack.h"
#include "medialibrary.h"

class DlnaCachedMusicTrack : public DlnaMusicTrack
{
    Q_OBJECT

public:
    explicit DlnaCachedMusicTrack(MediaLibrary* library, int idMedia, QObject *parent = Q_NULLPTR);

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return QString("Media(%1)").arg(idMedia); }

    QString getSystemName() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("filename", idMedia).toString(); return QString(); }

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE { return (double)metaDataDuration()*(double)metaDataBitrate()/8000.0; }

    qint64 getResumeTime() const Q_DECL_OVERRIDE;

    qint64 metaDataBitrate()              const Q_DECL_OVERRIDE { if (library) return library->getmetaData("bitrate", idMedia).toLongLong(); return -1; }
    int metaDataDuration()             const Q_DECL_OVERRIDE { if (library) return library->getmetaData("duration", idMedia).toInt(); return -1; }
    QString metaDataTitle()            const Q_DECL_OVERRIDE { if (library) return library->getmetaData("title", idMedia).toString(); return QString(); }
    QString metaDataGenre()            const Q_DECL_OVERRIDE { if (library) return library->getmetaData("genre", idMedia).toString(); return QString(); }
    QString metaDataPerformer()        const Q_DECL_OVERRIDE { if (library) return library->getmetaData("artist", idMedia).toString(); return QString(); }
    QString metaDataPerformerSort()    const Q_DECL_OVERRIDE { if (library) return library->getmetaDataArtist("artist_sort", idMedia).toString(); return QString(); }
    QString metaDataAlbum()            const Q_DECL_OVERRIDE { if (library) return library->getmetaData("album", idMedia).toString(); return QString(); }
    QString metaDataAlbumArtist()      const Q_DECL_OVERRIDE { if (library) return library->getmetaDataAlbum("artist_name", idMedia).toString(); return QString(); }
    int metaDataYear()                 const Q_DECL_OVERRIDE { if (library) return library->getmetaDataAlbum("year", idMedia).toInt(); return -1; }
    int metaDataTrackPosition()        const Q_DECL_OVERRIDE { if (library) return library->getmetaData("trackposition", idMedia).toInt(); return 0; }
    int metaDataDisc()                 const Q_DECL_OVERRIDE { if (library) return library->getmetaData("disc", idMedia).toInt(); return 0; }
    QString metaDataFormat()           const Q_DECL_OVERRIDE { if (library) return library->getmetaData("format", idMedia).toString(); return QString(); }
    QByteArray metaDataPicture()       const Q_DECL_OVERRIDE { if (library) return QByteArray::fromHex(library->getmetaData("picture", idMedia).toByteArray()); return QByteArray(); }
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("last_modified", idMedia).toDateTime().toString("yyyy-MM-dd"); return QString(); }

    // returns the samplerate of the audio track
    int samplerate() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("samplerate", idMedia).toInt(); return -1; }

    //returns the channel number of the audio track
    int channelCount() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("channelcount", idMedia).toInt(); return -1; }

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

private:
    MediaLibrary* library;
    int idMedia;
};

#endif // DLNACACHEDMUSICTRACK_H
