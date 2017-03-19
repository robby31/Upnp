#ifndef SOAPACTIONRESPONSE_H
#define SOAPACTIONRESPONSE_H

#include <QObject>

class SoapActionResponse : public QObject
{
    Q_OBJECT
public:
    explicit SoapActionResponse(QObject *parent = 0);

signals:

public slots:
};

#endif // SOAPACTIONRESPONSE_H