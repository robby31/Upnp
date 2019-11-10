#ifndef DLNANETWORKVIDEO_H
#define DLNANETWORKVIDEO_H

#include "dlnavideoitem.h"
#include "ffmpegtranscoding.h"
#include "qffmpeginputmedia.h"
#include "mediastreaming.h"

class DlnaNetworkVideo  : public DlnaVideoItem
{
    Q_OBJECT

public:
    explicit DlnaNetworkVideo(QObject *parent = Q_NULLPTR);
    ~DlnaNetworkVideo() Q_DECL_OVERRIDE;

    bool isValid() const;

    QString error() const;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE;

    QString getSystemName() const Q_DECL_OVERRIDE;

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE;

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE;

    qint64 metaDataBitrate()           const Q_DECL_OVERRIDE;
    qint64 metaDataDuration()          const Q_DECL_OVERRIDE;
    QString metaDataTitle()            const Q_DECL_OVERRIDE;
    QString metaDataGenre()            const Q_DECL_OVERRIDE { return QString(); }
    QString metaDataPerformer()        const Q_DECL_OVERRIDE { return QString(); }
    QString metaDataPerformerSort()    const Q_DECL_OVERRIDE { return QString(); }
    QString metaDataAlbum()            const Q_DECL_OVERRIDE { return QString(); }
    QString metaDataAlbumArtist()      const Q_DECL_OVERRIDE { return QString(); }
    int metaDataYear()                 const Q_DECL_OVERRIDE { return -1; }
    int metaDataTrackPosition()        const Q_DECL_OVERRIDE { return 0; }
    int metaDataDisc()                 const Q_DECL_OVERRIDE { return 0; }
    QString metaDataFormat()           const Q_DECL_OVERRIDE;
    QByteArray metaDataPicture()             Q_DECL_OVERRIDE { return QByteArray(); }
    QUrl thumbnailUrl() const Q_DECL_OVERRIDE;
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { return QString(); }

    QString sourceContainer() const Q_DECL_OVERRIDE;
    QString sourceAudioFormat() const Q_DECL_OVERRIDE;
    QString sourceVideoFormat() const Q_DECL_OVERRIDE;

    // returns the samplerate of the video track
    int samplerate() const Q_DECL_OVERRIDE;

    //returns the channel number of the video track
    int channelCount() const Q_DECL_OVERRIDE;

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

    QString resolution() const Q_DECL_OVERRIDE;
    QStringList subtitleLanguages() const Q_DECL_OVERRIDE { return QStringList(); }
    QStringList audioLanguages() const Q_DECL_OVERRIDE { return QStringList(); }
    QString framerate() const Q_DECL_OVERRIDE;

    QList<QUrl> mediaUrl() const;

    void setAnalyzeStream(const bool &flag);

    QUrl url() const;
    bool setUrl(const QUrl &url);
    bool waitUrl(const unsigned long &timeout=30000);
    void setMaxVideoHeight(const int &height);

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

signals:
    void videoUrlErrorSignal(const QString &message);
    void streamUrlDefined(const QList<QUrl> &url);

private slots:
    void parse_video();

private:
    bool m_analyzeStream = true;
    QUrl m_url;
    QList<QFfmpegInputMedia*> ffmpeg;
    AbstractMedia *m_media = Q_NULLPTR;
    QString m_error;

public:
    static qint64 objectCounter;
};

#endif // DLNANETWORKVIDEO_H
