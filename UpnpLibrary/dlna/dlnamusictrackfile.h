#ifndef DLNAMUSICTRACKFILE_H
#define DLNAMUSICTRACKFILE_H

#include <QFileInfo>

#include "dlnamusictrack.h"
#include "qffmpeginputmedia.h"

class DlnaMusicTrackFile : public DlnaMusicTrack
{
    Q_OBJECT

public:
    explicit DlnaMusicTrackFile(const QString& filename, QObject *parent = Q_NULLPTR);

    QFileInfo getFileInfo() const { return fileinfo; }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return fileinfo.fileName(); }

    QString getSystemName() const Q_DECL_OVERRIDE { return fileinfo.absoluteFilePath(); }

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
    QByteArray metaDataPicture() Q_DECL_OVERRIDE;
    QString metaDataLastModifiedDate() const Q_DECL_OVERRIDE { return fileinfo.lastModified().toString("yyyy-MM-dd"); }

    QString sourceContainer() const Q_DECL_OVERRIDE;
    QString sourceAudioFormat() const Q_DECL_OVERRIDE;
    QString sourceVideoFormat() const Q_DECL_OVERRIDE;

    // returns the samplerate of the audio track
    int samplerate() const Q_DECL_OVERRIDE;

    //returns the channel number of the audio track
    int channelCount() const Q_DECL_OVERRIDE;

    QHash<QString, double> volumeInfo(const int& timeout = 30000) Q_DECL_OVERRIDE;

protected:
    // Returns the process for transcoding
    QFfmpegTranscoding* getTranscodeProcess() Q_DECL_OVERRIDE;

private:
    QFileInfo fileinfo;
    QFfmpegInputMedia ffmpeg;
};

#endif // DLNAMUSICTRACKFILE_H
