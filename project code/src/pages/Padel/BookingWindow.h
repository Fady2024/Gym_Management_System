// BookingWindow.h
#ifndef BOOKINGWINDOW_H
#define BOOKINGWINDOW_H

#include <QDateTime>
#include <vector>
#include <QString>
#include <QDebug>
#include "../Model/Padel/Court.h"
#include "../Model/Padel/Booking.h"
#include "../../DataManager/padeldatamanager.h"
#include <iostream>
using namespace std;

class BookingWindow {
public:
    BookingWindow(PadelDataManager* padelManager);

    // UI display functions
    void showAvailableCourts(const QDate& date, const QTime& time);
    void showAvailableTimeSlots(int courtId, const QDateTime& baseTime);
    void showSuggestions(const QVector<QDateTime>& suggestions);

private:
    PadelDataManager* m_padelManager;
};

#endif // BOOKINGWINDOW_H
