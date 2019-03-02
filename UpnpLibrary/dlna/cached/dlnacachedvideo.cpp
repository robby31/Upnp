#include "dlnacachedvideo.h"

DlnaCachedVideo::DlnaCachedVideo(MediaLibrary* library, int idMedia, QObject *parent):
    DlnaVideoItem(parent),
    library(library),
    idMedia(idMedia)
{
    if (library)
    {
        QUrl url = library->get_param_value(idMedia, "thumbnailUrl").toUrl();

        if (url.isValid())
        {
            QNetworkRequest request(url);
            m_replyPicture = MyNetwork::manager().get(request);
            if (m_replyPicture)
            {
                connect(m_replyPicture, &QNetworkReply::finished, this, &DlnaCachedVideo::pictureReceived);
                connect(m_replyPicture, &QNetworkReply::finished, &m_loopPicture, &QEventLoop::quit);
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
    transcodeProcess->setFormat(transcodeFormat);
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
    if (library)
        return library->getmetaData("samplerate", idMedia).toInt();

    return -1;
}

int DlnaCachedVideo::channelCount() const
{
    if (library)
        return library->getmetaData("channelcount", idMedia).toInt();

    return -1;
}

QString DlnaCachedVideo::framerate() const
{
    if (library)
        return library->getmetaData("framerate", idMedia).toString();

    return QString();
}

qint64 DlnaCachedVideo::getResumeTime() const
{
    qint64 res = 0;

    if (library)
        res = library->getmetaData("progress_played", idMedia).toLongLong();

    if (res > 10000)
        return res - 10000;   // returns 10 seconds before resume time

    return 0;
}

QHash<QString, double> DlnaCachedVideo::volumeInfo(const int &timeout)
{
    Q_UNUSED(timeout)

    if (library)
    {
        return library->volumeInfo(idMedia);
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

    if (library)
        duration = library->getmetaData("duration", idMedia).toInt();

    if (duration <= 0)
        duration = 5*3600000;  // default duration when live video are unlimited

    return duration;
}

QUrl DlnaCachedVideo::thumbnailUrl() const
{
    if (library)
        return library->get_param_value(idMedia, "thumbnailUrl").toUrl();

    return QUrl();
}

void DlnaCachedVideo::pictureReceived()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply)
    {
        m_picture = reply->readAll();
        reply->deleteLater();
        m_replyPicture = Q_NULLPTR;
    }
}

QByteArray DlnaCachedVideo::metaDataPicture()
{
    waitPicture(2000);
    return m_picture;
}

bool DlnaCachedVideo::waitPicture(const int &timeout)
{
    if (m_replyPicture)
    {
        QTimer::singleShot(timeout, &m_loopPicture, &QEventLoop::quit);
        m_loopPicture.exec();
    }

    return true;
}
