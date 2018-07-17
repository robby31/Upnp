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

SoapActionResponse::SoapActionResponse(QByteArray data, QObject *parent):
    QObject(parent),
    m_valid(false)
{
    if (m_xml.setContent(data, true))
    {
        QDomElement envelope = m_xml.firstChildElement("Envelope");
        if (!envelope.isNull())
        {
            QDomElement body = envelope.firstChildElement("Body");
            if (!body.isNull())
            {
                if (body.childNodes().size() == 1)
                {
                    m_xmlAction = body.firstChild().toElement();

                    m_serviceType = m_xmlAction.namespaceURI();

                    QRegularExpression pattern("^(\\w+)Response$");
                    QRegularExpressionMatch match = pattern.match(m_xmlAction.tagName());
                    if (match.hasMatch())
                    {
                        m_actionName = match.captured(1);
                    }

                    m_valid = !m_serviceType.isEmpty() && !m_actionName.isEmpty();
                }
                else
                {
                    m_valid = false;
                    qCritical() << "invalid format in body element";
                }
            }
            else
            {
                m_valid = false;
                qCritical() << "unable to find body";
            }
        }
        else
        {
            m_valid = false;
            qCritical() << "unable to find envelope";
        }
    }
    else
    {
        m_valid = false;
        qCritical() << "unable to set xml content";
    }
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

QStringList SoapActionResponse::arguments() const
{
   QStringList res;

   QDomNode node = m_xmlAction.firstChild();
   while (!node.isNull())
   {
       QDomElement elt = node.toElement();
       res << elt.tagName();
       node = node.nextSibling();
   }

   return res;
}

QString SoapActionResponse::value(const QString &argumentName) const
{
    QDomElement elt = m_xmlAction.firstChildElement(argumentName);
    if (elt.isNull())
    {
        return QString();
    }
    else
    {
        return elt.firstChild().toText().data();
    }
}

QByteArray SoapActionResponse::toByteArray() const
{
    return m_xml.toByteArray(-1);
}

