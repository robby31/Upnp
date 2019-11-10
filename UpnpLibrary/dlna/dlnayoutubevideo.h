#ifndef DLNAYOUTUBEVIDEO_H
#define DLNAYOUTUBEVIDEO_H

#include <QTime>
#include <QRegularExpression>

#include "dlnavideoitem.h"
#include "youtube.h"
#include "ffmpegtranscoding.h"
#include "qffmpeginputmedia.h"

class DlnaYouTubeVideo : public DlnaVideoItem
{
    Q_OBJECT

public:
    explicit DlnaYouTubeVideo(QObject *parent = Q_NULLPTR);
    ~DlnaYouTubeVideo() Q_DECL_OVERRIDE;

    bool isValid() { return error().isEmpty() && !metaDataTitle().isEmpty() && !resolution().isEmpty() && metaDataDuration()>0; }

    QString error() const;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return metaDataTitle(); }

    QString getSystemName() const Q_DECL_OVERRIDE { return url().url();  }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return metaDataTitle(); }

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE;

    qint64 metaDataBitrate()           const Q_DECL_OVERRIDE { return ffmpeg.getBitrate(); }
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
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { return QString(); }

    // returns the samplerate of the video track
    int samplerate() const Q_DECL_OVERRIDE;

    //returns the channel number of the video track
    int channelCount() const Q_DECL_OVERRIDE;

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

    QString resolution() const Q_DECL_OVERRIDE;
    QStringList subtitleLanguages() const Q_DECL_OVERRIDE { return QStringList(); }
    QStringList audioLanguages() const Q_DECL_OVERRIDE { return QStringList(); }
    QString framerate() const Q_DECL_OVERRIDE;

    QUrl streamUrl() const { return m_streamUrl; }

    void setAnalyzeStream(const bool &flag) { m_analyzeStream = flag; }

    QUrl url() const;
    void setUrl(const QUrl &url);
    bool waitUrl(const unsigned long &timeout=30000);
    void setMaxVideoHeight(const int &height);

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

signals:
    void getVideoUrl(const QString &videoId);
    void videoUrlErrorSignal(const QString &message);
    void streamUrlDefined(const QString &url);

private slots:
    void videoUrl();

private:
    bool m_analyzeStream = true;
    QUrl m_streamUrl;

    QFfmpegInputMedia ffmpeg;

    Youtube m_youtube;

public:
    static qint64 objectCounter;
};

#endif // DLNAYOUTUBEVIDEO_H
