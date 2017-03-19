#include "soapactionresponse.h"

SoapActionResponse::SoapActionResponse(QString serviceType, QString actionName, QObject *parent):
    QObject(parent),
    m_valid(true),
    m_serviceType(serviceType),
    m_actionName(actionName)
{
    m_xml.appendChild(m_xml.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement envelope = m_xml.createElementNS("http://schemas.xmlsoap.org/soap/envelope/", "s:Envelope");
    envelope.setAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
    m_xml.appendChild(envelope);

    QDomElement body = m_xml.createElement("s:Body");
    envelope.appendChild(body);

    m_xmlAction = m_xml.createElementNS(serviceType, "u:"+actionName+"Response");
    body.appendChild(m_xmlAction);
}

bool SoapActionResponse::isValid() const
{
    return m_valid;
}

QString SoapActionResponse::serviceType() const
{
    return m_serviceType;
}

QString SoapActionResponse::actionName() const
{
    return m_actionName;
}

bool SoapActionResponse::addArgument(const QString &name, const QString &value)
{
    if (!m_xmlAction.elementsByTagName(name).isEmpty())
    {
        qCritical() << "argument" << name << "already exists.";
        return false;
    }
    else
    {
        QDomElement arg = m_xml.createElement(name);
        arg.appendChild(m_xml.createTextNode(value));
        m_xmlAction.appendChild(arg);
        return true;
    }
}

QByteArray SoapActionResponse::toByteArray() const
{
    return m_xml.toByteArray(-1);
}

