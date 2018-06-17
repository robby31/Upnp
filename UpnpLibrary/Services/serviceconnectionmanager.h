#ifndef SERVICECONNECTIONMANAGER_H
#define SERVICECONNECTIONMANAGER_H

#include <QObject>

class ServiceConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceConnectionManager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SERVICECONNECTIONMANAGER_H