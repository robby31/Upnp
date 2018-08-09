#ifndef DLNAMUSICTRACK_H
#define DLNAMUSICTRACK_H

#include "dlnaitem.h"
#include "qffmpegtranscoding.h"

class DlnaMusicTrack : public DlnaItem
{
    Q_OBJECT

public:
    DlnaMusicTrack(QObject *parent = Q_NULLPTR);

    // Return upnp class
    QString getUpnpClass() const Q_DECL_OVERRIDE { return QString("object.item.audioItem.musicTrack"); }

    // returns the bitrate of the audio track
    int bitrate() const Q_DECL_OVERRIDE;

    // Returns the mimeType for this DLNA node.
    QString mimeType() const Q_DECL_OVERRIDE;

    // Returns the mimeType of the source.
    QString sourceMimeType() const Q_DECL_OVERRIDE;

    // Returns the XML (DIDL) representation of the DLNA node.
    QDomElement getXmlContentDirectory(QDomDocument *xml, QStringList properties) Q_DECL_OVERRIDE;

    // Returns album art in jpeg format
    QImage getAlbumArt() const Q_DECL_OVERRIDE;

    static const QString UNKNOWN_AUDIO_TYPEMIME;
    static const QString AUDIO_MP3_TYPEMIME;
    static const QString AUDIO_MP4_TYPEMIME;
    static const QString AUDIO_WAV_TYPEMIME;
    static const QString AUDIO_WMA_TYPEMIME;
    static const QString AUDIO_FLAC_TYPEMIME;
    static const QString AUDIO_OGG_TYPEMIME;
    static const QString AUDIO_LPCM_TYPEMIME;
    static const QString AUDIO_TRANSCODE;

protected:
    // Returns the process for transcoding
    QFfmpegTranscoding* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

private:
    void updateDLNAOrgPn() Q_DECL_OVERRIDE;
};

#endif // DLNAMUSICTRACK_H
