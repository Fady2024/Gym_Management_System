// BookingWindow.h
#ifndef BOOKINGWINDOW_H
#define BOOKINGWINDOW_H

#include <QWidget>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QMessageBox>
#include <QListWidget>
#include <QLineEdit>
#include <QFrame>
#include "../Model/Padel/Court.h"
#include "../Model/Padel/Booking.h"
#include "../../DataManager/padeldatamanager.h"
#include "../../DataManager/userdatamanager.h"
#include "../../DataManager/memberdatamanager.h"

class BookingWindow : public QWidget {
    Q_OBJECT

public:
    BookingWindow(PadelDataManager* padelManager, QWidget *parent = nullptr);
    
    void retranslateUI();
    void updateTheme(bool isDark);
    void setCurrentUserEmail(const QString& email);
    void setUserDataManager(UserDataManager* manager) { m_userDataManager = manager; }
    void setMemberDataManager(MemberDataManager* manager) { m_memberDataManager = manager; }

private slots:
    void onCourtSelectionChanged(int index);
    void onDateChanged(const QDate& date);
    void refreshTimeSlots();
    void bookCourt();
    void cancelBooking();
    void rescheduleBooking();
    void refreshBookingsList();
    void onBookingItemSelected(int row);

private:
    PadelDataManager* m_padelManager;
    UserDataManager* m_userDataManager = nullptr;
    MemberDataManager* m_memberDataManager = nullptr;
    bool m_isDarkTheme = false;
    int m_currentUserId = 0;
    QString m_currentUserEmail;
    int m_selectedBookingId = -1;
    
    // UI components
    QComboBox* m_courtSelector;
    QDateEdit* m_dateSelector;
    QComboBox* m_timeSlotSelector;
    QPushButton* m_bookButton;
    QListWidget* m_bookingsList;
    QPushButton* m_cancelButton;
    QPushButton* m_rescheduleButton;
    QComboBox* m_rescheduleTimeSelector;
    QLabel* m_courtNameLabel;
    QLabel* m_courtLocationLabel;
    QLabel* m_courtPriceLabel;
    QLabel* m_courtDescriptionLabel;
    QListWidget* m_courtFeaturesListWidget;
    QLabel* m_statusLabel;
    QLabel* m_userInfoLabel;

    // Helper methods
    void setupUI();
    void setupConnections();
    void loadCourts();
    void updateCourtDetails(int courtId);
    bool canCancelOrReschedule(const Booking& booking);
    void showAvailableTimeSlots(int courtId, const QDate& date);
    QTime getEndTime(const QTime& startTime);
    void loadUserData();
    QFrame* createSeparator();
};

#endif // BOOKINGWINDOW_H
