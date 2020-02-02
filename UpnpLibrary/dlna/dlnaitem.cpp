#include "dlnaitem.h"

const QString DlnaItem::UNKNOWN_AUDIO_TYPEMIME = "audio/unknown";
const QString DlnaItem::AUDIO_MP3_TYPEMIME = "audio/mpeg";
const QString DlnaItem::AUDIO_MP4_TYPEMIME = "audio/mp4";
const QString DlnaItem::AUDIO_WMA_TYPEMIME = "audio/x-ms-wma";
const QString DlnaItem::AUDIO_FLAC_TYPEMIME = "audio/x-flac";
const QString DlnaItem::AUDIO_OGG_TYPEMIME = "audio/x-ogg";
const QString DlnaItem::AUDIO_LPCM_TYPEMIME = "audio/L16";
const QString DlnaItem::AUDIO_TRANSCODE = "audio/transcode";

const QString DlnaItem::UNKNOWN_VIDEO_TYPEMIME = "video/unknown";
const QString DlnaItem::MPEG_TYPEMIME = "video/mpeg";
const QString DlnaItem::MP4_TYPEMIME = "video/mp4";
const QString DlnaItem::AVI_TYPEMIME = "video/avi";
const QString DlnaItem::WMV_TYPEMIME = "video/x-ms-wmv";
const QString DlnaItem::ASF_TYPEMIME = "video/x-ms-asf";
const QString DlnaItem::MATROSKA_TYPEMIME = "video/x-matroska";
const QString DlnaItem::VIDEO_TRANSCODE = "video/transcode";
const QString DlnaItem::M3U8_TYPEMIME = "application/x-mpegURL";
const QString DlnaItem::MPEGTS_TYPEMIME = "video/vnd.dlna.mpeg-tts";

DlnaItem::DlnaItem(QObject *parent) :
    DlnaResource(parent),
    dlnaOrgOpFlags("01")       // seek by byte (exclusive)
{
}

QString DlnaItem::getDisplayName() const {
    QString title = metaDataTitle();
    if (title.isEmpty())
        title = getName();
    if (title.isEmpty())
        title = getSystemName();
    return title;
}

int DlnaItem::getLengthInSeconds() const {
    return qRound(double(getLengthInMilliSeconds())/1000.0);
}

qint64 DlnaItem::getLengthInMilliSeconds() const {
    if (metaDataDuration() > getResumeTime())
        return metaDataDuration() - getResumeTime();

    return 0;
}

void DlnaItem::setTranscodeFormat(TranscodeFormatAvailable format) {
    if (transcodeFormat != format) {
        transcodeFormat = format;

        if (toTranscode()) {
            setdlnaOrgOpFlags("10");         // seek by time (exclusive)
        }
    }
}

Device *DlnaItem::getStream()
{
    if (m_stream)
    {
#if !defined(QT_NO_DEBUG_OUTPUT)
        qDebug() << "stream in cache" << m_stream << m_stream->isOpen();
#endif

        if (!m_stream->isOpen())
        {
            return m_stream;
        }
    }

    if (toTranscode())
    {
        // DLNA node shall be transcoded
        TranscodeDevice* process = getTranscodeProcess();

        if (process)
        {
            process->setBitrate(bitrate());
        }

        setStream(process);

        return process;
    }

    Device* tmp = getOriginalStreaming();
    setStream(tmp);
    return tmp;
}

QString DlnaItem::getProtocolInfo() const
{
    if (m_protocolInfo.isEmpty())
    {
        qWarning() << getSystemName() << "protocol info is empty";
        return QString("http-get:*:%1:%2").arg(mimeType(), getDlnaContentFeatures());
    }

    return m_protocolInfo;
}

QString DlnaItem::getDlnaContentFeatures() const
{
    if (!m_protocolInfo.isEmpty())
    {
        QStringList l_params = m_protocolInfo.split(":");
        if (!l_params.isEmpty())
            return l_params.at(l_params.size()-1);
    }

    // default value
    QStringList result;

    if (!getdlnaOrgPN().isNull()) {
        result << QString("DLNA.ORG_PN=%1").arg(getdlnaOrgPN());
    }

    result << QString("DLNA.ORG_OP=%1").arg(getdlnaOrgOpFlags());
    if (toTranscode())
        result << "DLNA.ORG_CI=1";
    else
        result << "DLNA.ORG_CI=0";

    return result.join(";");
}

