#include "upnpservicedescription.h"

UpnpServiceDescription::UpnpServiceDescription():
    UpnpDescription("scpd"),
    m_info("service")
{
    QDomElement root = m_xml.root();
    root.setAttribute("xmlns", "urn:schemas-upnp-org:service-1-0");
    root.setAttribute("configId", "0");

    QDomElement specVersion = m_xml.addParam("specVersion", root);
    m_xml.addParam("major", specVersion);
    m_xml.setParam("major", "1", specVersion);
    m_xml.addParam("minor", specVersion);
    m_xml.setParam("minor", "0", specVersion);

    m_xml.addParam("actionList", root);
    m_xml.addParam("serviceStateTable", root);

    m_info.addParam("serviceType", m_info.root());
    m_info.addParam("serviceId", m_info.root());
    m_info.addParam("SCPDURL", m_info.root());
    m_info.addParam("controlURL", m_info.root());
    m_info.addParam("eventSubURL", m_info.root());
}

QString UpnpServiceDescription::version() const
{
    QDomElement specVersion = m_xml.getParam("specVersion", m_xml.root(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
        return QString("null");
    }
    else
    {
        return QString("%1.%2").arg(m_xml.getParamValue("major", specVersion)).arg(m_xml.getParamValue("minor", specVersion));
    }
}

void UpnpServiceDescription::setVersion(const int &major, const int &minor)
{
    QDomElement specVersion = m_xml.getParam("specVersion", m_xml.root(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
    }
    else
    {
        m_xml.setParam("major", QVariant::fromValue(major).toString(), specVersion);
        m_xml.setParam("minor", QVariant::fromValue(minor).toString(), specVersion);
    }
}

void UpnpServiceDescription::setConfigId(const QString &id)
{
    QDomElement root = m_xml.root();
    root.setAttribute("configId", id);
}

QString UpnpServiceDescription::serviceAttribute(const QString &name) const
{
    return m_info.getParamValue(name);
}

void UpnpServiceDescription::setServiceAttribute(const QString &name, const QString &value)
{
    m_info.setParam(name, value);
}

QDomElement UpnpServiceDescription::xmlInfo()
{
    return m_info.root();
}

QDomElement UpnpServiceDescription::addAction(const QString &name)
{
    QDomElement actionList = m_xml.getParam("actionList", m_xml.root(), false);
    QDomElement action = m_xml.addParam("action", actionList);

    m_xml.addParam("name", action);
    m_xml.setParam("name", name, action);

    m_xml.addParam("argumentList", action);

    return action;
}

QDomElement UpnpServiceDescription::addArgument(const QDomElement &action, const QString &name, const DIRECTION &direction, const QString &stateVariable)
{
    if (!stateVariablesName().contains(stateVariable))
    {
        qCritical() << "invalid state variable name" << stateVariable;
        return QDomElement();
    }
    else
    {
        QDomElement argumentList = m_xml.getParam("argumentList", action);
        if (argumentList.isNull())
            argumentList = m_xml.addParam("argumentList", action);

        QDomElement argument = m_xml.addParam("argument", argumentList);

        m_xml.addParam("name", argument);
        m_xml.setParam("name", name, argument);

        m_xml.addParam("direction", argument);
        switch (direction) {
        case IN:
            m_xml.setParam("direction", "in", argument);
            break;
        case OUT:
            m_xml.setParam("direction", "out", argument);
            break;
        default:
            qCritical() << "invalid direction" << direction;
            break;
        }

        m_xml.addParam("relatedStateVariable", argument);
        m_xml.setParam("relatedStateVariable", stateVariable, argument);

        return argument;
    }
}

QDomElement UpnpServiceDescription::addStateVariable(const QString &name, const bool &sendEvents, const bool &multicast, const QString &type, const QString &defaultValue)
{
    QDomElement serviceStateTable = m_xml.getParam("serviceStateTable", m_xml.root(), false);

    QDomElement stateVariable = m_xml.addParam("stateVariable", serviceStateTable);

    if (sendEvents)
        stateVariable.setAttribute("sendEvents", "yes");
    else
        stateVariable.setAttribute("sendEvents", "no");

    if (multicast)
        stateVariable.setAttribute("multicast", "yes");
    else
        stateVariable.setAttribute("multicast", "no");

    m_xml.addParam("name", stateVariable);
    m_xml.setParam("name", name, stateVariable);

    m_xml.addParam("dataType", stateVariable);
    m_xml.setParam("dataType", type, stateVariable);

    m_xml.addParam("defaultValue", stateVariable);
    m_xml.setParam("defaultValue", defaultValue, stateVariable);

    return stateVariable;
}

QDomElement UpnpServiceDescription::addAllowedValueRange(const QDomElement &stateVariable, const QString &minimum, const QString &maximum, const QString &step)
{
    QDomElement range = m_xml.addParam("allowedValueRange", stateVariable);

    m_xml.addParam("minimum", range);
    m_xml.setParam("minimum", minimum, range);

    m_xml.addParam("maximum", range);
    m_xml.setParam("maximum", maximum, range);

    m_xml.addParam("step", range);
    m_xml.setParam("step", step, range);

    return range;
}

QDomElement UpnpServiceDescription::addAllowedValueList(const QDomElement &stateVariable, const QStringList &values)
{
    QDomElement list = m_xml.addParam("allowedValueList", stateVariable);

    foreach (const QString &value, values)
    {
        QDomElement param = m_xml.addParam("allowedValue", list);
        m_xml.setParam(param, value);
    }

    return list;
}

QString UpnpServiceDescription::stringInfo()
{
    return m_info.toString();
}

QStringList UpnpServiceDescription::actionsName()
{
    QStringList res;

    QDomElement actionList = m_xml.getParam("actionList", m_xml.root(), false);

    if (!actionList.isNull())
    {
        QDomNodeList l_action = actionList.elementsByTagName("action");
        for (int i=0;i<l_action.size();++i)
        {
            QDomNode action = l_action.at(i);
            res << action.firstChildElement("name").firstChild().nodeValue();
        }
    }
    else
    {
        qCritical() << "unable to find actionList element";
    }

    return res;
}

QStringList UpnpServiceDescription::stateVariablesName()
{
    QStringList res;

    QDomElement variableList = m_xml.getParam("serviceStateTable", m_xml.root(), false);

    if (!variableList.isNull())
    {
        QDomNodeList l_variables = variableList.elementsByTagName("stateVariable");
        for (int i=0;i<l_variables.size();++i)
        {
            QDomNode variable = l_variables.at(i);
            res << variable.firstChildElement("name").firstChild().nodeValue();
        }
    }
    else
    {
        qCritical() << "unable to find serviceStateTable element";
    }

    return res;
}

QDomElement UpnpServiceDescription::stateVariables()
{
    return m_xml.getParam("serviceStateTable", m_xml.root(), false);
}
