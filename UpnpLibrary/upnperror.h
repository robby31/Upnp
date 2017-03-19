#ifndef UPNPERROR_H
#define UPNPERROR_H

#include <QObject>
#include <QDebug>
#include <QDomDocument>
#include <QNetworkReply>

class UpnpError : public QObject
{
    Q_OBJECT

public:
    enum ErrorTypes { INVALID_ACTION = 401, // No action by that name at this service.
                      INVALID_ARGS,         // Could be any of the following: not enough in args, no in arg by that name,
                                            // one or more in args are of the wrong data type.
                                            // Additionally, the UPnP Certification Test Tool Shall return the following warning message if there are too many in args:
                                            // ‘Sending too many in args is not recommended and may cause unexpected results’

                      ACTION_FAILED = 501,  // May be returned in current state of service prevents invoking that action.

                      ARGUMENT_VALUE_INVALID = 600,       // The argument value is invalid
                      ARGUMENT_VALUE_OUT_OF_RANGE,  // An argument value is less than the minimum or more than the maximum value of the allowedValueRange, or is not in the allowedValueList.
                      OPTIONAL_ACTION_NOT_IMPLEMENTED,   // The requested action is optional and is not implemented by the device.
                      OUT_OF_MEMORY,   // The device does not have sufficient memory available to complete the action.
                                       // This may be a temporary condition; the control point may choose to retry the unmodified request again later and it may succeed if memory is available.
                      HUMAN_INTERVENTION_REQUIRED,  // The device has encountered an error condition which it cannot resolve itself and required human intervention such as a reset or power cycle.
                                                    // See the device display or documentation for further guidance.
                      STRING_ARGUMENT_TOO_LONG,  // A string argument is too long for the device to handle properly.
                      ACTION_NOT_AUTHORIZED,     // The action requested requires authorization and the sender was not authorized.
                      SIGNATURE_FAILED,          // The sender's signature failed to verify.
                      SIGNATURE_MISSING,         // The action requested requires a digital signature and there was none provided.
                      NOT_ENCRYPTED,             // This action requires confidentiality but the action was not delivered encrypted.
                      INVALID_SEQUENCE,          // The <sequence> provided was not valid.
                      INVALID_CONTROL_URL,       // The controlURL within the <freshness> element does not match the controlURL of the action actually invoked (or the controlURL in the HTTP header).
                      NO_SUCH_SESSION,           // The session key reference is to a non-existent session.
                                                 // This could be because the device has expired a session, in which case the control point needs to open a new one.
                    };

    explicit UpnpError(QNetworkReply::NetworkError netError, QByteArray data, QObject *parent = 0);
    explicit UpnpError(ErrorTypes type, QObject *parent = 0);

    QNetworkReply::NetworkError netError() const;

    QString faultCode() const;
    QString faultString() const;

    int code() const;
    QString description() const;

    QByteArray toByteArray(const int indent = -1) const;

signals:

public slots:

private:
    QNetworkReply::NetworkError m_netError;
    QDomDocument m_data;
    QDomElement m_fault;
    QDomElement m_detail;
};

#endif // UPNPERROR_H
