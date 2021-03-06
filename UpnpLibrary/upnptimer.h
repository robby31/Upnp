#ifndef UPNPTIMER_H
#define UPNPTIMER_H

#include <QTimer>

class UpnpTimer : public QTimer
{
    Q_OBJECT

public:
    explicit UpnpTimer(QObject *parent = Q_NULLPTR);
    explicit UpnpTimer(const int &max, const int &period, QObject *parent = Q_NULLPTR);

    void setMaxExecution(const int &max);
    void setPeriod(const int &period);

signals:

private slots:
    void timeoutSlot();

private:
    int m_counter = 1;
    int m_max = -1;
    int m_period = -1;
};

#endif // UPNPTIMER_H
