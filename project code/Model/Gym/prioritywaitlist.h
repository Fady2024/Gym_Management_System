#ifndef PRIORITYWAITLIST_H
#define PRIORITYWAITLIST_H

#include <queue>
#include <vector>
#include <unordered_map>
#include <QDateTime>

struct GymWaitlistEntry {
    int memberId;
    bool isVIP;
    QDateTime joinTime;

    GymWaitlistEntry(int id, bool vip, const QDateTime& time)
        : memberId(id), isVIP(vip), joinTime(time) {}

    bool operator<(const GymWaitlistEntry& other) const {

        if (isVIP != other.isVIP) {
            return !isVIP;
        }

        return joinTime > other.joinTime;
    }
};

class PriorityWaitlist {
public:

    void addMember(int memberId, bool isVIP);

    void addMemberWithTime(int memberId, bool isVIP, const QDateTime& joinTime);

    bool removeMember(int memberId);

    int getNextMember() const;

    bool contains(int memberId) const;

    std::vector<int> getAllMembers() const;

    std::vector<GymWaitlistEntry> getAllEntries() const;

    size_t size() const;

    bool isEmpty() const;

    void clear();

private:
    std::priority_queue<GymWaitlistEntry> priorityQueue;
    std::unordered_map<int, bool> memberMap;
};

#endif