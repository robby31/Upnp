#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QStringList>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include "dlnaprofile.h"
#include "protocolinfo.h"

class Protocol
{
public:
    Protocol() = default;
    Protocol(const QString &profile_path);

    QList<ProtocolInfo> protocols() const;
    void setProtocols(const QStringList &protocols);
    void setProtocols(const QList<ProtocolInfo> &protocols);

    void setDlnaProfiles(const QList<DlnaProfile> &profiles);

    QList<ProtocolInfo> compatible();

    void setMimeType(const QString &mimeType);
    void setContainer(const QString &container);
    void setAudioCodec(const QString &codec);
    void setVideoCodec(const QString &codec);
    void setChannels(const int &channels);
    void setSampleRate(const int &sampleRate);
    void setBitrate(const qint64 &bitrate);

    void setDlnaOrgPn(const QString &dlna_org_pn);

private:
    void loadXmlProfiles(const QString &profile_path);
    void addProfile(const QDomNode &profile);
    DlnaProfile getProfile(const QString &pn);

private:
    QList<ProtocolInfo> m_protocols;

    QString m_mimeType;
    QString m_container;
    QString m_audioCodec;
    QString m_videoCodec;
    int m_channels = -1;
    int m_sampleRate = -1;
    qint64 m_bitrate = -1;

    QString m_dlna_org_pn;
    QList<DlnaProfile> m_profiles;
};

#endif // PROTOCOL_H
