#include "elapsedtimer.h"

ElapsedTimer::ElapsedTimer(QObject *parent):
    QObject(parent)
{
    DebugInfo::add_object(this);
    timer.invalidate();
}

ElapsedTimer::~ElapsedTimer() {
    DebugInfo::remove_object(this);
}

void ElapsedTimer::start()
{
    if (isPaused) {
        if (timer.isValid()) {
            if (timer.elapsed() < 2000) {
                // cancel pause
                m_elapsedFromStarting += timer.restart();
                isPaused = false;
            } else {
                timer.start();
                isPaused = false;
            }
        } else {
            timer.start();
            isPaused = false;
        }
    }
}

void ElapsedTimer::pause()
{
    if (!isPaused && timer.isValid()) {
        m_elapsedFromStarting += timer.restart();
        isPaused = true;
        ++m_pausedCounter;
    }
}

qint64 ElapsedTimer::elapsed() const
{
    if (!timer.isValid())
        return 0;

    if (isPaused)
        return m_elapsedFromStarting;

    return timer.elapsed() + m_elapsedFromStarting;
}

qint64 ElapsedTimer::pausedCounter() const
{
    return m_pausedCounter;
}
