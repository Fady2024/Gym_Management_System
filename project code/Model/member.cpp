#include "member.h"

Member::Member(const QString& name, const QString& email, const QString& password,
               const QString& photoPath, const QDate& dateOfBirth,
               const QDate& subscriptionStart, const QDate& subscriptionEnd,
               int classId = -1)  // hy5aleh -1 law madanesh 7aga
    : User(name, email, password, photoPath, dateOfBirth),
      subscriptionStart(subscriptionStart),
      subscriptionEnd(subscriptionEnd),
      classId(classId)
{
}
//setter we getter so blah blah blah
QDate Member::getSubscriptionStart() const {
    return subscriptionStart;
}

QDate Member::getSubscriptionEnd() const {
    return subscriptionEnd;
}

void Member::setSubscriptionStart(const QDate& date) {
    subscriptionStart = date;
}

void Member::setSubscriptionEnd(const QDate& date) {
    subscriptionEnd = date;
}

void Member::addClassToHistory(const QDate& date) {
    history.append(qMakePair(classId, date));
}

QList<QPair<QString, QDate>> Member::getHistory() const {
    return history;
}

int Member::getClassId() const {
    return classId;
}

void Member::setClassId(int classId) {
    this->classId = classId;
}
