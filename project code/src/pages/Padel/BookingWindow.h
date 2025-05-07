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
    // void bookCourt();
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
    void onAlternativeCourtButtonClicked(int courtId, const QString& timeSlot);

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
    QPushButton * m_prevWeekButton;
    QPushButton * m_nextWeekButton;
    QLabel * m_weekRangeLabel;

    //QPushButton* m_bookButton;

    QLabel* m_dayHeaders[7];
    QGridLayout* m_calendarGrid;
    QWidget* m_calendarCells[7][15];
    QListWidget* m_bookingsList;
    QPushButton* m_cancelButton;
    QPushButton* m_rescheduleButton;
    QComboBox* m_rescheduleTimeSelector;
    // QLabel* m_courtNameLabel;
    // QLabel* m_courtLocationLabel;
    // QLabel* m_courtPriceLabel;
    // QLabel* m_courtDescriptionLabel;
    // QListWidget* m_courtFeaturesListWidget;
    QLabel* m_statusLabel;
    QLabel* m_userInfoLabel;

    // Alternative court suggestions
    QWidget* m_alternativeCourtsWidget;
    QVBoxLayout* m_alternativeCourtsLayout;
    QLabel* m_alternativesLabel;
    QGridLayout* m_alternativeCourtsList;

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
    // QLabel* m_capacityLabel;

    // Helper methods
    void setupUI();
    void setupSearchUI(QWidget* parent, QVBoxLayout* parentLayout);
    QLayout* setupWaitlistUI(QWidget* parent);
    QWidget* setupAlternativeCourtsUI(QWidget* parent);
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
    void showPreviousWeek();
    void showNextWeek();
    void updateWeekRangeLabel();

    // Search and filter helpers
    void loadCourtsByFilter(const QString& nameQuery, const QString& location, bool availableOnly);
    void updateLocationFilter();

    // Waitlist helpers
    void joinWaitlist(int courtId);
    void updateWaitlistStatus(int courtId);
    int getMyWaitlistPosition(int courtId);

    // Alternative courts helpers
    void updateAlternativeCourts();
};

class CalendarButton : public QPushButton {
    Q_OBJECT
public:
    CalendarButton(QWidget* parent = nullptr) : QPushButton(parent) {
        setFlat(true);
        setStyleSheet(normalStyle());
        setCursor(Qt::PointingHandCursor);
    }

    void setSlotInfo(const QString& time, const QString& availability, bool isAvailable) {
        m_time = time;
        m_availability = availability;
        m_isAvailable = isAvailable;
        updateDisplay(false);
    }
    QString normalStyle() const {
        bool isSelectedDay = property("isSelectedDay").toBool();

        return QString(
            "QPushButton {"
            "  border: 1px solid %1;"
            "  padding: 5px;"
            "  text-align: center;"
            "  background-color: %2;"
            "  color: %3;"
            "}"
        ).arg(isSelectedDay ? "#7C3AED" : "#D1D5DB")  // Border color
         .arg(m_isAvailable ? (isSelectedDay ? "#EDE9FE" : "#F0FDF4")  // Background
                           : (isSelectedDay ? "#F3E8FF" : "#FEF2F2"))
         .arg(m_isAvailable ? (isSelectedDay ? "#5B21B6" : "#065F46")  // Text color
                           : (isSelectedDay ? "#86198F" : "#991B1B"));
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        updateDisplay(true);
        QPushButton::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override {
        updateDisplay(false);
        QPushButton::leaveEvent(event);
    }

private:
    QString m_time;
    QString m_availability;
    bool m_isAvailable;

    void updateDisplay(bool hovered) {
        if (hovered && m_isAvailable) {
            setText("Book Court");
            setStyleSheet(hoverStyle());
        } else {
            setText(QString("%1\n%2").arg(m_time).arg(m_availability));
            setStyleSheet(normalStyle());
        }
    }


    QString hoverStyle() const {
        return QString(
            "QPushButton {"
            "  border: 1px solid #4F46E5;"
            "  padding: 5px;"
            "  text-align: center;"
            "  background-color: #4F46E5;"
            "  color: white;"
            "  font-weight: bold;"
            "}"
        );
    }
};
#endif // BOOKINGWINDOW_H
