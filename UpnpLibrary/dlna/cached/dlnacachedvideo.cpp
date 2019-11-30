#include "dlnacachedvideo.h"

DlnaCachedVideo::DlnaCachedVideo(MediaLibrary* library, int idMedia, QObject *parent):
    DlnaVideoItem(parent),
    m_library(library),
    m_idMedia(idMedia)
{
    if (library)
    {
        QUrl url = library->get_param_value(idMedia, "thumbnailUrl").toUrl();

        if (url.isValid())
        {
            QNetworkRequest request(url);
            m_replyPicture = MyNetwork::manager().get(request);
            DebugInfo::add_object(m_replyPicture);
            if (m_replyPicture)
            {
                m_replyPicture->setParent(this);
                setReady(false);
                connect(m_replyPicture, &QNetworkReply::finished, this, &DlnaCachedVideo::pictureReceived);
                connect(m_replyPicture, &QNetworkReply::finished, m_replyPicture, &QNetworkReply::deleteLater);
            }
            else
            {
                qCritical() << "unable to get thumbnail" << url;
            }
        }
    }
}

TranscodeProcess *DlnaCachedVideo::getTranscodeProcess()
{
    auto transcodeProcess = new FfmpegTranscoding();
    transcodeProcess->setUrl(getSystemName());
    transcodeProcess->setOriginalLengthInMSeconds(metaDataDuration());
    transcodeProcess->setFormat(format());
    transcodeProcess->setVariableBitrate(true);
    transcodeProcess->setBitrate(bitrate());
    transcodeProcess->setAudioLanguages(audioLanguages());
    transcodeProcess->setSubtitleLanguages(subtitleLanguages());
    transcodeProcess->setFrameRate(framerate());
    transcodeProcess->setAudioChannelCount(channelCount());
    transcodeProcess->setAudioSampleRate(samplerate());
//    transcodeProcess->setVolumeInfo(volumeInfo());

    return transcodeProcess;
}

int DlnaCachedVideo::samplerate() const
{
    if (m_library)
        return m_library->getmetaData("samplerate", m_idMedia).toInt();

    return -1;
}

int DlnaCachedVideo::channelCount() const
{
    if (m_library)
        return m_library->getmetaData("channelcount", m_idMedia).toInt();

    return -1;
}

QString DlnaCachedVideo::framerate() const
{
    if (m_library)
        return m_library->getmetaData("framerate", m_idMedia).toString();

    return QString();
}

qint64 DlnaCachedVideo::getResumeTime() const
{
    qint64 res = 0;

    if (m_library)
        res = m_library->getmetaData("progress_played", m_idMedia).toLongLong();

    if (res > 10000)
        return res - 10000;   // returns 10 seconds before resume time

    return 0;
}

QHash<QString, double> DlnaCachedVideo::volumeInfo(const int &timeout)
{
    Q_UNUSED(timeout)

    if (m_library)
    {
        return m_library->volumeInfo(m_idMedia);
    }

    QHash<QString, double> result;
    return result;
}

Device *DlnaCachedVideo::getOriginalStreaming()
{
    return new StreamingFile(getSystemName());
}

qint64 DlnaCachedVideo::metaDataDuration() const
{
    qint64 duration = 0;

    if (m_library)
        duration = m_library->getmetaData("duration", m_idMedia).toInt();

    if (duration <= 0)
        duration = 5*3600000;  // default duration when live video are unlimited

    return duration;
}

QUrl DlnaCachedVideo::thumbnailUrl() const
{
    if (m_library)
        return m_library->get_param_value(m_idMedia, "thumbnailUrl").toUrl();

    return QUrl();
}

void DlnaCachedVideo::pictureReceived()
{
    setReady(true);

    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply)
    {
        m_picture = reply->readAll();
        m_replyPicture = Q_NULLPTR;
    }
}

QByteArray DlnaCachedVideo::metaDataPicture()
{
    return m_picture;
}

QString DlnaCachedVideo::sourceContainer() const
{
    return metaDataFormat();
}

QString DlnaCachedVideo::sourceAudioFormat() const
{
    if (m_library)
        return m_library->get_param_value(m_idMedia, "audio_format").toString();

    return QString();
}

QString DlnaCachedVideo::sourceVideoFormat() const
{
    if (m_library)
        return m_library->get_param_value(m_idMedia, "video_format").toString();

    return QString();
}
