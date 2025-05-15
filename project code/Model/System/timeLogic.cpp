#include "timeLogic.h"
#include <QDebug>
#include <QThread>

TimeLogic::TimeLogic()
    : multiplier(1.0f), paused(false), running(true) {
    currentTime = QDateTime::currentDateTime();
    start();  // Start the thread
}

TimeLogic::~TimeLogic() {
    {
        QMutexLocker locker(&mutex);
        running = false;
        paused = false;
    }
    pauseCond.wakeAll(); // In case it's paused
    wait(); // Wait for thread to finish
}

void TimeLogic::run() {
    while (true) {
        {
            QMutexLocker locker(&mutex);
            if (!running)
                break;
            if (paused)
                pauseCond.wait(&mutex);
        }

        QThread::msleep(static_cast<unsigned long>(1000.0 / multiplier));

        {
            QMutexLocker locker(&mutex);
            currentTime = currentTime.addSecs(1);
            qDebug() << currentTime.toString("ddd MMM dd hh:mm:ss yyyy");
        }
    }
}

void TimeLogic::pauseTime() {
    QMutexLocker locker(&mutex);
    paused = !paused;
    if (!paused)
        pauseCond.wakeAll();
}

void TimeLogic::setMultiplier(float m) {
    QMutexLocker locker(&mutex);
    multiplier = (m > 0.0f) ? m : 1.0f;
}
float TimeLogic::getMultiplier() {
    return multiplier;
}
void TimeLogic::incrementDays(int i) {
    QMutexLocker locker(&mutex);
    currentTime = currentTime.addDays(i);
}

QDateTime TimeLogic::getCurrentTime() {
    QMutexLocker locker(&mutex);
    return currentTime;
}

QString TimeLogic::getFormattedTime() {
    QMutexLocker locker(&mutex);
    return currentTime.toString("ddd MMM dd hh:mm:ss yyyy");
}
TimeLogic timeLogicInstance;