qint64 DlnaItem::size()
{
    if (toTranscode())
    {
        if (m_stream)
            return m_stream->size();

        Device *stream = getStream();
        if (stream)
            return stream->size();

        return -1;
    }

    return sourceSize();
}

void DlnaItem::setStream(Device *stream)
{
    if (stream)
    {
        if (m_stream)
        {
            qWarning() << this << "stream already defined" << m_stream << "replaced by" << stream;
            m_stream->disconnect(this);
            disconnect(m_stream);
            if (!m_stream->isOpen())
                m_stream->deleteLater();
        }

        connect(stream, &Device::destroyed, this, &DlnaItem::streamDestroyed);
        connect(this, &DlnaItem::destroyed, stream, &Device::requestDestroy);
        m_stream = stream;
    }
    else
    {
        qCritical() << "invalid stream" << stream;
    }
}

void DlnaItem::streamDestroyed(QObject *obj)
{
    Q_UNUSED(obj)
    m_stream = Q_NULLPTR;
}

void DlnaItem::setDlnaProfiles(Protocol *profiles)
{
    m_sinkProtocol = profiles;
    if (!m_sinkProtocol)
    {
        qCritical() << "invalid profiles" << profiles;
        return;
    }

    m_sinkProtocol->setMimeType(sourceMimeType());
    m_sinkProtocol->setContainer(sourceContainer());
    m_sinkProtocol->setVideoCodec(sourceVideoFormat());
    m_sinkProtocol->setAudioCodec(sourceAudioFormat());
    m_sinkProtocol->setChannels(channelCount());
    m_sinkProtocol->setSampleRate(samplerate());
    m_sinkProtocol->setBitrate(metaDataBitrate());

    m_compatibleSink = getSink();

    if (toTranscode())
    {
        setdlnaOrgOpFlags("10");

        m_sinkProtocol->setMimeType(mimeType());
        m_sinkProtocol->setContainer(container());
        m_sinkProtocol->setVideoCodec(videoFormat());
        m_sinkProtocol->setAudioCodec(audioFormat());
        m_sinkProtocol->setChannels(channelCount());
        m_sinkProtocol->setSampleRate(samplerate());
        m_sinkProtocol->setBitrate(bitrate());

        ProtocolInfo *sink = getSink();
        if (sink && sink->isValid())
        {
            setdlnaOrgPN(sink->pn());
            sink->setOption("DLNA.ORG_OP", "10");
            sink->setOption("DLNA.ORG_CI", "1");
            m_protocolInfo = sink->toString();
        }
        else
        {
            if (sink)
                qCritical() << "invalid sink found" << sink->toString() << "for" << mimeType() << container() << videoFormat() << audioFormat() << channelCount() << samplerate() << bitrate();
            else
                qCritical() << "invalid sink found" << mimeType() << container() << videoFormat() << audioFormat() << channelCount() << samplerate() << bitrate();
        }
    }
    else
    {
        setdlnaOrgOpFlags("01");
        setdlnaOrgPN(m_compatibleSink->pn());
        m_compatibleSink->setOption("DLNA.ORG_OP", "01");
        m_compatibleSink->setOption("DLNA.ORG_CI", "0");
        m_protocolInfo = m_compatibleSink->toString();
    }

    if (m_protocolInfo.isEmpty())
    {
        qCritical() << "unable to define protocolInfo";
    }
    else if (getdlnaOrgPN() == "LPCM")
    {
        if (m_protocolInfo.contains(":audio/L16:"))
            m_protocolInfo = m_protocolInfo.replace("audio/L16", mimeType());
    }
}

Protocol *DlnaItem::sinkProtocol() const
{
    return m_sinkProtocol;
}

