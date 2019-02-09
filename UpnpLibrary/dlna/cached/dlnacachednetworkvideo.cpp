#include "dlnacachednetworkvideo.h"

DlnaCachedNetworkVideo::DlnaCachedNetworkVideo(QNetworkAccessManager *manager, MediaLibrary* library, int idMedia, QObject *parent):
    DlnaCachedVideo(library, idMedia, parent),
    m_nam(manager),
    m_streamUrl()
{
}

TranscodeProcess *DlnaCachedNetworkVideo::getTranscodeProcess()
{
    FfmpegTranscoding* transcodeProcess = Q_NULLPTR;

    QString sysName = getSystemName();

    if (sysName.startsWith("http"))
    {
        // Network stream
        if (!m_nam)
        {
            qCritical() << "network not initialised.";
            return Q_NULLPTR;
        }

        transcodeProcess = new FfmpegTranscoding();

        // request from network url for streaming
        auto movie = new DlnaNetworkVideo(transcodeProcess);
        movie->setDlnaParent(this);
        connect(movie, &DlnaNetworkVideo::streamUrlDefined, transcodeProcess, &FfmpegTranscoding::setUrls);
        connect(movie, SIGNAL(videoUrlErrorSignal(QString)), transcodeProcess, SLOT(urlError(QString)));
        movie->setAnalyzeStream(false);
        movie->setNetworkAccessManager(m_nam);
        movie->setUrl(sysName);
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
        transcodeProcess->setFormat(transcodeFormat);
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
    if (sysName.startsWith("http"))
    {
        TranscodeProcess *process = getTranscodeProcess();
        process->setFormat(COPY);
        return process;
    }

    return new StreamingFile(getSystemName());
}
