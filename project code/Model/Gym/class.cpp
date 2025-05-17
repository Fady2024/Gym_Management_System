#include "../Gym/class.h"
#include <algorithm>

//setters

void Class::setCapacity(int capacity) 
{ this->capacity = capacity; }

void Class::setNumOfEnrolled(int numOfEnrolled) 
{ 
    this->numOfEnrolled = static_cast<int>(enrolled_members.size());
}

void Class::setFromDate(const QDate& fromDate) {
    this->fromDate = fromDate;
}

void Class::setToDate(const QDate& toDate) {
    this->toDate = toDate;
}

void Class::setCoachName(const QString& coachName) 
{this->coachName = coachName;}

void Class::setClassName(const QString& className)
{this->className = className;}

void Class::setId(int id)
{this->id = id;}

//getters

QString Class::getClassName() const { return className; }
QString Class::getCoachName() const { return coachName; }
QDate Class::getFromDate() const {
    return fromDate;
}

QDate Class::getToDate() const {
    return toDate;
}
int Class::getCapacity() const { return capacity; }
int Class::getNumOfEnrolled() const { return static_cast<int>(enrolled_members.size()); }
int Class::getId() const {return id;}

// Session management
void Class::addSession(const QDate& date) {
    sessions.push(date);
}

void Class::removeSession(const QDate& date) {
    std::queue<QDate> tempSessions;
    
    while (!sessions.empty()) {
        QDate currentDate = sessions.front();
        sessions.pop();
        
        if (currentDate != date) {
            tempSessions.push(currentDate);
        }
    }
    
    sessions = tempSessions;
}

std::queue<QDate> Class::getSessions() const {
    return sessions;
}

bool Class::hasSessionOnDate(const QDate& date) const {
    std::queue<QDate> tempSessions = sessions;
    
    while (!tempSessions.empty()) {
        if (tempSessions.front() == date) {
            return true;
        }
        tempSessions.pop();
    }
    
    return false;
}

// Capacity management
bool Class::isFull() const {
    return getNumOfEnrolled() >= capacity;
}

void Class::addToWaitlist(int memberId, bool isVIP) {
    waitlist.addMember(memberId, isVIP);
}

void Class::addToWaitlistWithTime(int memberId, bool isVIP, const QDateTime& joinTime) {
    waitlist.addMemberWithTime(memberId, isVIP, joinTime);
}

bool Class::removeFromWaitlist(int memberId) {
    return waitlist.removeMember(memberId);
}

int Class::getNextWaitlistMember() const {
    return waitlist.getNextMember();
}

std::vector<int> Class::getWaitlist() const {
    return waitlist.getAllMembers();
}

std::vector<GymWaitlistEntry> Class::getWaitlistEntries() const {
    return waitlist.getAllEntries();
}

bool Class::isInWaitlist(int memberId) const {
    return waitlist.contains(memberId);
}

size_t Class::getWaitlistSize() const {
    return waitlist.size();
}