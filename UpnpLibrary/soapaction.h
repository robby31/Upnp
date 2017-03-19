#ifndef SOAPACTION_H
#define SOAPACTION_H

#include <QObject>

class SoapAction : public QObject
{
    Q_OBJECT
public:
    explicit SoapAction(QObject *parent = 0);

signals:

public slots:
};

#endif // SOAPACTION_H