#include "booking.h"
#include<./user.h>
//#include "Court.h"
Booking::Booking() {}

Booking::Booking(int bookingId, const Court& court, const QDateTime& start, const QDateTime& end, User user)
    : m_bookingId(bookingId), m_court(court), m_startTime(start), m_endTime(end),m_user(user){
 
}

int Booking::getBookingId() const { return m_bookingId; }
User Booking::getuser() const { return m_user; }
Court Booking::getCourt() const { return m_court; }
QDateTime Booking::getStartTime() const { return m_startTime; }
QDateTime Booking::getEndTime() const { return m_endTime; }


void Booking::setBookingId(int id) { m_bookingId = id; }
void Booking::setuser(User u) { m_user = u; }
void Booking::setCourt(const Court& court) { m_court = court; }
void Booking::setStartTime(const QDateTime& start) { m_startTime = start; }
void Booking::setEndTime(const QDateTime& end) { m_endTime = end; }

