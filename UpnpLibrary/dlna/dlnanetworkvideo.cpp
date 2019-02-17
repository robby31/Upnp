#include "dlnanetworkvideo.h"

qint64 DlnaNetworkVideo::objectCounter = 0;

DlnaNetworkVideo::DlnaNetworkVideo(QObject *parent) :
    DlnaVideoItem(parent),
    m_analyzeStream(true)
{
    ++objectCounter;

}

DlnaNetworkVideo::~DlnaNetworkVideo()
{
    --objectCounter;
    delete m_media;
    qDeleteAll(ffmpeg);
}

bool DlnaNetworkVideo::isValid() const
{
    if (!m_media)
        return false;

    if (m_media->isLive())
        return m_media->isValid() && !metaDataTitle().isEmpty() && !resolution().isEmpty();

    return m_media->isValid() && !metaDataTitle().isEmpty() && !resolution().isEmpty() && metaDataDuration()>0;
}

QString DlnaNetworkVideo::error() const
{
    if (m_media)
        return m_media->error();

    return m_error;
}

QString DlnaNetworkVideo::getName() const
{
    return metaDataTitle();
}

QString DlnaNetworkVideo::getSystemName() const
{
    return url().url();
}

QString DlnaNetworkVideo::getDisplayName() const
{
    return metaDataTitle();
}

qint64 DlnaNetworkVideo::sourceSize() const
{
    if (m_media && m_media->isLive())
        return 0;

    qint64 size = 0;

    foreach (QFfmpegInputMedia *media, ffmpeg)
        size += media->size();

    return size;
}

qint64 DlnaNetworkVideo::metaDataBitrate() const
{
    qint64 bitrate = 0;

    foreach (QFfmpegInputMedia *media, ffmpeg)
        bitrate += media->getBitrate();

    return bitrate;
}

qint64 DlnaNetworkVideo::metaDataDuration() const
{
    if (m_media && m_media->isLive())
        return 0;

    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->getDurationInMicroSec() > 0)
            return media->getDurationInMicroSec()/1000;

    return 0;
}

QString DlnaNetworkVideo::metaDataTitle() const
{
    if (m_media)
        return m_media->title();

    return QString();
}

QString DlnaNetworkVideo::metaDataFormat() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (!media->getFormat().isEmpty())
            return media->getFormat();

    return QString();
}

int DlnaNetworkVideo::samplerate() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->audioStream())
            return media->getAudioSamplerate();

    return -1;
}

int DlnaNetworkVideo::channelCount() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->audioStream())
            return media->getAudioChannelCount();

    return -1;
}

QHash<QString, double> DlnaNetworkVideo::volumeInfo(const int& timeout)
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->audioStream())
            return media->getVolumeInfo(timeout);

    QHash<QString, double> res;
    return res;
}

QString DlnaNetworkVideo::resolution() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->videoStream())
            return media->getVideoResolution();

    return QString();
}

QString DlnaNetworkVideo::framerate() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->videoStream())
            return  QString().sprintf("%2.3f", media->getVideoFrameRate());

    return QString();
}

QList<QUrl> DlnaNetworkVideo::mediaUrl() const
{
    if (m_media)
        return m_media->mediaUrl();

    QList<QUrl> res;
    return res;
}

void DlnaNetworkVideo::setAnalyzeStream(const bool &flag)
{
    m_analyzeStream = flag;
}

QUrl DlnaNetworkVideo::url() const
{
    return m_url;
}

void DlnaNetworkVideo::setUrl(const QUrl &url)
{
    m_url = url;

    delete m_media;

    MediaStreaming streaming;
    m_media = streaming.get_media(url);

    if (m_media)
    {
        connect(m_media, &AbstractMedia::mediaReady, this, &DlnaNetworkVideo::parse_video);
    }
    else
    {
        qCritical() << "ERROR, unable to set url" << url;
        m_error = QString("unable to set url %1").arg(url.url());
    }
}

bool DlnaNetworkVideo::waitUrl(const unsigned long &timeout)
{
    if (m_media)
        return m_media->waitReady(static_cast<int>(timeout));

    return true;
}

void DlnaNetworkVideo::setMaxVideoHeight(const int &height)
{  
    if (m_media)
        m_media->setMaxHeight(height);
    else
        qCritical() << "Unable to set playback quality because Network Stream is not initialized (call setUrl before).";
}

TranscodeProcess *DlnaNetworkVideo::getTranscodeProcess()
{
    auto transcodeProcess = new FfmpegTranscoding();

    transcodeProcess->setOriginalLengthInMSeconds(metaDataDuration());
    transcodeProcess->setFormat(transcodeFormat);
    transcodeProcess->setBitrate(bitrate());
    transcodeProcess->setAudioLanguages(audioLanguages());
    transcodeProcess->setSubtitleLanguages(subtitleLanguages());
    transcodeProcess->setFrameRate(framerate());
    transcodeProcess->setAudioChannelCount(channelCount());
    transcodeProcess->setAudioSampleRate(samplerate());
    transcodeProcess->setUrls(mediaUrl());
    return transcodeProcess;
}

Device *DlnaNetworkVideo::getOriginalStreaming()
{
    TranscodeProcess *process = getTranscodeProcess();
    process->setFormat(COPY);
    return process;
}

void DlnaNetworkVideo::parse_video()
{
    qDeleteAll(ffmpeg);
    ffmpeg.clear();

    QList<QUrl> urls = mediaUrl();
    if (m_analyzeStream && !urls.isEmpty())
    {
        foreach (const QUrl &url, urls)
        {
            auto tmp = new QFfmpegInputMedia();
            tmp->open(url.toString());
            ffmpeg << tmp;
        }
    }

    if (m_media && m_media->isValid())
    {
        emit streamUrlDefined(mediaUrl());
    }
    else
    {
        QString message = QString("invalid media %1").arg(url().toString());
        if (m_media && !m_media->error().isEmpty())
            message = m_media->error();
        emit videoUrlErrorSignal(message);
    }
}
