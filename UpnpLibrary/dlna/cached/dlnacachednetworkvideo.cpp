#include "dlnacachednetworkvideo.h"

DlnaCachedNetworkVideo::DlnaCachedNetworkVideo(MediaLibrary* p_library, int p_idMedia, QObject *parent):
    DlnaCachedVideo(p_library, p_idMedia, parent)
{
}

TranscodeProcess *DlnaCachedNetworkVideo::getTranscodeProcess()
{
    FfmpegTranscoding* transcodeProcess = Q_NULLPTR;

    QString sysName = getSystemName();

    if (library() && !library()->isLocalUrl(sysName))
    {
        transcodeProcess = new FfmpegTranscoding();

        // request from network url for streaming
        auto movie = new DlnaNetworkVideo(transcodeProcess);
        movie->setDlnaParent(this);
        connect(movie, &DlnaNetworkVideo::streamUrlDefined, transcodeProcess, &FfmpegTranscoding::setUrls);
        connect(movie, SIGNAL(videoUrlErrorSignal(QString)), transcodeProcess, SLOT(urlError(QString)));
        movie->setAnalyzeStream(false);
        movie->setUrl(sysName);
        movie->setMaxVideoHeight(720);
    }
    else if (!m_streamUrl.isEmpty())
    {
        // local video
        transcodeProcess = new FfmpegTranscoding();
        transcodeProcess->setUrl(m_streamUrl);
    }

    if (transcodeProcess)
    {
        transcodeProcess->setOriginalLengthInMSeconds(metaDataDuration());
        transcodeProcess->setFormat(format());
        transcodeProcess->setVariableBitrate(true);
        transcodeProcess->setBitrate(bitrate());
        transcodeProcess->setAudioLanguages(audioLanguages());
        transcodeProcess->setSubtitleLanguages(subtitleLanguages());
        transcodeProcess->setFrameRate(framerate());
        transcodeProcess->setAudioChannelCount(channelCount());
        transcodeProcess->setAudioSampleRate(samplerate());
//        transcodeProcess->setVolumeInfo(volumeInfo());

        return transcodeProcess;
    }

    qCritical() << "No stream to return.";
    return Q_NULLPTR;
}

Device *DlnaCachedNetworkVideo::getOriginalStreaming()
{
    QString sysName = getSystemName();
    if (library() && !library()->isLocalUrl(sysName))
    {
        TranscodeProcess *process = getTranscodeProcess();
        process->setFormat(COPY);
        return process;
    }

    return new StreamingFile(getSystemName());
}

QString DlnaCachedNetworkVideo::metaDataTitle() const
{
    if (library())
        return library()->getmetaData("title", idMedia()).toString();

    return QString();
}
