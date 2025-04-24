#include "Booking.h"
#include "../user.h"
#include "Court.h"

Booking::Booking()
    : m_bookingId(0)
    , m_price(0.0)
    , m_isVip(false)
    , m_isCancelled(false)
{
}

Booking::Booking(int bookingId, const Court& court, const QDateTime& startTime,
                 const QDateTime& endTime, const User& user)
    : m_bookingId(bookingId)
    , m_court(court)
    , m_user(user)
    , m_startTime(startTime)
    , m_endTime(endTime)
    , m_price(0.0)
    , m_isVip(false)
    , m_isCancelled(false)
{
}

Booking::Booking(const Booking& other)
    : m_bookingId(other.m_bookingId)
    , m_court(other.m_court)
    , m_user(other.m_user)
    , m_startTime(other.m_startTime)
    , m_endTime(other.m_endTime)
    , m_price(other.m_price)
    , m_isVip(other.m_isVip)
    , m_isCancelled(other.m_isCancelled)
{
}

Booking& Booking::operator=(const Booking& other) {
    if (this != &other) {
        m_bookingId = other.m_bookingId;
        m_court = other.m_court;
        m_user = other.m_user;
        m_startTime = other.m_startTime;
        m_endTime = other.m_endTime;
        m_price = other.m_price;
        m_isVip = other.m_isVip;
        m_isCancelled = other.m_isCancelled;
    }
    return *this;
}

// Getters
int Booking::getBookingId() const {
    return m_bookingId;
}

const Court& Booking::getCourt() const {
    return m_court;
}

const User& Booking::getUser() const {
    return m_user;
}

int Booking::getCourtId() const {
    return m_court.getId();
}

int Booking::getUserId() const {
    return m_user.getId();
}

int Booking::getMemberId() const {
    return m_user.getId();
}

const QDateTime& Booking::getStartTime() const {
    return m_startTime;
}

const QDateTime& Booking::getEndTime() const {
    return m_endTime;
}

double Booking::getPrice() const {
    return m_price;
}

bool Booking::isVip() const {
    return m_isVip;
}

bool Booking::isCancelled() const {
    return m_isCancelled;
}

// Setters
void Booking::setBookingId(int id) {
    m_bookingId = id;
}

void Booking::setCourt(const Court& court) {
    m_court = court;
}

void Booking::setUser(const User& user) {
    m_user = user;
}

void Booking::setCourtId(int id) {
    m_court.setId(id);
}

void Booking::setUserId(int id) {
    m_user.setId(id);
}

void Booking::setStartTime(const QDateTime& time) {
    m_startTime = time;
}

void Booking::setEndTime(const QDateTime& time) {
    m_endTime = time;
}

void Booking::setPrice(double price) {
    m_price = price;
}

void Booking::setVip(bool vip) {
    m_isVip = vip;
}

void Booking::setCancelled(bool cancelled) {
    m_isCancelled = cancelled;
}

// Booking operations
void Booking::cancel() {
    m_isCancelled = true;
}

void Booking::reschedule(const QDateTime& newStartTime, const QDateTime& newEndTime) {
    m_startTime = newStartTime;
    m_endTime = newEndTime;
}

