#include "BookingWindow.h"
#include <iostream>
using namespace std;
#include <vector>

bool BookingWindow::canCancelOrReschedule(const Booking& booking) {
    QDateTime now = QDateTime::currentDateTime();
    QDateTime bookingTime(booking.getStartTime());
    return now.secsTo(bookingTime) > 3 * 60; // 3 min as 3 hours

}
 void BookingWindow::cancelBooking(int bookingId ,vector<Booking>& bookings) {
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


 void BookingWindow::rescheduleBooking(int bookingId, const QDateTime& newTime , vector<Booking>& bookings) {
     for (Booking& booking : bookings) {
         if (booking.getBookingId() == bookingId) {
            
             if (canCancelOrReschedule(booking)) {
                 qDebug() << "Cannot reschedule: Less than 3 hours before booking time.";
                 return;
             }

             // Check availability using a global CourtManager or similar
         /*    if (!isCourtAvailable(booking.getCourt().getId(), newTime)) {
                 qDebug() << "Court not available at new requested time.";
                 return;
             }*/

             booking.setStartTime(newTime);
             qDebug() << "Booking rescheduled successfully.";
             return;
         }
     }

     qDebug() << "Booking ID not found.";
 }

