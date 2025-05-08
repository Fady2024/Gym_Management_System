#include "../Gym/class.h"
#include <algorithm>

//setters

void Class::setCapacity(int capacity)
{
    this->capacity = capacity;
}

void Class::setNumOfEnrolled(int numOfEnrolled)
{
    if (numOfEnrolled < static_cast<int>(enrolled_members.size())) {
        this->numOfEnrolled = static_cast<int>(enrolled_members.size());
    }
    else {
        this->numOfEnrolled = numOfEnrolled;
    }
}

void Class::setFromDate(const QDate& fromDate) {
    this->fromDate = fromDate;
}

void Class::setToDate(const QDate& toDate) {
    this->toDate = toDate;
}

void Class::setCoachName(const QString& coachName)
{
    this->coachName = coachName;
}

void Class::setClassName(const QString& className)
{
    this->className = className;
}

void Class::setId(int id)
{
    this->id = id;
}

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
int Class::getNumOfEnrolled() const { return numOfEnrolled; }
int Class::getId() const { return id; }

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
    return numOfEnrolled >= capacity;
}

// Waitlist management
void Class::addToWaitlist(int memberId) {
    // Check if the member is already in the waitlist
    if (!isInWaitlist(memberId)) {
        waiting_users_ids.push_back(memberId);
    }
}

void Class::removeFromWaitlist(int memberId) {
    auto it = std::find(waiting_users_ids.begin(), waiting_users_ids.end(), memberId);
    if (it != waiting_users_ids.end()) {
        waiting_users_ids.erase(it);
    }
}

int Class::getNextWaitlistMember() const {
    if (waiting_users_ids.empty()) {
        return -1; // Return -1 if no one is in the waitlist
    }
    return waiting_users_ids.front();
}

std::deque<int> Class::getWaitlist() const {
    return waiting_users_ids;
}

bool Class::isInWaitlist(int memberId) const {
    return std::find(waiting_users_ids.begin(), waiting_users_ids.end(), memberId) != waiting_users_ids.end();
}

void Class::assignFromWaitlistIfAvailable() {
    if (!waiting_users_ids.empty() && static_cast<int>(enrolled_members.size()) < capacity) {
        int nextId = waiting_users_ids.front();
        waiting_users_ids.pop_front();
        enrolled_members.insert(nextId);
        numOfEnrolled = static_cast<int>(enrolled_members.size());
        qDebug() << "Assigned member from waitlist:" << nextId;
    }
}

void Class::cancelEnrollment(int memberId) {
    if (enrolled_members.erase(memberId)) {
        numOfEnrolled = static_cast<int>(enrolled_members.size());
        assignFromWaitlistIfAvailable();
    }
}