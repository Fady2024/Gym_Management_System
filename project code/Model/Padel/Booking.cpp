#include "booking.h"
#include "Court.h"
Booking::Booking() {}

Booking::Booking(int bookingId, const Court& court, const QDateTime& start, const QDateTime& end, const QString& memberName)
    : m_bookingId(bookingId), m_court(court), m_startTime(start), m_endTime(end), m_memberName(memberName) {}

int Booking::getBookingId() const { return m_bookingId; }
Court Booking::getCourt() const { return m_court; }
QDateTime Booking::getStartTime() const { return m_startTime; }
QDateTime Booking::getEndTime() const { return m_endTime; }
QString Booking::getMemberName() const { return m_memberName; }

void Booking::setBookingId(int id) { m_bookingId = id; }
void Booking::setCourt(const Court& court) { m_court = court; }
void Booking::setStartTime(const QDateTime& start) { m_startTime = start; }
void Booking::setEndTime(const QDateTime& end) { m_endTime = end; }
void Booking::setMemberName(const QString& name) { m_memberName = name; }
