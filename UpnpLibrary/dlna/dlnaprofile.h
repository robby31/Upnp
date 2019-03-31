#ifndef DLNAPROFILE_H
#define DLNAPROFILE_H

#include <QDebug>
#include <QString>
#include <QVariant>

class DlnaProfile
{
public:
    DlnaProfile() = default;
    DlnaProfile(const QString &pn);

    bool isValid() const;

    bool addInfo(const QString &param, const QVariant &value);

    QString pn() const;
    QString mimeType() const;
    QString container() const;
    QStringList codecAudio() const;
    QStringList codecVideo() const;
    QList<int> channels() const;
    QList<int> sampleRate() const;
    QList<int> bitrate() const;

private:
    QString m_pn;
    QString m_mimeType;
    QString m_container;
    QStringList m_codecAudio;
    QStringList m_codecVideo;
    QList<int> m_channels;
    QList<int> m_sampleRate;
    QList<int> m_bitrate;
};

#endif // DLNAPROFILE_H
