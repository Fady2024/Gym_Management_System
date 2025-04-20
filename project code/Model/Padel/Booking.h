#ifndef BOOKING_H
#define BOOKING_H

#include <QString>
#include <QDateTime>
#include "Court.h"
#include<./user.h>

class Booking {
public:
    Booking();
    Booking(int bookingId, const Court& court, const QDateTime& start, const QDateTime& end, User user);

    int getBookingId() const;
    User getuser() const;
    Court getCourt() const;
    QDateTime getStartTime() const;
    QDateTime getEndTime() const;
   

    void setBookingId(int id);
    void setuser(User user);
    void setCourt(const Court& court);
    void setStartTime(const QDateTime& start);
    void setEndTime(const QDateTime& end);
   

private:
    int m_bookingId;
    User m_user;
    Court m_court;
    QDateTime m_startTime;
    QDateTime m_endTime;
    QString m_memberName;
    QDate m_date;
};

#endif // BOOKING_H
