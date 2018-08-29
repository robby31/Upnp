#ifndef DLNACACHEDVIDEO_H
#define DLNACACHEDVIDEO_H

#include "../dlnavideoitem.h"
#include "ffmpegtranscoding.h"
#include "medialibrary.h"

class DlnaCachedVideo : public DlnaVideoItem
{
    Q_OBJECT

public:
    explicit DlnaCachedVideo(MediaLibrary* library, int idMedia, QObject *parent = Q_NULLPTR);

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return QString("Media(%1)").arg(idMedia); }

    QString getSystemName() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("filename", idMedia).toString(); return QString(); }

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE { return (double)metaDataDuration()*(double)metaDataBitrate()/8000.0; }

    // return true if the track shall be transcoded
    bool toTranscode() const Q_DECL_OVERRIDE { return true; }

    qint64 getResumeTime() const Q_DECL_OVERRIDE;

    qint64 metaDataBitrate()           const Q_DECL_OVERRIDE { if (library) return library->getmetaData("bitrate", idMedia).toLongLong(); return -1; }
    qint64 metaDataDuration()          const Q_DECL_OVERRIDE { if (library) return library->getmetaData("duration", idMedia).toInt(); return -1; }
    QString metaDataTitle()            const Q_DECL_OVERRIDE { return QFileInfo(getSystemName()).completeBaseName(); }
    QString metaDataGenre()            const Q_DECL_OVERRIDE { return ""; }
    QString metaDataPerformer()        const Q_DECL_OVERRIDE { return ""; }
    QString metaDataPerformerSort()    const Q_DECL_OVERRIDE { return ""; }
    QString metaDataAlbum()            const Q_DECL_OVERRIDE { return ""; }
    QString metaDataAlbumArtist()      const Q_DECL_OVERRIDE { return ""; }
    int metaDataYear()                 const Q_DECL_OVERRIDE { return -1; }
    int metaDataTrackPosition()        const Q_DECL_OVERRIDE { return 0; }
    int metaDataDisc()                 const Q_DECL_OVERRIDE { return 0; }
    QString metaDataFormat()           const Q_DECL_OVERRIDE { if (library) return library->getmetaData("format", idMedia).toString(); return QString(); }
    QByteArray metaDataPicture()       const Q_DECL_OVERRIDE { return QByteArray(); }
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("last_modified", idMedia).toString(); return QString(); }

    // returns the samplerate of the video track
    int samplerate() const Q_DECL_OVERRIDE;

    //returns the channel number of the video track
    int channelCount() const Q_DECL_OVERRIDE;

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

    QString resolution()            const Q_DECL_OVERRIDE { if (library) return library->getmetaData("resolution", idMedia).toString(); return QString(); }
    QStringList subtitleLanguages() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("subtitlelanguages", idMedia).toString().split(","); return QStringList(); }
    QStringList audioLanguages()    const Q_DECL_OVERRIDE { if (library) return library->getmetaData("audiolanguages", idMedia).toString().split(","); return QStringList(); }
    QString framerate()             const Q_DECL_OVERRIDE;

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

    MediaLibrary* library;
    int idMedia;
};

#endif // DLNACACHEDVIDEO_H
