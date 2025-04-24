#include "BookingWindow.h"
#include <QDebug>

BookingWindow::BookingWindow(PadelDataManager* padelManager)
    : m_padelManager(padelManager)
{
}

void BookingWindow::showAvailableCourts(const QDate& date, const QTime& time)
{
    QVector<Court> availableCourts = m_padelManager->searchAvailableCourts(date, time);
    
    qDebug() << "Available Courts for" << date.toString("yyyy-MM-dd") << "at" << time.toString("HH:mm");
    for (const Court& court : availableCourts) {
        qDebug() << "Court ID:" << court.getId()
                 << "Location:" << court.getLocation()
                 << "Type:" << court.getType()
                 << "Price:" << court.getPrice();
    }
}

void BookingWindow::showAvailableTimeSlots(int courtId, const QDateTime& baseTime)
{
    QVector<QDateTime> availableSlots = m_padelManager->getAvailableTimeSlots(courtId, baseTime);
    
    qDebug() << "Available Time Slots for Court" << courtId;
    for (const QDateTime& slot : availableSlots) {
        qDebug() << slot.toString("yyyy-MM-dd HH:mm");
    }
}

void BookingWindow::showSuggestions(const QVector<QDateTime>& suggestions)
{
    qDebug() << "Suggested Time Slots:";
    for (const QDateTime& suggestion : suggestions) {
        qDebug() << suggestion.toString("yyyy-MM-dd HH:mm");
    }
}

