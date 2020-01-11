#include "dlnayoutubevideo.h"

DlnaYouTubeVideo::DlnaYouTubeVideo(QObject *parent) :
    DlnaVideoItem(parent),
    m_youtube(this)
{
    connect(&m_youtube, &Youtube::mediaReady, this, &DlnaYouTubeVideo::videoUrl);
}

void DlnaYouTubeVideo::setMaxVideoHeight(const int &height)
{
    m_youtube.setMaxHeight(height);
}

QUrl DlnaYouTubeVideo::url() const
{
    return m_youtube.url();
}

void DlnaYouTubeVideo::setUrl(const QUrl &url)
{
    m_youtube.setUrl(url);
}

void DlnaYouTubeVideo::videoUrl()
{
    if (!m_streamUrl.isEmpty())
    {
        qCritical() << "url already received" << m_streamUrl;
    }
    else
    {
        if (m_youtube.videoUrl().isValid())
        {
            if (m_analyzeStream)
                ffmpeg.open(m_youtube.videoUrl().url());
        }
        else if (m_youtube.audioUrl().isValid())
        {
            if (m_analyzeStream)
                ffmpeg.open(m_youtube.audioUrl().url());
        }

        if (m_youtube.videoUrl().isValid() && m_youtube.audioUrl().isValid() && m_youtube.videoUrl() != m_youtube.audioUrl())
            qCritical() << "only video url is used.";

        emit streamUrlDefined(m_youtube.videoUrl(), m_youtube.audioUrl());
    }
}

bool DlnaYouTubeVideo::waitUrl(const unsigned long &timeout)
{
    return m_youtube.waitReady(static_cast<int>(timeout));
}

TranscodeProcess *DlnaYouTubeVideo::getTranscodeProcess()
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
    return m_youtube.title();
}

QString DlnaYouTubeVideo::error() const
{
    return m_youtube.error();
}
