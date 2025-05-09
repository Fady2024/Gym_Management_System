#ifndef CLASS_H
#define CLASS_H

#include <QString>
#include <QDate>
#include <QTime>
#include <QVector>
#include <deque>
#include <queue>
#include <QJsonObject>
#include <set>

class Class
{
public:
	Class() : id(0), capacity(0), numOfEnrolled(0) {}
	//setters
	void setClassName(const QString& className);
	void setCoachName(const QString& coachName);
	void setFromDate(const QDate& fromDate);
	void setToDate(const QDate& toDate);
	void setCapacity(int capacity);
	void setNumOfEnrolled(int numOfEnrolled);
	void setId(int id);

	//getters
	[[nodiscard]] int getCapacity() const;
	[[nodiscard]] int getNumOfEnrolled() const;
	[[nodiscard]] int getId() const;
	[[nodiscard]] QDate getToDate() const;
	[[nodiscard]] QDate getFromDate() const; //kant time we heya bel shohor .-.
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
	void addToWaitlist(int memberId, bool isVIP);
	void removeFromWaitlist(int memberId);
	[[nodiscard]] int getNextWaitlistMember() const;
	[[nodiscard]] std::deque<int> getWaitlist() const;
	[[nodiscard]] bool isInWaitlist(int memberId) const;
	void assignFromWaitlistIfAvailable();
	void cancelEnrollment(int memberId);

	// Member management
	void addMember(int memberId) { enrolled_members.insert(memberId); }
	void removeMember(int memberId) { enrolled_members.erase(memberId); }
	bool isMemberEnrolled(int memberId) const { return enrolled_members.find(memberId) != enrolled_members.end(); }
	const std::set<int>& getEnrolledMembers() const { return enrolled_members; }

private:
	int id;
	QString className;
	QString coachName;
	std::queue<QDate> sessions;
	QDate fromDate;
	QDate toDate;
	int capacity;
	int numOfEnrolled;
	std::deque<int> waiting_users_ids;
	std::set<int> enrolled_members;  // New member tracking
	int numofvipInWaitlist;
};
#endif