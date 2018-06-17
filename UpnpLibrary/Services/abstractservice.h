#ifndef ABSTRACTSERVICE_H
#define ABSTRACTSERVICE_H

#include <QObject>

class AbstractService : public QObject
{
    Q_OBJECT
public:
    explicit AbstractService(QObject *parent = nullptr);

signals:

public slots:
};

#endif // ABSTRACTSERVICE_H