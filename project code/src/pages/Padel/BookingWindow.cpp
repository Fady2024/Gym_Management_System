#include "BookingWindow.h"
#include <iostream>
#include <QDebug>
using namespace std;
#include <vector>

bool BookingWindow::canCancelOrReschedule(const Booking& booking) {
    QDateTime now = QDateTime::currentDateTime();
    QDateTime bookingTime(booking.getStartTime());
    return now.secsTo(bookingTime) > 3 * 60; // 3 min as 3 hours
}

bool BookingWindow::isBooked(const Court& court, const QDate& date, const QTime& time, vector<Booking>& bookings) {
    for (int i = 0; i < bookings.size(); i++) {
        if (bookings[i].getCourt().getId() == court.getId() &&
            bookings[i].getStartTime().date() == date &&
            bookings[i].getStartTime().time() == time) {
            return true;
        }
    }
    return false;
}

vector<Court> BookingWindow::searchAvailableCourts(
    const QDate& date,
    const QTime& time,
    const QString& location,
    vector<Court>& courts,
    vector<Booking>& bookings) {

    vector<Court> available;

    for (int i = 0; i < courts.size(); i++) {
        if (courts[i].getLocation() == location) {
            //Check if the time is available in this court
            bool timeExists = false;
            for (int s = 0; s < courts[i].getAllTimeSlots().size(); s++) {
                if (courts[i].getAllTimeSlots()[s] == time) {
                    timeExists = true;
                    break;
                }
            }

            if (timeExists && !isBooked(courts[i], date, time, bookings)) {
                available.push_back(courts[i]);
            }
        }
    }

    return available;
}

void BookingWindow::showAvailableCourts(vector<Court>& courts, const QDate& date, QTime& time, vector<Booking>& bookings) {
    qDebug() << "Available Courts:";

    for (const Court& court : courts) {
        if (!isBooked(court, date, time, bookings)) {
            qDebug() << court.getName() << "-" << court.getLocation();
        }
    }
}

void BookingWindow::showAvailableTimeSlots(const Court& court, const QDateTime& baseTime, vector<Booking>& bookings) {
    QDate date = baseTime.date();
    qDebug() << "Available Time Slots for Court:" << court.getName() << "on" << date.toString("dd-MM-yyyy");

    const vector<QTime>& timeSlots = court.getAllTimeSlots();
    for (const QTime& timeSlot : timeSlots) {
        if (!isBooked(court, date, timeSlot, bookings)) {
            qDebug() << "•" << timeSlot.toString("hh:mm");
        }
    }
}

void BookingWindow::cancelBooking(int bookingId, vector<Booking>& bookings) {
    for (int i = 0; i < bookings.size(); ++i) {
        if (bookings[i].getBookingId() == bookingId) {
            if (canCancelOrReschedule(bookings[i])) {
                qDebug() << "Cannot cancel: Less than 3 hours before booking time.";
                return;
            }

            bookings.erase(bookings.begin() + i);
            qDebug() << "Booking canceled successfully.";
            return;
        }
    }

    qDebug() << "Booking ID not found.";
}

void BookingWindow::rescheduleBooking(int bookingId, const QDateTime& startTime, const QDateTime& endtime, vector<Booking>& bookings) {
    for (Booking& booking : bookings) {
        if (booking.getBookingId() == bookingId) {
            if (canCancelOrReschedule(booking)) {
                qDebug() << "Cannot reschedule: Less than 3 hours before booking time.";
                return;
            }

            booking.setStartTime(startTime);
            booking.setEndTime(endtime);
            qDebug() << "Booking rescheduled successfully.";
            return;
        }
    }

    qDebug() << "Booking ID not found.";
}

