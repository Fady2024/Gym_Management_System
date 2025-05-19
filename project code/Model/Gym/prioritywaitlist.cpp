#include "prioritywaitlist.h"
#include <QDateTime>

void PriorityWaitlist::addMember(int memberId, bool isVIP) {

    if (memberMap.find(memberId) != memberMap.end()) {
        return;
    }

    priorityQueue.push(GymWaitlistEntry(memberId, isVIP, QDateTime::currentDateTime()));

    memberMap[memberId] = true;
}

void PriorityWaitlist::addMemberWithTime(int memberId, bool isVIP, const QDateTime& joinTime) {

    if (memberMap.find(memberId) != memberMap.end()) {
        return;
    }

    priorityQueue.push(GymWaitlistEntry(memberId, isVIP, joinTime));

    memberMap[memberId] = true;
}

bool PriorityWaitlist::removeMember(int memberId) {

    if (memberMap.find(memberId) == memberMap.end()) {
        return false;
    }

    memberMap.erase(memberId);

    return true;
}

int PriorityWaitlist::getNextMember() const {
    if (priorityQueue.empty()) {
        return -1;
    }

    std::priority_queue<GymWaitlistEntry> tempQueue = priorityQueue;

    while (!tempQueue.empty()) {
        GymWaitlistEntry entry = tempQueue.top();
        tempQueue.pop();

        if (memberMap.find(entry.memberId) != memberMap.end()) {
            return entry.memberId;
        }
    }

    return -1;
}

bool PriorityWaitlist::contains(int memberId) const {
    return memberMap.find(memberId) != memberMap.end();
}

std::vector<int> PriorityWaitlist::getAllMembers() const {
    std::vector<int> result;

    if (priorityQueue.empty()) {
        return result;
    }

    std::priority_queue<GymWaitlistEntry> tempQueue = priorityQueue;

    while (!tempQueue.empty()) {
        GymWaitlistEntry entry = tempQueue.top();
        tempQueue.pop();

        if (memberMap.find(entry.memberId) != memberMap.end()) {
            result.push_back(entry.memberId);
        }
    }

    return result;
}

std::vector<GymWaitlistEntry> PriorityWaitlist::getAllEntries() const {
    std::vector<GymWaitlistEntry> result;

    if (priorityQueue.empty()) {
        return result;
    }

    std::priority_queue<GymWaitlistEntry> tempQueue = priorityQueue;

    while (!tempQueue.empty()) {
        GymWaitlistEntry entry = tempQueue.top();
        tempQueue.pop();

        if (memberMap.find(entry.memberId) != memberMap.end()) {
            result.push_back(entry);
        }
    }

    return result;
}

size_t PriorityWaitlist::size() const {
    return memberMap.size();
}

bool PriorityWaitlist::isEmpty() const {
    return memberMap.empty();
}

void PriorityWaitlist::clear() {
    while (!priorityQueue.empty()) {
        priorityQueue.pop();
    }
    memberMap.clear();
}