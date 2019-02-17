#ifndef DLNACACHEDNETWORKVIDEO_H
#define DLNACACHEDNETWORKVIDEO_H

#include "dlnacachedvideo.h"
#include "ffmpegtranscoding.h"
#include "../dlnanetworkvideo.h"

class DlnaCachedNetworkVideo : public DlnaCachedVideo
{
    Q_OBJECT

public:
    explicit DlnaCachedNetworkVideo(MediaLibrary* library, int idMedia, QObject *parent = Q_NULLPTR);

    //returns the size of the source
    qint64 sourceSize() const Q_DECL_OVERRIDE { return (double)metaDataDuration()*(double)metaDataBitrate()/8000.0; }

    QString metaDataTitle() const Q_DECL_OVERRIDE { if (library) return library->getmetaData("title", idMedia).toString(); return QString(); }

    // return true if the track shall be transcoded
    bool toTranscode() const Q_DECL_OVERRIDE { return true; }

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

private:
    QString m_streamUrl;
};

#endif // DLNACACHEDNETWORKVIDEO_H
