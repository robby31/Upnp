#include "dlnanetworkvideo.h"

DlnaNetworkVideo::DlnaNetworkVideo(QObject *parent) :
    DlnaVideoItem(parent)
{

}

DlnaNetworkVideo::~DlnaNetworkVideo()
{
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

    if (size == 0 && m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->size();
    }

    return size;
}

qint64 DlnaNetworkVideo::metaDataBitrate() const
{
    qint64 bitrate = 0;

    foreach (QFfmpegInputMedia *media, ffmpeg)
        bitrate += media->getBitrate();

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->bitrate();
    }

    return bitrate;
}

qint64 DlnaNetworkVideo::metaDataDuration() const
{
    if (m_media && m_media->isLive())
        return 0;

    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->getDurationInMicroSec() > 0)
            return media->getDurationInMicroSec()/1000;

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->duration();
    }

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

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->format();
    }

    return QString();
}
QString DlnaNetworkVideo::sourceContainer() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (!media->getFormat().isEmpty())
            return media->getFormat();

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->format();
    }

    return QString();
}

QString DlnaNetworkVideo::sourceAudioFormat() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (!media->getAudioFormat().isEmpty())
            return media->getAudioFormat();

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->audioCodec();
    }

    return QString();
}

QString DlnaNetworkVideo::sourceVideoFormat() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (!media->getVideoFormat().isEmpty())
            return media->getVideoFormat();

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->videoCodec();
    }

    return QString();
}


int DlnaNetworkVideo::samplerate() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->audioStream())
            return media->getAudioSamplerate();

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->sampleRate();
    }

    return -1;
}

int DlnaNetworkVideo::channelCount() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->audioStream())
            return media->getAudioChannelCount();

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->channels();
    }

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

    if (m_media)
        return m_media->resolution();

    return QString();
}

QString DlnaNetworkVideo::framerate() const
{
    foreach (QFfmpegInputMedia *media, ffmpeg)
        if (media->videoStream())
            return  QString().sprintf("%2.3f", media->getVideoFrameRate());

    if (m_media)
    {
        AbstractStream *stream = m_media->getStream();
        if (stream)
            return stream->frameRate();
    }

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

bool DlnaNetworkVideo::setUrl(const QUrl &url)
{
    m_url = url;

    if (m_media)
        m_media->deleteLater();

    MediaStreaming streaming;
    m_media = streaming.get_media(url);

    if (m_media)
    {
        m_media->setParent(this);
        connect(m_media, &AbstractMedia::mediaReady, this, &DlnaNetworkVideo::parse_video);
    }
    else
    {
        qCritical() << "ERROR, unable to set url" << url;
        m_error = QString("unable to set url %1").arg(url.url());
        return false;
    }

    return true;
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
    transcodeProcess->setFormat(format());
    transcodeProcess->setVariableBitrate(true);
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
            if (!tmp->open(url.toString()))
            {
                qCritical() << "unable to open url" << url << tmp->error();
                delete tmp;
            }
            else
            {
                ffmpeg << tmp;
            }
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

QUrl DlnaNetworkVideo::thumbnailUrl() const
{
    if (m_media)
        return m_media->thumbnail();

    return QUrl();
}

