#ifndef TIMELOGIC_H
#define TIMELOGIC_H

#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <QThread>
using namespace std;
class MemberDataManager;
class TimeLogic : public QThread {
    Q_OBJECT

private:
    QDateTime currentTime;
    float multiplier;
    bool paused;
    bool running;
    QMutex mutex;
    QWaitCondition pauseCond;
    int currentMemberId;
    MemberDataManager* memberDataManager;

    void run() override;
public:
    TimeLogic();
    ~TimeLogic() override;
    void pauseTime();
    float getMultiplier();
    void setMultiplier(float newMultiplier);
    void incrementDays(int i);
    QDateTime getCurrentTime();
	QString getFormattedTime();
    void getRemindersSubEnd();
    bool hasActiveSubscription() const;
    void setCurrentMemberId(int memberId);
    void setMemberDataManager(MemberDataManager* dataManager);
};
extern TimeLogic timeLogicInstance;

#endif
