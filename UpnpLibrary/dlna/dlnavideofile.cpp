#include "dlnavideofile.h"

DlnaVideoFile::DlnaVideoFile(const QString &filename, QObject *parent):
    DlnaVideoItem(parent),
    fileinfo(filename)
{
    ffmpeg.open(filename, false);

    QMimeDatabase db;
    mime_type = db.mimeTypeForFile(fileinfo);
}

TranscodeProcess *DlnaVideoFile::getTranscodeProcess()
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

QString DlnaVideoFile::metaDataTitle() const {
    return ffmpeg.metaData("title");
}

QString DlnaVideoFile::metaDataGenre() const {
    return ffmpeg.metaData("genre");
}

QString DlnaVideoFile::metaDataPerformer() const{
    return ffmpeg.metaData("artist");
}

QString DlnaVideoFile::metaDataPerformerSort() const{
    return ffmpeg.metaData("artist-sort");
}

QString DlnaVideoFile::metaDataAlbum() const {
    return ffmpeg.metaData("album");
}

QString DlnaVideoFile::metaDataAlbumArtist() const {
    return ffmpeg.metaData("album_artist");
}

int DlnaVideoFile::metaDataYear() const
{
    QDateTime date = QDateTime::fromString(ffmpeg.metaData("date"), "yyyy-MM-dd");
    if (date.isValid())
        return date.toString("yyyy").toInt();
    return -1;
}

int DlnaVideoFile::metaDataTrackPosition() const {
    return ffmpeg.metaData("track").split('/').at(0).toInt();
}

int DlnaVideoFile::metaDataDisc() const
{
    return ffmpeg.metaData("disc").split('/').at(0).toInt();
}

QString DlnaVideoFile::metaDataFormat() const {
    return ffmpeg.getFormat();
}

QByteArray DlnaVideoFile::metaDataPicture() {
    return ffmpeg.getPicture();
}

qint64 DlnaVideoFile::metaDataDuration() const {
    return ffmpeg.getDurationInMicroSec()/1000;
}

qint64 DlnaVideoFile::metaDataBitrate() const {
    return ffmpeg.getBitrate();
}

int DlnaVideoFile::channelCount() const
{
    return ffmpeg.getAudioChannelCount();
}

int DlnaVideoFile::samplerate() const
{
    return ffmpeg.getAudioSamplerate();
}

QHash<QString, double> DlnaVideoFile::volumeInfo(const int& timeout)
{
    return ffmpeg.getVolumeInfo(timeout);
}

QString DlnaVideoFile::resolution() const {
    return ffmpeg.getVideoResolution();
}

QStringList DlnaVideoFile::audioLanguages() const {
    return ffmpeg.getAudioLanguages();
}

QStringList DlnaVideoFile::subtitleLanguages() const {
    return ffmpeg.getSubtitleLanguages();
}

QString DlnaVideoFile::framerate() const {
    return QString().sprintf("%2.3f", ffmpeg.getVideoFrameRate());
}

Device *DlnaVideoFile::getOriginalStreaming()
{
    return new StreamingFile(getSystemName());
}

QString DlnaVideoFile::sourceContainer() const
{
    return ffmpeg.getFormat();
}

QString DlnaVideoFile::sourceAudioFormat() const
{
    return ffmpeg.getAudioFormat();
}

QString DlnaVideoFile::sourceVideoFormat() const
{
    return ffmpeg.getVideoFormat();
}
