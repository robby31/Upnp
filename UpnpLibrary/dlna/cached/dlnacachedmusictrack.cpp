#include "dlnacachedmusictrack.h"

DlnaCachedMusicTrack::DlnaCachedMusicTrack(MediaLibrary *library, int idMedia, QObject *parent):
    DlnaMusicTrack(parent),
    library(library),
    idMedia(idMedia)
{
    setTranscodeFormat(MP3);   // default transcode format
}

qint64 DlnaCachedMusicTrack::getResumeTime() const
{
    qint64 res = 0;

    if (library)
        res = library->getmetaData("progress_played", idMedia).toLongLong();

    if (res > 10000)
        return res - 10000;   // returns 10 seconds before resume time

    return 0;
}

QHash<QString, double> DlnaCachedMusicTrack::volumeInfo(const int& timeout)
{
    Q_UNUSED(timeout)

    if (library)
        return library->volumeInfo(idMedia);

    QHash<QString, double> result;
    return result;
}

QString DlnaCachedMusicTrack::sourceContainer() const
{
    return metaDataFormat();
}

QString DlnaCachedMusicTrack::sourceAudioFormat() const
{
    if (library)
        return library->get_param_value(idMedia, "audio_format").toString();

    return QString();
}

QString DlnaCachedMusicTrack::sourceVideoFormat() const
{
    if (library)
        return library->get_param_value(idMedia, "video_format").toString();

    return QString();
}
