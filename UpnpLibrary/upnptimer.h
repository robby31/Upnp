#ifndef UPNPTIMER_H
#define UPNPTIMER_H

#include <QTimer>

class UpnpTimer : public QTimer
{
    Q_OBJECT

public:
    explicit UpnpTimer(QObject *parent = 0);
    explicit UpnpTimer(const int &max, const int &period, QObject *parent = 0);

    void setMaxExecution(const int &max);
    void setPeriod(const int &period);

signals:

private slots:
    void timeoutSlot();

private:
    int m_counter;
    int m_max;
    int m_period;
};

#endif // UPNPTIMER_H
