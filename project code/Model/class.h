#ifndef CLASS_H
#define CLASS_H

#include <QString>
#include <QDate>
#include <QTime>
#include <QVector>
#include <deque>
#include <queue>
#include <QJsonObject>

class Class
{
public:
	//setters
	void setClassName(const QString& className);
	void setCoachName(const QString& coachName);
	void setFrom(const QTime& from);
	void setTo(const QTime& to);
	void setCapacity(int capacity);
	void setNumOfEnrolled(int numOfEnrolled);
	void setId(int id);

	//getters
	[[nodiscard]] int getCapacity() const;
	[[nodiscard]] int getNumOfEnrolled() const;
	[[nodiscard]] int getId() const;
	[[nodiscard]] QTime getTo() const;
	[[nodiscard]] QTime getFrom() const;
	[[nodiscard]] QString getClassName() const;
	[[nodiscard]] QString getCoachName() const;
	
	// Session management
	void addSession(const QDate& date);
	void removeSession(const QDate& date);
	[[nodiscard]] std::queue<QDate> getSessions() const;
	[[nodiscard]] bool hasSessionOnDate(const QDate& date) const;
	
	// Capacity management
	[[nodiscard]] bool isFull() const;
	
	// Waitlist management
	void addToWaitlist(int memberId);
	void removeFromWaitlist(int memberId);
	[[nodiscard]] int getNextWaitlistMember() const;
	[[nodiscard]] std::deque<int> getWaitlist() const;
	[[nodiscard]] bool isInWaitlist(int memberId) const;

private:
	int id;
	QString className;
	QString coachName;
	std::queue<QDate> sessions;
	QTime from;
	QTime to;
	int capacity;
	int numOfEnrolled;
	std::deque<int>waiting_users_ids;
};
#endif
