#include "protocolinfo.h"

ProtocolInfo::ProtocolInfo(QObject *parent):
    QObject(parent)
{
    DebugInfo::add_object(this);
}

ProtocolInfo::ProtocolInfo(const QString &info, QObject *parent):
    QObject(parent)
{
    DebugInfo::add_object(this);

    QStringList args = info.split(":");
    if (args.size() != 4)
    {
        qCritical() << "invalid protocol format" << info;
    }
    else
    {
        m_transport = args[0];
        m_mimeType = args[2];

        QStringList options = args[3].split(";");
        for (const QString &option : options)
        {
            if (option == "*")
                continue;

            QRegularExpression pattern(R"(^([^=]+)=(.+)$)");
            QRegularExpressionMatch match = pattern.match(option);
            if (match.hasMatch())
            {
                if (m_options.contains(match.captured(1)))
                {
                    qCritical() << "option already defined" << match.captured(1);
                    continue;
                }

                setOption(match.captured(1), match.captured(2));
            }
            else
            {
                qCritical() << "invalid option" << option;
            }
        }
    }
}

ProtocolInfo::~ProtocolInfo()
{
    DebugInfo::remove_object(this);
}

QString ProtocolInfo::transport() const
{
    return m_transport;
}

void ProtocolInfo::setTransport(const QString &transport)
{
    m_transport = transport;
}

QString ProtocolInfo::mimeType() const
{
    return m_mimeType;
}

void ProtocolInfo::setMimeType(const QString &mime_type)
{
    m_mimeType = mime_type;
}

QString ProtocolInfo::option(const QString &name) const
{
    if (m_options.contains(name))
        return m_options[name];

    return QString();
}

void ProtocolInfo::setOption(const QString &param, const QString &value)
{
    if (param == "DLNA.ORG_FLAGS")
    {
        bool ok = false;
        m_flags = value.leftRef(8).toUInt(&ok, 16);
        if (!ok)
        {
            qCritical() << "invalid option" << param << value;
            m_flags = 0;
        }

        return;
    }

    m_options[param] = value;

    if (param == "DLNA.ORG_OP" && value.size() == 2)
    {
        if (value.at(0) == "1")
            setFlag(DLNA_ORG_FLAG_TIME_BASED_SEEK);
        if (value.at(1) == "1")
            setFlag(DLNA_ORG_FLAG_BYTE_BASED_SEEK);
    }
}

QString ProtocolInfo::options() const
{
    QStringList params = m_options.keys();
    QStringList res;

    if (params.contains("DLNA.ORG_PN"))
    {
        res << QString("DLNA.ORG_PN=%1").arg(m_options["DLNA.ORG_PN"]);
        params.removeAll("DLNA.ORG_PN");
    }

    if (params.contains("DLNA.ORG_OP"))
    {
        res << QString("DLNA.ORG_OP=%1").arg(m_options["DLNA.ORG_OP"]);
        params.removeAll("DLNA.ORG_OP");
    }

    if (params.contains("DLNA.ORG_CI"))
    {
        res << QString("DLNA.ORG_CI=%1").arg(m_options["DLNA.ORG_CI"]);
        params.removeAll("DLNA.ORG_CI");
    }

    if (m_flags != 0)
    {
        char dlna_info[448];
        sprintf(dlna_info, "%s=%.8x%.24x", "DLNA.ORG_FLAGS", m_flags, 0);
        res << QString(dlna_info).toUpper();
    }

    if (!params.isEmpty())
        qCritical() << "invalid parameters for protocolInfo" << params;

    if (res.isEmpty())
        return "*";

    return res.join(";");
}

QString ProtocolInfo::toString() const
{
    return QString("%1:*:%2:%3").arg(m_transport, m_mimeType, options());
}

QString ProtocolInfo::pn() const
{
    return option("DLNA.ORG_PN");
}

void ProtocolInfo::setPN(const QString &pn)
{
    setOption("DLNA.ORG_PN", pn);
}

bool ProtocolInfo::isValid() const
{
    return !toString().isEmpty() && !m_transport.isEmpty() && !m_mimeType.isEmpty() && !pn().isEmpty();
}

void ProtocolInfo::setFlag(const dlna_org_flags_t &flag)
{
    m_flags |= flag;
}

void ProtocolInfo::removeFlag(const dlna_org_flags_t &flag)
{
    int tmp = ~flag;
    m_flags &= tmp;
}
