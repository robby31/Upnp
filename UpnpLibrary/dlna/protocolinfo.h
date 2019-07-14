#ifndef PROTOCOLINFO_H
#define PROTOCOLINFO_H

#include <QDebug>
#include <QStringList>
#include <QRegularExpression>
#include "debuginfo.h"

class ProtocolInfo : public QObject
{
    Q_OBJECT

public:
    typedef enum {
      DLNA_ORG_FLAG_SENDER_PACED               = (1 << 31),
      DLNA_ORG_FLAG_TIME_BASED_SEEK            = (1 << 30),
      DLNA_ORG_FLAG_BYTE_BASED_SEEK            = (1 << 29),
      DLNA_ORG_FLAG_PLAY_CONTAINER             = (1 << 28),
      DLNA_ORG_FLAG_S0_INCREASE                = (1 << 27),
      DLNA_ORG_FLAG_SN_INCREASE                = (1 << 26),
      DLNA_ORG_FLAG_RTSP_PAUSE                 = (1 << 25),
      DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE    = (1 << 24),
      DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE = (1 << 23),
      DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE  = (1 << 22),
      DLNA_ORG_FLAG_CONNECTION_STALL           = (1 << 21),
      DLNA_ORG_FLAG_DLNA_V15                   = (1 << 20),
    } dlna_org_flags_t;

    ProtocolInfo(QObject *parent = Q_NULLPTR);
    ProtocolInfo(const QString &info, QObject *parent = Q_NULLPTR);
    ~ProtocolInfo() Q_DECL_OVERRIDE;

    bool isValid() const;

    QString transport() const;
    void setTransport(const QString &transport);

    QString mimeType() const;
    void setMimeType(const QString &mime_type);

    QString options() const;
    QString option(const QString &name) const;
    void setOption(const QString &param, const QString &value);

    void setFlag(const dlna_org_flags_t &flag);
    void removeFlag(const dlna_org_flags_t &flag);

    QString toString() const;

    QString pn() const;
    void setPN(const QString &pn);

private:
    QString m_transport;
    QString m_mimeType;
    QHash<QString, QString> m_options;
    uint m_flags = 0;
};

#endif // PROTOCOLINFO_H
