#include "dlnayoutubevideo.h"

qint64 DlnaYouTubeVideo::objectCounter = 0;

DlnaYouTubeVideo::DlnaYouTubeVideo(QObject *parent) :
    DlnaVideoItem(parent),
    m_analyzeStream(true)
{
    ++objectCounter;
}

DlnaYouTubeVideo::~DlnaYouTubeVideo()
{
    --objectCounter;
}

QString DlnaYouTubeVideo::unavailableMessage()
{
    if (m_youtube)
        return m_youtube->unavailableMessage();

    return QString();
}

void DlnaYouTubeVideo::setNetworkAccessManager(QNetworkAccessManager *manager)
{
    if (m_youtube)
    {
        qCritical() << "youtube already initialised";
    }
    else if (manager)
    {
        m_youtube = new Youtube(manager);
        connect(this, SIGNAL(destroyed()), m_youtube, SLOT(deleteLater()));
        connect(m_youtube, &Youtube::mediaReady, this, &DlnaYouTubeVideo::videoUrl);
    }
}

void DlnaYouTubeVideo::setMaxVideoHeight(const int &height)
{
    if (m_youtube)
        m_youtube->setMaxHeight(height);
    else
        qCritical() << "Unable to set quality because Youtube is not initialized (call setNetworkAccessManager before).";
}

QUrl DlnaYouTubeVideo::url() const
{
    if (m_youtube)
        return m_youtube->url();

    return QUrl();
}

void DlnaYouTubeVideo::setUrl(const QUrl &url)
{
    if (m_youtube)
    {
        m_youtube->setUrl(url);
    }
    else
    {
        qCritical() << "ERROR, unable to set url" << m_youtube;
    }
}

void DlnaYouTubeVideo::videoUrl()
{
    if (!m_streamUrl.isEmpty())
    {
        qCritical() << "url already received" << m_streamUrl;
    }
    else
    {
        QList<QUrl> urls = m_youtube->mediaUrl();
        if (!urls.isEmpty())
            m_streamUrl = urls.at(0);
        else
            m_streamUrl.clear();

        if (m_streamUrl.isValid() && m_analyzeStream)
            ffmpeg.open(m_streamUrl.url());

        emit streamUrlDefined(m_streamUrl.url());
    }
}

bool DlnaYouTubeVideo::waitUrl(const unsigned long &timeout)
{
    if (m_youtube)
        return m_youtube->waitReady(static_cast<int>(timeout));

    return true;
}

TranscodeProcess *DlnaYouTubeVideo::getTranscodeProcess()
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
    transcodeProcess->setUrl(streamUrl().url());
    return transcodeProcess;
}

Device *DlnaYouTubeVideo::getOriginalStreaming()
{
    TranscodeProcess *process = getTranscodeProcess();
    process->setFormat(COPY);
    return process;
}

QHash<QString, double> DlnaYouTubeVideo::volumeInfo(const int& timeout)
{
    return ffmpeg.getVolumeInfo(timeout);
}

QString DlnaYouTubeVideo::metaDataFormat() const
{
    return ffmpeg.getFormat();
}

qint64 DlnaYouTubeVideo::sourceSize() const
{
    return ffmpeg.size();
}

qint64 DlnaYouTubeVideo::metaDataDuration() const
{
    return ffmpeg.getDurationInMicroSec()/1000;
}

int DlnaYouTubeVideo::samplerate() const
{
    return ffmpeg.getAudioSamplerate();
}

int DlnaYouTubeVideo::channelCount() const
{
    return ffmpeg.getAudioChannelCount();
}

QString DlnaYouTubeVideo::resolution() const
{
    return ffmpeg.getVideoResolution();
}

QString DlnaYouTubeVideo::framerate() const
{
    return QString().sprintf("%2.3f", ffmpeg.getVideoFrameRate());
}

QString DlnaYouTubeVideo::metaDataTitle() const
{
    if (m_youtube)
        return m_youtube->title();

    return QString();
}

QString DlnaYouTubeVideo::error() const
{
    if (m_youtube)
        return m_youtube->error();

    return QString();
}
