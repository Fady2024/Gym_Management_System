#ifndef BOOKING_H
#define BOOKING_H

#include <QString>
#include <QDateTime>
#include "Court.h"

class Booking {
public:
    Booking();
    Booking(int bookingId, const Court& court, const QDateTime& start, const QDateTime& end);

    int getBookingId() const;
    Court getCourt() const;
    QDateTime getStartTime() const;
    QDateTime getEndTime() const;
   

    void setBookingId(int id);
    void setCourt(const Court& court);
    void setStartTime(const QDateTime& start);
    void setEndTime(const QDateTime& end);
   

private:
    int m_bookingId;
    Court m_court;
    QDateTime m_startTime;
    QDateTime m_endTime;
    QString m_memberName;
    QDate m_date;
};

#endif // BOOKING_H
