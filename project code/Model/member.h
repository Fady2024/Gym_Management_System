#ifndef MEMBER_H
#define MEMBER_H

#include "user.h"
#include <QList>
#include <QPair>

class Member : public User // ba inherit 3shan mekasel
{
public:
  Member();
  Member(const QString &name, const QString &email, const QString &password,
         const QString &photoPath, const QDate &dateOfBirth,
         const QDate &subscriptionStart, const QDate &subscriptionEnd,
         int classId = -1); // bos ya m3alem -1 = not enrolled yet

  QDate getSubscriptionStart() const;
  QDate getSubscriptionEnd() const;
  void setSubscriptionStart(const QDate &date);
  void setSubscriptionEnd(const QDate &date);

  void addClassToHistory(const QDate &date);
  QList<QPair<int, QDate>> getHistory() const;

  int getClassId() const;
  void setClassId(int classId);

private:
  QDate subscriptionStart;
  QDate subscriptionEnd;
  QList<QPair<int, QDate>> history;
  int classId;
};

#endif
