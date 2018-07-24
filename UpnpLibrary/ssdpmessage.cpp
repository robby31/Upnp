#include "ssdpmessage.h"

const QString SsdpMessage::CRLF = "\r\n";

const QString SsdpMessage::ALIVE = "ssdp:alive";

const QString SsdpMessage::BYEBYE = "ssdp:byebye";

const QString SsdpMessage::DISCOVER = "\"ssdp:discover\"";

SsdpMessage::SsdpMessage()
{

}

SsdpMessage::SsdpMessage(SsdpFormat format)
{
    // initialize start line
    if (format == SEARCH)
        m_header << "M-SEARCH * HTTP/1.1";
    else if (format == NOTIFY)
        m_header << "NOTIFY * HTTP/1.1";
    else if (format == HTTP)
        m_header << "HTTP/1.1 200 OK";
}

bool SsdpMessage::addHeader(const QString &param, const QString &value)
{
    m_header << QString("%1: %2").arg(param).arg(value);
    return true;
}

QString SsdpMessage::getHeader(const QString &param) const
{
    QRegularExpression stValue("^"+param+":\\s*(.+)\\s*$", QRegularExpression::CaseInsensitiveOption);

    for (int i=1;i<m_header.size();++i)
    {
        QRegularExpressionMatch match = stValue.match(m_header.at(i));
        if (match.hasMatch())
            return match.captured(1).trimmed();
    }

    return QString();
}

bool SsdpMessage::addRawHeader(const QString &data)
{
    m_header << data;
    return true;
}

QByteArray SsdpMessage::toUtf8() const
{
    QStringList res(toStringList());
    return res.join("\r\n").toUtf8();
}

QStringList SsdpMessage::toStringList() const
{
    QStringList data(m_header);
    while (data.at(data.size()-2) != "")
        data << "";
    return data;
}

SsdpMessage SsdpMessage::fromByteArray(const QByteArray &input)
{
    QStringList l_message = QVariant::fromValue(input).toString().split(CRLF);

    SsdpFormat format(INVALID);

    if (l_message.size()>0)
    {
        if (l_message.at(0) == "NOTIFY * HTTP/1.1")
            format = NOTIFY;
        else if (l_message.at(0) == "M-SEARCH * HTTP/1.1")
            format = SEARCH;
        else if (l_message.at(0) == "HTTP/1.1 200 OK")
            format = HTTP;

        SsdpMessage message(format);

        for (int i=1;i<l_message.size();++i)
            message.addRawHeader(l_message.at(i));

        return message;
    }

    return SsdpMessage();
}

SsdpFormat SsdpMessage::format() const
{
    if (m_header.size()>0)
    {
        if (m_header.at(0) == "NOTIFY * HTTP/1.1")
            return NOTIFY;
        else if (m_header.at(0) == "M-SEARCH * HTTP/1.1")
            return SEARCH;
        else if (m_header.at(0) == "HTTP/1.1 200 OK")
            return HTTP;
    }

    return INVALID;
}

QString SsdpMessage::startLine() const
{
    if (m_header.size()>0)
    {
        return m_header.at(0);
    }
    else
    {
        return QString();
    }
}

int SsdpMessage::cacheControl() const
{
    QString cacheControl = getHeader("CACHE-CONTROL");
    if (!cacheControl.isEmpty())
    {
        QRegularExpression pattern("max-age\\s*=\\s*(\\d+)");
        QRegularExpressionMatch match = pattern.match(cacheControl);
        if (match.hasMatch())
        {
            return match.captured(1).toInt();
        }
        else
        {
            qCritical() << "Invalid format CACHE-CONTROL" << cacheControl;
        }
    }
    else
    {
        qCritical() << "no cache-control in SsdpMessage";
    }

    return -1;
}

QString SsdpMessage::getUuidFromUsn() const
{
    QRegularExpression usn("^uuid:([\\w\\-]+)");
    QRegularExpressionMatch match = usn.match(getHeader("USN"));

    if (match.hasMatch())
    {
        return match.captured(1);
    }

    return QString();
}