ProtocolInfo *DlnaItem::getSink(const QString &dlna_org_pn)
{
    if (m_sinkProtocol)
    {
        if (!dlna_org_pn.isEmpty())
            m_sinkProtocol->setDlnaOrgPn(dlna_org_pn);

        QList<ProtocolInfo*> protocols = m_sinkProtocol->compatible();
        //    if (protocols.size() > 1)
        //    {
        //        foreach (ProtocolInfo *elt, protocols)
        //            qWarning() << elt->toString();
        //        qWarning() << protocols.size() << "protocols found but first used";
        //    }

        if (!protocols.isEmpty())
        {
            ProtocolInfo *sink = protocols.at(0);
            sink->setFlag(ProtocolInfo::DLNA_ORG_FLAG_DLNA_V15);
            sink->setFlag(ProtocolInfo::DLNA_ORG_FLAG_SENDER_PACED);
            sink->setFlag(ProtocolInfo::DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE);

            // delete all protocols except which returned
            for (int index=1; index<protocols.size(); ++index)
                delete protocols.at(index);

            sink->setParent(this);
            return sink;
        }

        // delete all protocols
        qDeleteAll(protocols);
    }

    return new ProtocolInfo(this);
}

bool DlnaItem::isSourceSinkCompatible() const
{
    if (m_compatibleSink)
        return m_compatibleSink->isValid();

    return false;
}

bool DlnaItem::toTranscode() const
{
    return !isSourceSinkCompatible();
}

QUrl DlnaItem::thumbnailUrl() const
{
    return QUrl();
}

QImage DlnaItem::getAlbumArt()
{
    QImage picture;

    QByteArray bytesPicture = metaDataPicture();
    if (picture.loadFromData(bytesPicture))
        return picture;

    return picture;
}

QString DlnaItem::mimeType() const
{
    if (toTranscode())
    {
        if (transcodeFormat == MP3)
            return AUDIO_MP3_TYPEMIME;

        if (transcodeFormat == AAC || transcodeFormat == ALAC)
            return AUDIO_MP4_TYPEMIME;

        if (transcodeFormat == LPCM_S16BE)
            return QString("%1;rate=%2;channels=%3").arg(AUDIO_LPCM_TYPEMIME).arg(samplerate()).arg(channelCount());

        if (transcodeFormat == MPEG2_AC3)
            return MPEGTS_TYPEMIME;

        if (transcodeFormat == H264_AAC || transcodeFormat == H264_AC3)
            return MPEGTS_TYPEMIME;

        qCritical() << "Unable to define mimeType for transcoding" << getSystemName();
    }

    return sourceMimeType();
}

QString DlnaItem::container() const
{
    if (toTranscode())
    {
        if (transcodeFormat == MP3)
            return "mp3";

        if (transcodeFormat == AAC || transcodeFormat == ALAC)
            return "mov,mp4,m4a,3gp,3g2,mj2";

        if (transcodeFormat == LPCM_S16BE)
            return "lpcm";

        if (transcodeFormat == MPEG2_AC3)
            return "mpegts";

        if (transcodeFormat == H264_AAC || transcodeFormat == H264_AC3)
            return "mpegts";

        qCritical() << "Unable to define container for transcoding" << getSystemName();
    }

    return sourceContainer();
}

QString DlnaItem::audioFormat() const
{
    if (toTranscode())
    {
        if (transcodeFormat == MP3)
            return "mp3";

        if (transcodeFormat == AAC)
            return "aac";

        if (transcodeFormat == ALAC)
            return "alac";

        if (transcodeFormat == LPCM_S16BE)
            return "lpcm";

        if (transcodeFormat == MPEG2_AC3)
            return "ac3";

        if (transcodeFormat == H264_AC3)
            return "ac3";

        if (transcodeFormat == H264_AAC)
            return "aac";

        qCritical() << "Unable to define audio format for transcoding" << getSystemName();
    }

    return sourceAudioFormat();
}

QString DlnaItem::videoFormat() const
{
    if (toTranscode())
    {
        if (transcodeFormat == MP3)
            return QString();

        if (transcodeFormat == AAC || transcodeFormat == ALAC)
            return QString();

        if (transcodeFormat == LPCM_S16BE)
            return QString();

        if (transcodeFormat == MPEG2_AC3)
            return "mpeg";

        if (transcodeFormat == H264_AAC || transcodeFormat == H264_AC3)
            return "h264";

        qCritical() << "Unable to define video format for transcoding" << getSystemName();
    }

    return sourceVideoFormat();
}
