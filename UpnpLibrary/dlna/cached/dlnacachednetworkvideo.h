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
    ~DlnaCachedNetworkVideo() Q_DECL_OVERRIDE;

    QString metaDataTitle() const Q_DECL_OVERRIDE;

    // return true if the track shall be transcoded
    bool toTranscode() const Q_DECL_OVERRIDE { return true; }

protected:
    // Returns the process for transcoding
    TranscodeProcess* getTranscodeProcess() Q_DECL_OVERRIDE;

    // Returns the process for original streaming
    Device* getOriginalStreaming() Q_DECL_OVERRIDE;

private:
    QString m_streamUrl;

public:
    static qint64 objectCounter;
};

#endif // DLNACACHEDNETWORKVIDEO_H
