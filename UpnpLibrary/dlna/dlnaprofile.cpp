#include "dlnaprofile.h"

DlnaProfile::DlnaProfile(QObject *parent):
    QObject(parent)
{
    DebugInfo::add_object(this);
}

DlnaProfile::DlnaProfile(const QString &pn, QObject *parent):
    QObject(parent),
    m_pn(pn)
{
    DebugInfo::add_object(this);
}

DlnaProfile::~DlnaProfile()
{
    DebugInfo::remove_object(this);
}

bool DlnaProfile::isValid() const
{
    return !m_pn.isEmpty() && !m_mimeType.isEmpty();
}

bool DlnaProfile::addInfo(const QString &param, const QVariant &value)
{
    if (param == "mime_type")
    {
        if (!m_mimeType.isEmpty())
        {
            qCritical() << "mime type alread defined" << m_pn << m_mimeType << value;
        }

        m_mimeType = value.toString();
        return true;
    }

    if (param == "container")
    {
        if (!m_container.isEmpty())
        {
            qCritical() << "container alread defined" << m_pn << m_container << value;
        }

        m_container = value.toString();
        return true;
    }

    if (param == "codec_audio")
    {
        if (!m_codecAudio.contains(value.toString()))
            m_codecAudio << value.toString();
        return true;
    }

    if (param == "codec_video")
    {
        if (!m_codecVideo.contains(value.toString()))
            m_codecVideo << value.toString();
        return true;
    }

    if (param == "channels")
    {
        bool flag_ok = false;
        int data = value.toInt(&flag_ok);

        if (!flag_ok)
        {
            qCritical() << "invalid channel" << value;
            return false;
        }

        if (!m_channels.contains(data))
            m_channels << data;

        return true;
    }

    if (param == "sample_rate")
    {
        bool flag_ok = false;
        int data = value.toInt(&flag_ok);

        if (!flag_ok)
        {
            qCritical() << "invalid sample rate" << value;
            return false;
        }

        if (!m_sampleRate.contains(data))
            m_sampleRate << data;

        return true;
    }

    if (param == "bitrate")
    {
        bool flag_ok = false;
        int data = value.toInt(&flag_ok);

        if (!flag_ok)
        {
            qCritical() << "invalid bitrate" << value;
            return false;
        }

        if (!m_bitrate.contains(data))
            m_bitrate << data;

        return true;
    }

    qCritical() << "unknown attribute in dlna profile" << param;
    return false;
}

QString DlnaProfile::pn() const
{
    return m_pn;
}

QString DlnaProfile::mimeType() const
{
    return m_mimeType;
}

QString DlnaProfile::container() const
{
    return m_container;
}

QStringList DlnaProfile::codecAudio() const
{
    return m_codecAudio;
}

QStringList DlnaProfile::codecVideo() const
{
    return m_codecVideo;
}

QList<int> DlnaProfile::channels() const
{
    return m_channels;
}

QList<int> DlnaProfile::sampleRate() const
{
    return m_sampleRate;
}

QList<int> DlnaProfile::bitrate() const
{
    return m_bitrate;
}
