#include "protocol.h"

Protocol::Protocol(const QString &profile_path)
{
    loadXmlProfiles(profile_path);
}

void Protocol::loadXmlProfiles(const QString &profile_path)
{
    QFile fd(profile_path);
    if (!fd.open(QFile::ReadOnly))
    {
        qCritical() << "unable to open profiles" <<profile_path;
    }
    else
    {
        QDomDocument xml;
        xml.setContent(&fd);

        QDomNodeList l_dlna = xml.elementsByTagName("profile");
        for (int i=0;i<l_dlna.size();++i)
        {
            QDomNode dlna = l_dlna.at(i);
            addProfile(dlna);
        }
    }

    qWarning() << m_profiles.size() << "dlna profiles loaded.";
}

void Protocol::addProfile(const QDomNode &profile)
{
    QString pn = profile.toElement().attributeNode("pn").value();

    if (!pn.isEmpty())
    {
        DlnaProfile dlna_profile(pn);

        QDomNodeList children = profile.childNodes();
        for (int i=0;i<children.size();++i)
        {
            QDomNode child = children.at(i);

            QString info = child.nodeName();
            QVariant value;
            if (child.childNodes().size()==1)
                value = QVariant::fromValue(child.childNodes().at(0).toText().nodeValue());

            dlna_profile.addInfo(info, value);
        }


        m_profiles << dlna_profile;
    }
}

QList<ProtocolInfo> Protocol::protocols() const
{
    return m_protocols;
}

void Protocol::setProtocols(const QStringList &protocols)
{
    m_protocols.clear();

    for (const QString &protocol : protocols)
    {
        ProtocolInfo protocolInfo(protocol);

        if (!getProfile(protocolInfo.pn()).isValid())
            qDebug() << "undefined profile for" << protocolInfo.pn() << protocolInfo.toString();

        m_protocols << protocolInfo;
    }
}

void Protocol::setProtocols(const QList<ProtocolInfo> &protocols)
{
    m_protocols = protocols;
}

QList<ProtocolInfo> Protocol::compatible()
{
    qDebug() << "GET COMPATIBLE" << m_mimeType << m_container << m_videoCodec << m_audioCodec << m_channels << m_sampleRate << m_bitrate << m_dlna_org_pn;
    QList<ProtocolInfo> res;

    QStringList protocolChecked;

    for (const ProtocolInfo &protocol : m_protocols)
    {
        protocolChecked << protocol.pn();
        qDebug() << "check" << protocol.toString();

        if (m_mimeType.startsWith("audio/L16"))
        {
            if (!protocol.mimeType().startsWith("audio/L16"))
            {
                qDebug() << "invalid mime type" << protocol.mimeType() << m_mimeType;
                continue;
            }
        }
        else if (!m_mimeType.isEmpty() && protocol.mimeType() != m_mimeType)
        {
            qDebug() << "invalid mime type" << protocol.mimeType() << m_mimeType;
            continue;
        }

        DlnaProfile profile = getProfile(protocol.pn());
        if (!profile.isValid())
        {
            qDebug() << "unable to find profile for" << protocol.pn();
            continue;
        }

        if (!m_container.isEmpty() && profile.container() != m_container)
        {
            qDebug() << "invalid container" << profile.container() << m_container;
            continue;
        }

        if (!m_audioCodec.isEmpty() && !profile.codecAudio().contains(m_audioCodec))
        {
            qDebug() << "invalid audio codec" << profile.codecAudio() << m_audioCodec;
            continue;
        }

        if (m_channels != -1 && !profile.channels().contains(m_channels))
        {
            qDebug() << "invalid channels" << profile.channels() << m_channels;
            continue;
        }

        if (m_sampleRate != -1 && !profile.sampleRate().contains(m_sampleRate))
        {
            qDebug() << "invalid sample rate" << profile.sampleRate() << m_sampleRate;
            continue;
        }

        if (m_mimeType.startsWith("video"))
        {
            if (!m_videoCodec.isEmpty() && !profile.codecVideo().contains(m_videoCodec))
            {
                qDebug() << "invalid video codec" << profile.codecVideo() << m_videoCodec;
                continue;
            }
        }

        if (!m_dlna_org_pn.isEmpty())
        {
            if (!protocol.pn().isEmpty() && protocol.pn() != m_dlna_org_pn)
            {
                qDebug() << "invalid PN" << protocol.pn() << m_dlna_org_pn;
                continue;
            }
        }

        ProtocolInfo foundProtocol;
        foundProtocol.setTransport(protocol.transport());
        foundProtocol.setMimeType(protocol.mimeType());
        foundProtocol.setPN(protocol.pn());
        res << foundProtocol;
        qDebug() << "found COMPATIBLE" << protocol.pn() << protocol.toString() << foundProtocol.toString();
    }

    if (res.isEmpty())
        qDebug() << "NOT FOUND protocol compatible" << m_mimeType << m_container << m_videoCodec << m_audioCodec << m_channels << m_sampleRate << m_bitrate << m_dlna_org_pn << "in following PN" << protocolChecked;

    return res;
}

void Protocol::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

void Protocol::setContainer(const QString &container)
{
    m_container = container;
}

void Protocol::setAudioCodec(const QString &codec)
{
    m_audioCodec = codec;
}

void Protocol::setVideoCodec(const QString &codec)
{
    m_videoCodec = codec;
}

void Protocol::setChannels(const int &channels)
{
    m_channels = channels;
}

void Protocol::setSampleRate(const int &sampleRate)
{
    m_sampleRate = sampleRate;
}

void Protocol::setBitrate(const qint64 &bitrate)
{
    m_bitrate = bitrate;
}

void Protocol::setDlnaOrgPn(const QString &dlna_org_pn)
{
    m_dlna_org_pn = dlna_org_pn;
}

DlnaProfile Protocol::getProfile(const QString &pn)
{
    for (const DlnaProfile &profile : m_profiles)
    {
        if (profile.pn() == pn)
            return profile;
    }

    return DlnaProfile();
}

void Protocol::setDlnaProfiles(const QList<DlnaProfile> &profiles)
{
    m_profiles = profiles;
}
