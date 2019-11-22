#ifndef DLNAITEM_H
#define DLNAITEM_H

#include "dlnaresource.h"
#include "protocol.h"

#include "transcodeprocess.h"
#include "streamingfile.h"


class DlnaItem : public DlnaResource
{
    Q_OBJECT

    Q_DISABLE_COPY_MOVE(DlnaItem)

public:
    explicit DlnaItem(QObject *parent = Q_NULLPTR);

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE;

    bool isFolder() const Q_DECL_OVERRIDE { return false; }
    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE { Q_UNUSED(index) Q_UNUSED(parent) return Q_NULLPTR; }
    int getChildrenSize() const Q_DECL_OVERRIDE { return 0; }

    // Returns the time where the media has been stopped during last play
    virtual qint64 getResumeTime() const { return 0; }

    // Returns an InputStream of this DLNA node.
    Device *getStream();

    // return true if the track shall be transcoded
    virtual bool toTranscode() const;

    TranscodeFormatAvailable format() const { return transcodeFormat; }
    void setTranscodeFormat(TranscodeFormatAvailable format);

    // Returns the mimeType for this DLNA node.
    QString mimeType() const;

    // Returns the mimeType of the source.
    virtual QString sourceMimeType() const = 0;

    // returns the size of the dlna item
    virtual qint64 size();

    //returns the size of the source
    virtual qint64 sourceSize() const = 0;

    // return the length in seconds of the media
    int getLengthInSeconds() const;
    qint64 getLengthInMilliSeconds() const;

    // returns the samplerate of the audio track
    virtual int samplerate() const = 0;

    //returns the channel number of the audio track
    virtual int channelCount() const = 0;

    // returns the bitrate in bits/sec
    virtual qint64 bitrate() const = 0;

    // returns volume information
    virtual QHash<QString, double> volumeInfo(const int& timeout = 30000) = 0;

    virtual qint64 metaDataBitrate() const = 0;
    virtual qint64 metaDataDuration() const = 0;
    virtual QString metaDataTitle() const = 0;
    virtual QString metaDataGenre() const = 0;
    virtual QString metaDataPerformer() const = 0;
    virtual QString metaDataPerformerSort() const = 0;
    virtual QString metaDataAlbum() const = 0;
    virtual QString metaDataAlbumArtist() const = 0;
    virtual int metaDataYear() const = 0;
    virtual int metaDataTrackPosition() const = 0;
    virtual int metaDataDisc() const = 0;
    virtual QString metaDataFormat() const = 0;
    virtual QByteArray metaDataPicture() = 0;
    virtual QUrl thumbnailUrl() const;
    virtual QString metaDataLastModifiedDate() const = 0;

    QString container() const;
    virtual QString sourceContainer() const = 0;

    QString audioFormat() const;
    virtual QString sourceAudioFormat() const = 0;

    QString videoFormat() const;
    virtual QString sourceVideoFormat() const = 0;

    // Returns album art in jpeg format
    QImage getAlbumArt() Q_DECL_OVERRIDE;

    QString getProtocolInfo() const;

    QString getDlnaContentFeatures() const;

    QString getdlnaOrgOpFlags() const { return dlnaOrgOpFlags; }
    void setdlnaOrgOpFlags (const QString& arg) { dlnaOrgOpFlags=arg; }

    QString getdlnaOrgPN() const { return dlnaOrgPN; }
    void setdlnaOrgPN(const QString& arg) { dlnaOrgPN=arg; }

    void setUserAgent(const QString& userAgent) { m_userAgent = userAgent; emit userAgentChanged(); }

    void setStream(Device *stream);

    void setDlnaProfiles(Protocol *profiles);
    Protocol *sinkProtocol() const;
    bool isSourceSinkCompatible() const;

    static const QString UNKNOWN_AUDIO_TYPEMIME;
    static const QString AUDIO_MP3_TYPEMIME;
    static const QString AUDIO_MP4_TYPEMIME;
    static const QString AUDIO_WMA_TYPEMIME;
    static const QString AUDIO_FLAC_TYPEMIME;
    static const QString AUDIO_OGG_TYPEMIME;
    static const QString AUDIO_LPCM_TYPEMIME;
    static const QString AUDIO_TRANSCODE;

    static const QString UNKNOWN_VIDEO_TYPEMIME;
    static const QString MPEG_TYPEMIME;
    static const QString MP4_TYPEMIME;
    static const QString AVI_TYPEMIME;
    static const QString WMV_TYPEMIME;
    static const QString ASF_TYPEMIME;
    static const QString MATROSKA_TYPEMIME;
    static const QString M3U8_TYPEMIME;
    static const QString MPEGTS_TYPEMIME;
    static const QString VIDEO_TRANSCODE;

private:
    ProtocolInfo *getSink(const QString &dlna_org_pn = QString());

protected:
    // Returns the process for transcoding
    virtual TranscodeDevice* getTranscodeProcess() = 0;

    // Returns the process for original streaming
    virtual Device* getOriginalStreaming() = 0;

signals:
    void userAgentChanged();

private slots:
    void streamDestroyed(QObject *obj);

private:
    TranscodeFormatAvailable transcodeFormat = UNKNOWN;  // default transcode format

    QString dlnaOrgOpFlags;

    // DLNA.ORG_PN
    QString dlnaOrgPN;

    QString m_userAgent;

    Device *m_stream = Q_NULLPTR;

    Protocol *m_sinkProtocol = Q_NULLPTR;
    ProtocolInfo *m_compatibleSink = Q_NULLPTR;
    QString m_protocolInfo;
};

#endif // DLNAITEM_H
