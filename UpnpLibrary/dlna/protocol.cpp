#include "protocol.h"

Protocol::Protocol(QObject *parent):
    QObject(parent)
{
    DebugInfo::add_object(this);
}

Protocol::Protocol(const QString &profile_path, QObject *parent):
    QObject(parent)
{
    DebugInfo::add_object(this);

    loadXmlProfiles(profile_path);
}

Protocol::~Protocol()
{
    qDeleteAll(m_profiles);
    m_profiles.clear();

    qDeleteAll(m_protocols);
    m_protocols.clear();
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

    qInfo() << m_profiles.size() << "dlna profiles loaded.";
}

void Protocol::addProfile(const QDomNode &profile)
{
    QString pn = profile.toElement().attributeNode("pn").value();

    if (!pn.isEmpty())
    {
        auto dlna_profile = new DlnaProfile(pn, this);

        QDomNodeList children = profile.childNodes();
        for (int i=0;i<children.size();++i)
        {
            QDomNode child = children.at(i);

            QString info = child.nodeName();
            QVariant value;
            if (child.childNodes().size()==1)
                value = QVariant::fromValue(child.childNodes().at(0).toText().nodeValue());

            dlna_profile->addInfo(info, value);
        }


        m_profiles << dlna_profile;
    }
}

void Protocol::setProtocols(const QStringList &protocols)
{
    qDeleteAll(m_protocols);
    m_protocols.clear();

    for (const QString &protocol : protocols)
    {
        auto protocolInfo = new ProtocolInfo(protocol, this);

#if !defined(QT_NO_DEBUG_OUTPUT)
        DlnaProfile *profile = getProfile(protocolInfo->pn());
        if (!profile or !profile->isValid())
            qDebug() << "undefined profile for" << protocolInfo->pn() << protocolInfo->toString();
#endif

        m_protocols << protocolInfo;
    }
}

QList<ProtocolInfo *> Protocol::compatible()
{
#if !defined(QT_NO_DEBUG_OUTPUT)
    qDebug() << "GET COMPATIBLE" << m_mimeType << m_container << m_videoCodec << m_audioCodec << m_channels << m_sampleRate << m_bitrate << m_dlna_org_pn;
#endif

    QList<ProtocolInfo*> res;

    QStringList protocolChecked;

    for (ProtocolInfo *protocol : m_protocols)
    {
        protocolChecked << protocol->pn();

#if !defined(QT_NO_DEBUG_OUTPUT)
        qDebug() << "check" << protocol->toString();
#endif

        if (m_mimeType.startsWith("audio/L16"))
        {
            if (!protocol->mimeType().startsWith("audio/L16"))
            {
#if !defined(QT_NO_DEBUG_OUTPUT)
                qDebug() << "invalid mime type" << protocol->mimeType() << m_mimeType;
#endif
                continue;
            }
        }
        else if (!m_mimeType.isEmpty() && protocol->mimeType() != m_mimeType)
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "invalid mime type" << protocol->mimeType() << m_mimeType;
#endif
            continue;
        }

        DlnaProfile *profile = getProfile(protocol->pn());
        if (!profile or !profile->isValid())
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "unable to find profile for" << protocol->pn();
#endif
            continue;
        }

        if (!m_container.isEmpty() && profile->container() != m_container)
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "invalid container" << profile->container() << m_container;
#endif
            continue;
        }

        if (!m_audioCodec.isEmpty() && !profile->codecAudio().contains(m_audioCodec))
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "invalid audio codec" << profile->codecAudio() << m_audioCodec;
#endif
            continue;
        }

        if (m_channels != -1 && !profile->channels().contains(m_channels))
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "invalid channels" << profile->channels() << m_channels;
#endif
            continue;
        }

        if (m_sampleRate != -1 && !profile->sampleRate().contains(m_sampleRate))
        {
#if !defined(QT_NO_DEBUG_OUTPUT)
            qDebug() << "invalid sample rate" << profile->sampleRate() << m_sampleRate;
#endif
            continue;
        }

        if (m_mimeType.startsWith("video"))
        {
            if (!m_videoCodec.isEmpty() && !profile->codecVideo().contains(m_videoCodec))
            {
#if !defined(QT_NO_DEBUG_OUTPUT)
                qDebug() << "invalid video codec" << profile->codecVideo() << m_videoCodec;
#endif
                continue;
            }
        }

        if (!m_dlna_org_pn.isEmpty())
        {
            if (!protocol->pn().isEmpty() && protocol->pn() != m_dlna_org_pn)
            {
#if !defined(QT_NO_DEBUG_OUTPUT)
                qDebug() << "invalid PN" << protocol->pn() << m_dlna_org_pn;
#endif
                continue;
            }
        }

        auto foundProtocol = new ProtocolInfo(this);
        foundProtocol->setTransport(protocol->transport());
        foundProtocol->setMimeType(protocol->mimeType());
        foundProtocol->setPN(protocol->pn());
        res << foundProtocol;

#if !defined(QT_NO_DEBUG_OUTPUT)
        qDebug() << "found COMPATIBLE" << protocol->pn() << protocol->toString() << foundProtocol->toString();
#endif
    }

#if !defined(QT_NO_DEBUG_OUTPUT)
    if (res.isEmpty())
        qDebug() << "NOT FOUND protocol compatible" << m_mimeType << m_container << m_videoCodec << m_audioCodec << m_channels << m_sampleRate << m_bitrate << m_dlna_org_pn << "in following PN" << protocolChecked;
#endif

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

DlnaProfile* Protocol::getProfile(const QString &pn)
{
    for (DlnaProfile *profile : m_profiles)
    {
        if (profile->pn() == pn)
            return profile;
    }

    return Q_NULLPTR;
}

