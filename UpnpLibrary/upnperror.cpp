#include "upnperror.h"

UpnpError::UpnpError(QNetworkReply::NetworkError netError, QByteArray data, QObject *parent) :
    QObject(parent),
    m_netError(netError)
{
    m_data.setContent(data);

    // read UPNP error
    QDomElement body = m_data.documentElement().firstChildElement("s:Body");
    if (!body.isNull())
    {
        m_fault = body.firstChildElement("s:Fault");
        if (!m_fault.isNull())
        {
            m_detail = m_fault.firstChildElement("detail");
            if (!m_detail.isNull())
            {
                m_detail = m_detail.firstChildElement("UPnPError");
                if (m_detail.isNull())
                {
                    qCritical() << "invalid UPNP Error" << m_data.toString();
                }
            }
            else
            {
                qCritical() << "invalid UPNP Error" << m_data.toString();
            }
        }
        else
        {
            qCritical() << "invalid UPNP Error" << m_data.toString();
        }
    }
    else
    {
        qCritical() << "invalid UPNP Error" << m_data.toString();
    }

    if (faultCode() != "s:Client")
        qCritical() << "invalid faultCode" << faultCode();

    if (faultString() != "UPnPError")
        qCritical() << "invalid faultString" << faultString();
}

QNetworkReply::NetworkError UpnpError::netError() const
{
    return m_netError;
}

QString UpnpError::faultCode() const
{
    QDomElement elt = m_fault.firstChildElement("faultcode");
    if (!elt.isNull())
    {
        return elt.firstChild().nodeValue();
    }
    else
    {
        return QString();
    }
}

QString UpnpError::faultString() const
{
    QDomElement elt = m_fault.firstChildElement("faultstring");
    if (!elt.isNull())
    {
        return elt.firstChild().nodeValue();
    }
    else
    {
        return QString();
    }
}

int UpnpError::code() const
{
    QDomElement elt = m_detail.firstChildElement("errorCode");
    if (!elt.isNull())
    {
        return elt.firstChild().nodeValue().toInt();
    }
    else
    {
        return -5;
    }
}

QString UpnpError::description() const
{
    QDomElement elt = m_detail.firstChildElement("errorDescription");
    if (!elt.isNull())
    {
        return elt.firstChild().nodeValue();
    }
    else
    {
        return QString();
    }
}
