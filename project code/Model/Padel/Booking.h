#ifndef BOOKING_H
#define BOOKING_H

#include <QString>
#include <QDateTime>
#include "Court.h"

class Booking {
public:
    Booking();
    Booking(int bookingId, const Court& court, const QDateTime& start, const QDateTime& end, const QString& memberName);

    int getBookingId() const;
    Court getCourt() const;
    QDateTime getStartTime() const;
    QDateTime getEndTime() const;
    QString getMemberName() const;

    void setBookingId(int id);
    void setCourt(const Court& court);
    void setStartTime(const QDateTime& start);
    void setEndTime(const QDateTime& end);
    void setMemberName(const QString& name);

private:
    int m_bookingId;
    Court m_court;
    QDateTime m_startTime;
    QDateTime m_endTime;
    QString m_memberName;
};

#endif // BOOKING_H
