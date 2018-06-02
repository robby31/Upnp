#ifndef UPNPSERVICEDESCRIPTION_H
#define UPNPSERVICEDESCRIPTION_H

#include "upnpdescription.h"

class UpnpServiceDescription : public UpnpDescription
{

public:
    enum DIRECTION { IN, OUT };

    UpnpServiceDescription();

    void setConfigId(const QString &id);

    QString version() const;
    void setVersion(const int &major, const int &minor);

    QString serviceAttribute(const QString &name) const;
    void setServiceAttribute(const QString &name, const QString &value);

    QDomElement addAction(const QString &name);
    QDomElement addArgument(const QDomElement &action, const QString &name, const DIRECTION &direction, const QString &stateVariable);
    QStringList actionsName();

    QDomElement addStateVariable(const QString &name, const bool &sendEvents, const bool &multicast, const QString &type, const QString &defaultValue);
    QDomElement addAllowedValueRange(const QDomElement &stateVariable, const QString &minimum, const QString &maximum, const QString &step);
    QDomElement addAllowedValueList(const QDomElement &stateVariable, const QStringList &values);
    QStringList stateVariablesName();

    QDomElement xmlInfo();
    QString stringInfo();

private:
    QStringList m_actions;

    XmlDescription m_info;
};

#endif // UPNPSERVICEDESCRIPTION_H
