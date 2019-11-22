#ifndef ELAPSEDTIMER_H
#define ELAPSEDTIMER_H

#include <QElapsedTimer>
#include <QDebug>
#include "debuginfo.h"

class ElapsedTimer : public QObject
{
    Q_OBJECT

public:
    explicit ElapsedTimer(QObject *parent = Q_NULLPTR);

    void invalidate() { timer.invalidate(); }
    bool isValid() const { return timer.isValid(); }

    void start();
    void pause();
    bool isStatePaused() { return isPaused; }

    qint64 pausedCounter() const;

    qint64 elapsed() const;
    qint64 elapsedFromBeginning() const { return m_elapsedBeforeStarting+elapsed(); }
    void addMSec(const qint64 &time) { m_elapsedBeforeStarting += time; }

private:
    QElapsedTimer timer;
    qint64 m_elapsedBeforeStarting = 0;
    qint64 m_elapsedFromStarting = 0;
    bool isPaused = true;
    qint64 m_pausedCounter = 0;
};

#endif // ELAPSEDTIMER_H
