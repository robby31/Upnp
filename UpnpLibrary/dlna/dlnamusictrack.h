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
    qint64 bitrate() const Q_DECL_OVERRIDE;

    // Returns the mimeType of the source.
    QString sourceMimeType() const Q_DECL_OVERRIDE;

    // Returns the XML (DIDL) representation of the DLNA node.
    QDomElement getXmlContentDirectory(QDomDocument *xml, QStringList properties) Q_DECL_OVERRIDE;

protected:
    // Returns the process for transcoding
    QFfmpegTranscoding* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;
};

#endif // DLNAMUSICTRACK_H
