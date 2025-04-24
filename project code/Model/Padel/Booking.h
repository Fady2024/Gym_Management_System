#ifndef BOOKING_H
#define BOOKING_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include "../user.h"
#include "Court.h"

class Booking {
public:
    // Constructors
    Booking();
    Booking(int bookingId, const Court& court, const QDateTime& startTime, 
            const QDateTime& endTime, const User& user);
    Booking(const Booking& other);
    Booking& operator=(const Booking& other);

    // Getters
    int getBookingId() const;
    const Court& getCourt() const;
    const User& getUser() const;
    const QDateTime& getStartTime() const;
    const QDateTime& getEndTime() const;
    double getPrice() const;
    bool isVip() const;
    bool isCancelled() const;
    int getMemberId() const;
    int getCourtId() const;
    int getUserId() const;

    // Setters
    void setBookingId(int id);
    void setCourt(const Court& court);
    void setUser(const User& user);
    void setStartTime(const QDateTime& time);
    void setEndTime(const QDateTime& time);
    void setPrice(double price);
    void setVip(bool vip);
    void setCancelled(bool cancelled);
    void setCourtId(int id);
    void setUserId(int id);

    // Booking operations
    void cancel();
    void reschedule(const QDateTime& newStartTime, const QDateTime& newEndTime);

private:
    int m_bookingId;
    Court m_court;
    User m_user;
    QDateTime m_startTime;
    QDateTime m_endTime;
    double m_price;
    bool m_isVip;
    bool m_isCancelled;
};

#endif // BOOKING_H
