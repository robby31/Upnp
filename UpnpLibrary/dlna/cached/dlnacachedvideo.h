#ifndef DLNACACHEDVIDEO_H
#define DLNACACHEDVIDEO_H

#include "../dlnavideoitem.h"
#include "ffmpegtranscoding.h"
#include "medialibrary.h"
#include "mynetwork.h"
#include <QNetworkReply>

class DlnaCachedVideo : public DlnaVideoItem
{
    Q_OBJECT

public:
    explicit DlnaCachedVideo(MediaLibrary* library, int idMedia, QObject *parent = Q_NULLPTR);

    int idMedia() const { return m_idMedia; }
    MediaLibrary *library() const { return m_library; }

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return QString("Media(%1)").arg(m_idMedia); }

    QString getSystemName() const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("filename", m_idMedia).toString(); return QString(); }

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE { return static_cast<qint64>(static_cast<double>(metaDataDuration())*static_cast<double>(metaDataBitrate())/8000.0); }

    // return true if the track shall be transcoded
    bool toTranscode() const Q_DECL_OVERRIDE { return true; }

    qint64 getResumeTime() const Q_DECL_OVERRIDE;

    qint64 metaDataBitrate()           const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("bitrate", m_idMedia).toLongLong(); return -1; }
    qint64 metaDataDuration()          const Q_DECL_OVERRIDE;
    QString metaDataTitle()            const Q_DECL_OVERRIDE { return QFileInfo(getSystemName()).completeBaseName(); }
    QString metaDataGenre()            const Q_DECL_OVERRIDE { return ""; }
    QString metaDataPerformer()        const Q_DECL_OVERRIDE { return ""; }
    QString metaDataPerformerSort()    const Q_DECL_OVERRIDE { return ""; }
    QString metaDataAlbum()            const Q_DECL_OVERRIDE { return ""; }
    QString metaDataAlbumArtist()      const Q_DECL_OVERRIDE { return ""; }
    int metaDataYear()                 const Q_DECL_OVERRIDE { return -1; }
    int metaDataTrackPosition()        const Q_DECL_OVERRIDE { return 0; }
    int metaDataDisc()                 const Q_DECL_OVERRIDE { return 0; }
    QString metaDataFormat()           const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("format", m_idMedia).toString(); return QString(); }
    QByteArray metaDataPicture()             Q_DECL_OVERRIDE;
    QUrl thumbnailUrl() const Q_DECL_OVERRIDE;
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("last_modified", m_idMedia).toString(); return QString(); }

    QString sourceContainer() const Q_DECL_OVERRIDE;
    QString sourceAudioFormat() const Q_DECL_OVERRIDE;
    QString sourceVideoFormat() const Q_DECL_OVERRIDE;

    // returns the samplerate of the video track
    int samplerate() const Q_DECL_OVERRIDE;

    //returns the channel number of the video track
    int channelCount() const Q_DECL_OVERRIDE;

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

    QString resolution()            const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("resolution", m_idMedia).toString(); return QString(); }
    QStringList subtitleLanguages() const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("subtitlelanguages", m_idMedia).toString().split(","); return QStringList(); }
    QStringList audioLanguages()    const Q_DECL_OVERRIDE { if (m_library) return m_library->getmetaData("audiolanguages", m_idMedia).toString().split(","); return QStringList(); }
    QString framerate()             const Q_DECL_OVERRIDE;

private slots:
    void pictureReceived();

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

private:
    MediaLibrary* m_library;
    int m_idMedia;

    QNetworkReply *m_replyPicture = Q_NULLPTR;
    QByteArray m_picture;
};

#endif // DLNACACHEDVIDEO_H
