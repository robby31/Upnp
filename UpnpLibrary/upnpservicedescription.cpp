#include "upnpservicedescription.h"

UpnpServiceDescription::UpnpServiceDescription():
    UpnpDescription("scpd"),
    m_info("service")
{
    QDomElement root = xmlDescription();
    root.setAttribute("xmlns", "urn:schemas-upnp-org:service-1-0");
    root.setAttribute("configId", "0");

    QDomElement specVersion = addParam("specVersion", root);
    addParam("major", specVersion);
    setParam("major", "1", specVersion);
    addParam("minor", specVersion);
    setParam("minor", "0", specVersion);

    addParam("actionList", root);
    addParam("serviceStateTable", root);

    m_info.addParam("serviceType", m_info.root());
    m_info.addParam("serviceId", m_info.root());
    m_info.addParam("SCPDURL", m_info.root());
    m_info.addParam("controlURL", m_info.root());
    m_info.addParam("eventSubURL", m_info.root());
}

QString UpnpServiceDescription::version() const
{
    QDomElement specVersion = getParam("specVersion", xmlDescription(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
        return QString("null");
    }

    return QString("%1.%2").arg(getParamValue("major", specVersion), getParamValue("minor", specVersion));
}

void UpnpServiceDescription::setVersion(const int &major, const int &minor)
{
    QDomElement specVersion = getParam("specVersion", xmlDescription(), false);
    if (specVersion.isNull())
    {
        qCritical() << "unable to find specVersion";
    }
    else
    {
        setParam("major", QVariant::fromValue(major).toString(), specVersion);
        setParam("minor", QVariant::fromValue(minor).toString(), specVersion);
    }
}

void UpnpServiceDescription::setConfigId(const QString &id)
{
    QDomElement root = xmlDescription();
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
    QDomElement actionList = getParam("actionList", xmlDescription(), false);
    QDomElement action = addParam("action", actionList);

    addParam("name", action);
    setParam("name", name, action);

    addParam("argumentList", action);

    return action;
}

QDomElement UpnpServiceDescription::addArgument(const QDomElement &action, const QString &name, const DIRECTION &direction, const QString &stateVariable)
{
    if (!stateVariablesName().contains(stateVariable))
    {
        qCritical() << "invalid state variable name" << stateVariable;
        return QDomElement();
    }

    QDomElement argumentList = getParam("argumentList", action);
    if (argumentList.isNull())
        argumentList = addParam("argumentList", action);

    QDomElement argument = addParam("argument", argumentList);

    addParam("name", argument);
    setParam("name", name, argument);

    addParam("direction", argument);
    switch (direction) {
    case IN:
        setParam("direction", "in", argument);
        break;
    case OUT:
        setParam("direction", "out", argument);
        break;
    default:
        qCritical() << "invalid direction" << direction;
        break;
    }

    addParam("relatedStateVariable", argument);
    setParam("relatedStateVariable", stateVariable, argument);

    return argument;
}

QDomElement UpnpServiceDescription::addStateVariable(const QString &name, const bool &sendEvents, const bool &multicast, const QString &type, const QString &defaultValue)
{
    QDomElement serviceStateTable = getParam("serviceStateTable",xmlDescription(), false);

    QDomElement stateVariable = addParam("stateVariable", serviceStateTable);

    if (sendEvents)
        stateVariable.setAttribute("sendEvents", "yes");
    else
        stateVariable.setAttribute("sendEvents", "no");

    if (multicast)
        stateVariable.setAttribute("multicast", "yes");
    else
        stateVariable.setAttribute("multicast", "no");

    addParam("name", stateVariable);
    setParam("name", name, stateVariable);

    addParam("dataType", stateVariable);
    setParam("dataType", type, stateVariable);

    addParam("defaultValue", stateVariable);
    setParam("defaultValue", defaultValue, stateVariable);

    return stateVariable;
}

QDomElement UpnpServiceDescription::addAllowedValueRange(const QDomElement &stateVariable, const QString &minimum, const QString &maximum, const QString &step)
{
    QDomElement range = addParam("allowedValueRange", stateVariable);

    addParam("minimum", range);
    setParam("minimum", minimum, range);

    addParam("maximum", range);
    setParam("maximum", maximum, range);

    addParam("step", range);
    setParam("step", step, range);

    return range;
}

QDomElement UpnpServiceDescription::addAllowedValueList(const QDomElement &stateVariable, const QStringList &values)
{
    QDomElement list = addParam("allowedValueList", stateVariable);

    foreach (const QString &value, values)
    {
        QDomElement param = addParam("allowedValue", list);
        setParam(param, value);
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

    QDomElement actionList = getParam("actionList", xmlDescription(), false);

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

    QDomElement variableList = getParam("serviceStateTable", xmlDescription(), false);

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
    return getParam("serviceStateTable", xmlDescription(), false);
}
