#include "upnperror.h"

UpnpError::UpnpError() :
    m_netError(QNetworkReply::NoError)
{
    m_data.clear();
    m_fault.clear();
    m_detail.clear();
}

UpnpError::UpnpError(QNetworkReply::NetworkError netError, QByteArray data) :
    m_netError(netError)
{
    m_data.setContent(data, true);

    // read UPNP error
    QDomElement body = m_data.documentElement().firstChildElement("Body");
    if (!body.isNull())
    {
        m_fault = body.firstChildElement("Fault");
        if (!m_fault.isNull())
        {
            m_detail = m_fault.firstChildElement("detail");
            if (!m_detail.isNull())
            {
                m_detail = m_detail.firstChildElement("UPnPError");
                if (m_detail.isNull())
                {
                    qCritical() << "invalid UPNP Error (wrong detail)" << m_data.toString();
                }
            }
            else
            {
                qCritical() << "invalid UPNP Error (detail not found)" << m_data.toString();
            }
        }
        else
        {
            qCritical() << "invalid UPNP Error (Fault not found)" << m_data.toString();
        }
    }
    else
    {
        qCritical() << "invalid UPNP Error (body not found)" << m_data.toString();
    }

    if (faultCode() != "s:Client")
        qCritical() << "invalid faultCode" << faultCode();

    if (faultString() != "UPnPError")
        qCritical() << "invalid faultString" << faultString();
}

UpnpError::UpnpError(ErrorTypes type):
    m_netError(QNetworkReply::InternalServerError)
{
    m_data.appendChild(m_data.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement envelope = m_data.createElementNS("http://schemas.xmlsoap.org/soap/envelope/", "s:Envelope");
    envelope.setAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
    m_data.appendChild(envelope);

    QDomElement body = m_data.createElement("s:Body");
    envelope.appendChild(body);

    m_fault = m_data.createElement("s:Fault");
    body.appendChild(m_fault);

    QDomElement faultCode = m_data.createElement("faultcode");
    faultCode.appendChild(m_data.createTextNode("s:Client"));
    m_fault.appendChild(faultCode);

    QDomElement faultString = m_data.createElement("faultstring");
    faultString.appendChild(m_data.createTextNode("UPnPError"));
    m_fault.appendChild(faultString);

    QDomElement detail = m_data.createElement("detail");
    m_fault.appendChild(detail);

    m_detail = m_data.createElementNS("urn:schemas-upnp-org:control-1-0", "UPnPError");
    detail.appendChild(m_detail);

    QDomElement errorCode = m_data.createElement("errorCode");
    QDomElement errorDescription = m_data.createElement("errorDescription");

    errorCode.appendChild(m_data.createTextNode(QString("%1").arg(type)));

    if (type == BAD_REQUEST)
    {
        errorDescription.appendChild(m_data.createTextNode("Bad Request"));
    }
    else if (type == INVALID_ACTION)
    {
        errorDescription.appendChild(m_data.createTextNode("Invalid Action"));
    }
    else if (type == INVALID_ARGS)
    {
        errorDescription.appendChild(m_data.createTextNode("Invalid Args"));
    }
    else if (type == PRECONDITIN_FAILED)
    {
        errorDescription.appendChild(m_data.createTextNode("Precondition Failed"));
    }
    else if (type == ACTION_FAILED)
    {
        errorDescription.appendChild(m_data.createTextNode("Action Failed"));
    }
    else if (type == ARGUMENT_VALUE_INVALID)
    {
        errorDescription.appendChild(m_data.createTextNode("Argument Value Invalid"));
    }
    else if (type == ARGUMENT_VALUE_OUT_OF_RANGE)
    {
        errorDescription.appendChild(m_data.createTextNode("Argument Value Out of Range"));
    }
    else if (type == OPTIONAL_ACTION_NOT_IMPLEMENTED)
    {
        errorDescription.appendChild(m_data.createTextNode("Optional Action Not Implemented"));
    }
    else if (type == OUT_OF_MEMORY)
    {
        errorDescription.appendChild(m_data.createTextNode("Out of Memory"));
    }
    else if (type == HUMAN_INTERVENTION_REQUIRED)
    {
        errorDescription.appendChild(m_data.createTextNode("Human Intervention Required"));
    }
    else if (type == STRING_ARGUMENT_TOO_LONG)
    {
        errorDescription.appendChild(m_data.createTextNode("String Argument Too Long"));
    }
    else if (type == ACTION_NOT_AUTHORIZED)
    {
        errorDescription.appendChild(m_data.createTextNode("Action not authorized"));
    }
    else if (type == SIGNATURE_FAILED)
    {
        errorDescription.appendChild(m_data.createTextNode("Signature failure"));
    }
    else if (type == SIGNATURE_MISSING)
    {
        errorDescription.appendChild(m_data.createTextNode("Signature missing"));
    }
    else if (type == NOT_ENCRYPTED)
    {
        errorDescription.appendChild(m_data.createTextNode("Not encrypted"));
    }
    else if (type == INVALID_SEQUENCE)
    {
        errorDescription.appendChild(m_data.createTextNode("Invalid sequence"));
    }
    else if (type == INVALID_CONTROL_URL)
    {
        errorDescription.appendChild(m_data.createTextNode("Invalid control URL"));
    }
    else if (type == NO_SUCH_SESSION)
    {
        errorDescription.appendChild(m_data.createTextNode("No such session"));
    }
    else if (type == INVALID_OBJECT)
    {
        errorDescription.appendChild(m_data.createTextNode("No such object"));
    }
    else if (type == INVALID_CONNECTION)
    {
        errorDescription.appendChild(m_data.createTextNode("Invalid connection reference"));
    }
    else if (type == INVALID_PROCESS_REQUEST)
    {
        errorDescription.appendChild(m_data.createTextNode("Cannot process the request"));
    }
    else
    {
        qCritical() << "invalid type" << type;
    }

    m_detail.appendChild(errorCode);
    m_detail.appendChild(errorDescription);
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

QByteArray UpnpError::toByteArray(const int indent) const
{
    return m_data.toByteArray(indent);
}
