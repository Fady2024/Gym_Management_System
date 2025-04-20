// BookingWindow.h
#ifndef BOOKINGWINDOW_H
#define BOOKINGWINDOW_H

#include <QDateTime>
#include <vector>
#include "../Model/Padel/Court.h"
#include "../Model/Padel/Booking.h"
#include<iostream>
using namespace std;


class BookingWindow {
public:
    BookingWindow();

    //void searchAvailableCourts(const QDateTime& dateTime, const QString& location);
    //void selectCourt(int index);
    void cancelBooking(int bookingId, vector<Booking>& bookings);
    void rescheduleBooking(int bookingId, const QDateTime& newTime, vector<Booking>& bookings);
    bool canCancelOrReschedule(const Booking& booking);

private:
 
   

   // void showAvailableCourts(const std::vector<Court>& courts);
    //void showAvailableTimeSlots(const Court& court, const QDateTime& baseTime);
    //void showSuggestions(const std::vector<QDateTime>& suggestions);
};

#endif // BOOKINGWINDOW_H
