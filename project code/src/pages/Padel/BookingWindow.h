// BookingWindow.h
#ifndef BOOKINGWINDOW_H
#define BOOKINGWINDOW_H

#include <QDateTime>
#include <vector>
#include <QString>
#include <QDebug>
#include "../Model/Padel/Court.h"
#include "../Model/Padel/Booking.h"
#include <iostream>
using namespace std;

class BookingWindow {
public:
    BookingWindow();

    bool isBooked(const Court& court, const QDate& date, const QTime& time, vector<Booking>& bookings);
    vector<Court> searchAvailableCourts(const QDate& date, const QTime& time, const QString& location, 
        vector<Court>& courts, vector<Booking>& bookings);
    vector<QDateTime> suggestNextSlots(const Court& court, const QDate& date, const QTime& fromTime, vector<Booking>& bookings);
    void cancelBooking(int bookingId, vector<Booking>& bookings);
    void rescheduleBooking(int bookingId, const QDateTime& startTime, const QDateTime& endTime, vector<Booking>& bookings);
    bool canCancelOrReschedule(const Booking& booking);

private:
    void showAvailableCourts(vector<Court>& courts, const QDate& date, QTime& time, vector<Booking>& bookings);
    void showAvailableTimeSlots(const Court& court, const QDateTime& baseTime, vector<Booking>& bookings);
    void showSuggestions(vector<QDateTime>& suggestions);
};

#endif // BOOKINGWINDOW_H
