#include "dlnaitem.h"

DlnaItem::DlnaItem(QObject *parent) :
    DlnaResource(parent),
    transcodeFormat(UNKNOWN),  // default transcode format
    dlnaOrgOpFlags("01"),      // seek by byte (exclusive)
    dlnaOrgPN(),
    m_userAgent(),
    m_stream(Q_NULLPTR)
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
        updateDLNAOrgPn();

        if (toTranscode()) {
            setdlnaOrgOpFlags("10");         // seek by time (exclusive)
        }
    }
}

Device *DlnaItem::getStream()
{
    if (m_stream)
    {
        qDebug() << "stream in cache" << m_stream << m_stream->isOpen();
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
            m_stream->deleteLater();

        connect(stream, SIGNAL(destroyed(QObject*)), this, SLOT(streamDestroyed(QObject*)));
        connect(this, SIGNAL(destroyed(QObject*)), stream, SLOT(deleteLater()));
        m_stream = stream;
    }
    else
    {
        qCritical() << "invalid stream" << stream;
    }
}

void DlnaItem::streamDestroyed(QObject *obj)
{
    qDebug() << "stream destroyed" << obj;
    m_stream = Q_NULLPTR;
}

void DlnaItem::setSinkProtocol(const QStringList &protocol)
{
    m_sinkProtocol = protocol;
    m_compatibleSink.clear();
    m_protocolInfo.clear();

    m_compatibleSink = getSink(sourceMimeType());

    if (toTranscode())
    {
        if (format() == MP3)
        {
            QString sink = getSink("audio/mpeg");
            if (!sink.isNull())
            {
                QStringList l_params = sink.split(";");
                if (!l_params.isEmpty())
                {
                    l_params.insert(1, "DLNA.ORG_OP=10");
                    setdlnaOrgOpFlags("10");
                    l_params.append("DLNA.ORG_CI=1");
                    m_protocolInfo = l_params.join(";");
                }
            }
        }
        else if (format() == AAC || format() == ALAC)
        {
            QString sink = getSink("audio/mp4");
            if (!sink.isNull())
            {
                QStringList l_params = sink.split(";");
                if (!l_params.isEmpty())
                {
                    l_params.insert(1, "DLNA.ORG_OP=10");
                    setdlnaOrgOpFlags("10");
                    l_params.append("DLNA.ORG_CI=1");
                    m_protocolInfo = l_params.join(";");
                }
            }
        }
        else if (format() == LPCM_S16BE)
        {
            QString sink = getSink(QString("audio/L16;rate=%1;channels=%2").arg(samplerate()).arg(channelCount()));
            if (!sink.isNull())
            {
                QStringList l_params = sink.split(";");
                if (!l_params.isEmpty())
                {
                    l_params.insert(3, "DLNA.ORG_OP=10");
                    setdlnaOrgOpFlags("10");
                    l_params.append("DLNA.ORG_CI=1");
                    m_protocolInfo = l_params.join(";");
                }
            }
        }
        else if (format() == WAV)
        {
            QString sink = getSink("audio/wav");
            if (!sink.isNull())
            {
                QStringList l_params = sink.split(";");
                if (!l_params.isEmpty())
                {
                    l_params.insert(1, "DLNA.ORG_OP=10");
                    setdlnaOrgOpFlags("10");
                    l_params.append("DLNA.ORG_CI=1");
                    m_protocolInfo = l_params.join(";");
                }
            }
        }
        else if (format() == H264_AAC || format() == H264_AC3)
        {
            QString sink = getSink("video/mp4");
            if (!sink.isNull())
            {
                QStringList l_params = sink.split(";");
                if (!l_params.isEmpty())
                {
                    l_params.insert(1, "DLNA.ORG_OP=10");
                    setdlnaOrgOpFlags("10");
                    l_params.append("DLNA.ORG_CI=1");
                    m_protocolInfo = l_params.join(";");
                }
            }
        }
        else if (format() == MPEG2_AC3)
        {
            QString sink = getSink("video/mpeg");
            if (!sink.isNull())
            {
                QStringList l_params = sink.split(";");
                if (!l_params.isEmpty())
                {
                    l_params.insert(1, "DLNA.ORG_OP=10");
                    setdlnaOrgOpFlags("10");
                    l_params.append("DLNA.ORG_CI=1");
                    m_protocolInfo = l_params.join(";");
                }
            }
        }
    }
    else
    {
        QStringList l_params = m_compatibleSink.split(";");
        if (!l_params.isEmpty())
        {
            l_params.insert(1, "DLNA.ORG_OP=01");
            setdlnaOrgOpFlags("01");
            l_params.append("DLNA.ORG_CI=0");
            m_protocolInfo = l_params.join(";");
        }
    }

    if (m_protocolInfo.isEmpty())
        qCritical() << "unable to define protocolInfo, protocols available:" << protocol.size();
}

QStringList DlnaItem::sinkProtocol() const
{
    return m_sinkProtocol;
}

QString DlnaItem::getSink(const QString &mime_type)
{
    QRegularExpression pattern("([^:]+):([^:]+):([^:]+):([^:]+)");
    foreach (const QString &sink, sinkProtocol())
    {
        QRegularExpressionMatch match = pattern.match(sink);
        if (match.hasMatch())
        {
            if (mime_type == match.captured(3))
                return sink;
        }
    }

    return QString();
}

bool DlnaItem::isSourceSinkCompatible() const
{
    return !m_compatibleSink.isEmpty();
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
