#include "upnptimer.h"

UpnpTimer::UpnpTimer(QObject *parent) :
    QTimer(parent),
    m_counter(1),
    m_max(-1),
    m_period(-1)
{
}

UpnpTimer::UpnpTimer(const int &max, const int &period, QObject *parent) :
    QTimer(parent),
    m_counter(1),
    m_max(max),
    m_period(period)
{
    connect(this, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
}

void UpnpTimer::timeoutSlot()
{
    if (m_counter >= m_max)
    {
        start(m_period);

        // stop counting
        m_counter = -1;
    }
    else if (m_counter >= 0)
    {
        // increment if counter > 0
        ++m_counter;
    }
}

void UpnpTimer::setMaxExecution(const int &max)
{
    if (max > 0)
        m_max = max;
}

void UpnpTimer::setPeriod(const int &period)
{
    if (period > 0)
        m_period = period;
}

