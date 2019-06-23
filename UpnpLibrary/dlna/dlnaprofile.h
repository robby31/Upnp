#ifndef DLNAPROFILE_H
#define DLNAPROFILE_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QVariant>
#include "debuginfo.h"

class DlnaProfile : public QObject
{
    Q_OBJECT

public:
    explicit DlnaProfile(QObject *parent = Q_NULLPTR);
    explicit DlnaProfile(const QString &pn, QObject *parent = Q_NULLPTR);
    ~DlnaProfile() Q_DECL_OVERRIDE;

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
