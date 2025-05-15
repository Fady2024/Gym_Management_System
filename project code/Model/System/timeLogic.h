#ifndef TIMELOGIC_H
#define TIMELOGIC_H

#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <QDateTime>
using namespace std;
class TimeLogic {
private:
    QDateTime currentTime;
    float multiplier;
    thread timeThread;
    void incrementTime();
public:
    TimeLogic();
    ~TimeLogic();
    float getMultiplier();
    void incrementDays(int i);
    void setMultiplier(float newMultiplier);
    QDateTime getCurrentTime();
	QString getFormattedTime();
};
extern TimeLogic timeLogicInstance;

#endif
