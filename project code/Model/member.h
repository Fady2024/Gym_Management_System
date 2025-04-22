#ifndef MEMBER_H
#define MEMBER_H

#include "user.h"
#include <QList>
#include <QPair>
#include "subscription.h"

class Member {
public:
  Member();
  // Constructor that takes a User reference and member-specific data
  Member(int memberId, int userId, int classId = -1);
  
  // Getters
  int getId() const { return memberId; }
  int getUserId() const { return userId; }
  int getClassId() const { return classId; }
  const Subscription& getSubscription() const { return subscription; }
  QList<QPair<int, QDate>> getHistory() const { return history; }
  
  // Setters
  void setId(int id) { memberId = id; }
  void setUserId(int id) { userId = id; }
  void setClassId(int id) { classId = id; }
  void setSubscription(const Subscription& sub) { subscription = sub; }
  bool hasActiveSubscription() const { return subscription.isActive(); }
  
  // History management
  void addClassToHistory(const QDate &date);
  
  // Convert to string for debugging
  QString toString() const;

private:
  int memberId;
  int userId;
  int classId;
  Subscription subscription;
  QList<QPair<int, QDate>> history;
};

#endif
