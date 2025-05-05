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
#include <QCheckBox>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QTimer>
#include <QApplication>
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
    void bookCourtDirectly(int courtId, const QDateTime& startTime, const QDateTime& endTime);
    void cancelBooking();
    void rescheduleBooking();
    void refreshBookingsList();
    void onBookingItemSelected(int row);
    void performSearch();
    void clearSearch();
    void onJoinWaitlistClicked();
    void onCheckWaitlistClicked();
    void onBookButtonClicked(int courtId, const QDate& date, const QTime& startTime, const QTime& endTime);

private:
    PadelDataManager* m_padelManager;
    UserDataManager* m_userDataManager = nullptr;
    MemberDataManager* m_memberDataManager = nullptr;
    bool m_isDarkTheme = false;
    int m_currentUserId = 0;
    QString m_currentUserEmail;
    int m_selectedBookingId = -1;
    QTime m_selectedWaitlistTime;
    
    // UI components
    QComboBox* m_courtSelector;
    QDateEdit* m_dateSelector;
    QComboBox* m_timeSlotSelector;
    QPushButton* m_bookButton;
    QGridLayout* m_timeSlotGrid;
    QGridLayout* m_slotsLayout;
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
    
    // Search and Filter UI
    QLineEdit* m_searchBox;
    QPushButton* m_searchButton;
    QPushButton* m_clearSearchButton;
    QComboBox* m_locationFilter;
    QCheckBox* m_showAvailableOnly;
    QLabel* m_totalResultsLabel;
    
    // Waitlist UI
    QPushButton* m_joinWaitlistButton;
    QPushButton* m_checkWaitlistButton;
    QLabel* m_waitlistStatusLabel;
    
    // Court info UI (renamed from Capacity info UI)
    QLabel* m_capacityLabel;

    // Add m_noSlotsLabel declaration
    QLabel* m_noSlotsLabel = nullptr;
    
    // Helper methods
    void setupUI();
    void setupSearchUI(QWidget* parent, QVBoxLayout* parentLayout);
    QLayout* setupWaitlistUI(QWidget* parent);
    void setupConnections();
    void loadCourts();
    void updateCourtDetails(int courtId);
    bool canCancelOrReschedule(const Booking& booking);
    void showAvailableTimeSlots(int courtId, const QDate& date);
    QTime getEndTime(const QTime& startTime);
    void loadUserData();
    QFrame* createSeparator();
    void clearTimeSlotGrid();
    QWidget* createTimeSlotWidget(const QString& startTimeStr, const QString& endTimeStr, 
                                 int currentAttendees, int maxAttendees);
    void bookTimeSlot(int courtId, const QDate& date, const QString& startTimeStr, const QString& endTimeStr);
    void createTimeSlotWidgets(const QVector<QTime>& timeSlots, 
                           const QVector<Booking>& bookings,
                           int courtId, 
                           const QDate& date,
                           int maxAttendees,
                           int rows,
                           int columns,
                           bool userHasBooking);
    int countCurrentBookings(const QVector<Booking>& bookings, 
                         int courtId, 
                         const QDateTime& startTime, 
                         const QDateTime& endTime);
    QWidget* createSingleTimeSlotWidget(const QTime& timeSlot, 
                                   int currentBookings, 
                                   int maxAttendees, 
                                   int courtId, 
                                   const QDate& date,
                                   bool userHasBooking);
    
    // Search and filter helpers
    void loadCourtsByFilter(const QString& nameQuery, const QString& location, bool availableOnly);
    void updateLocationFilter();
    
    // Waitlist helpers
    void joinWaitlist(int courtId);
    void updateWaitlistStatus(int courtId);
    int getMyWaitlistPosition(int courtId);
};

#endif // BOOKINGWINDOW_H
