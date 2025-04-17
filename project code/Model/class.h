#ifndef CLASS_H
#define CLASS_H

#include <QString>
#include <QDate>
#include <queue>
#include<deque>

class Class
{
public:
	Class();

	//setters
	void setClassName(const QString& className);
	void setCoachName(const QString& coachName);
	void setFrom(const QTime& from);
	void setTo(const QTime& to);
	void setCapacity(int capacity);
	void setNumOfEnrolled(int numOfEnrolled);

	//getters
	[[nodiscard]] int getCapacity() const;
	[[nodiscard]] int getNumOfEnrolled() const;
	[[nodiscard]] QTime getTo() const;
	[[nodiscard]] QTime getFrom() const;
	[[nodiscard]] QString getClassName() const;
	[[nodiscard]] QString getCoachName() const;

private:
	QString className;
	QString coachName;
	queue<QDate> sessions;
	QTime from;
	QTime to;
	int capacity;
	int numOfEnrolled;
	deque<int>waiting_users_ids
};