#ifndef DLNAVIDEOFILE_H
#define DLNAVIDEOFILE_H

#include "dlnavideoitem.h"
#include "ffmpegtranscoding.h"
#include "qffmpeginputmedia.h"

class DlnaVideoFile : public DlnaVideoItem
{
    Q_OBJECT

public:
    explicit DlnaVideoFile(const QString& filename, QObject *parent = Q_NULLPTR);
    ~DlnaVideoFile() Q_DECL_OVERRIDE;

    QFileInfo getFileInfo() const { return fileinfo; }

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return fileinfo.fileName(); }

    QString getSystemName() const Q_DECL_OVERRIDE { return fileinfo.absoluteFilePath(); }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return fileinfo.completeBaseName(); }

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE { return fileinfo.size(); }

    qint64 metaDataBitrate() const Q_DECL_OVERRIDE;
    qint64 metaDataDuration() const Q_DECL_OVERRIDE;
    QString metaDataTitle() const Q_DECL_OVERRIDE;
    QString metaDataGenre() const Q_DECL_OVERRIDE;
    QString metaDataPerformer() const Q_DECL_OVERRIDE;
    QString metaDataPerformerSort() const Q_DECL_OVERRIDE;
    QString metaDataAlbum() const Q_DECL_OVERRIDE;
    QString metaDataAlbumArtist() const Q_DECL_OVERRIDE;
    int metaDataYear() const Q_DECL_OVERRIDE;
    int metaDataTrackPosition() const Q_DECL_OVERRIDE;
    int metaDataDisc() const Q_DECL_OVERRIDE;
    QString metaDataFormat() const Q_DECL_OVERRIDE;
    QByteArray metaDataPicture() const Q_DECL_OVERRIDE;
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { return fileinfo.lastModified().toString("yyyy-MM-dd"); }

    // returns the samplerate of the video track
    int samplerate() const Q_DECL_OVERRIDE;

    //returns the channel number of the video track
    int channelCount() const Q_DECL_OVERRIDE;

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

    QString resolution() const Q_DECL_OVERRIDE;
    QStringList subtitleLanguages() const Q_DECL_OVERRIDE;
    QStringList audioLanguages() const Q_DECL_OVERRIDE;
    QString framerate() const Q_DECL_OVERRIDE;

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

private:
    QFileInfo fileinfo;
    QMimeType mime_type;
    QFfmpegInputMedia ffmpeg;

public:
    static qint64 objectCounter;
};

#endif // DLNAVIDEOFILE_H
