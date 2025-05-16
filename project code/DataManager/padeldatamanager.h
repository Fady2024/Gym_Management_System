#ifndef PADELDATAMANAGER_H
#define PADELDATAMANAGER_H

#include <unordered_map>
#include "../Model/Padel/Court.h"
#include "../Model/Padel/Booking.h"
#include "../DataManager/memberdatamanager.h"
#include <QString>
#include <QVector>
#include <QTimer>
#include <queue>

// Structure to store waitlist entries
struct WaitlistEntry {
    int memberId;
    int courtId;
    QDateTime requestedTime;
    bool isVIP;
    int priority;
};

class PadelDataManager : public QObject {
    Q_OBJECT

public:
    explicit PadelDataManager(QObject* parent = nullptr);
    ~PadelDataManager() override;

    bool initializeFromFile();
    bool saveToFile();
    void handleApplicationClosing();

    // Court management
    bool addCourt(const Court& court, QString& errorMessage);
    bool updateCourt(const Court& court, QString& errorMessage);
    bool deleteCourt(int courtId, QString& errorMessage);
    int getBookedCourtsCount() const;
    Court getCourtById(int courtId) const;
    QVector<Court> getAllCourts() const;
    QVector<Court> getCourtsByLocation(const QString& location) const;

    QVector<Court> getAvailableCourts(const QDateTime& startTime, const QDateTime& endTime, 
                                    const QString& location = QString()) const;
    int getCurrentAttendees(int courtId, const QDateTime& startTime, const QDateTime& endTime) const;

    Court findClosestAvailableCourt(int originalCourtId, const QDateTime& startTime, const QDateTime& endTime) const;

    // Booking management
    bool deleteBooking(int bookingId, QString& errorMessage);
    bool createBooking(int userId, int courtId, const QDateTime& startTime, 
                     const QDateTime& endTime, QString& errorMessage, bool isFromWaitlist = false);
    bool cancelBooking(int bookingId, QString& errorMessage);
    bool rescheduleBooking(int bookingId, const QDateTime& newStartTime, 
                         const QDateTime& newEndTime, QString& errorMessage);
    QVector<Booking> getAllBookings() const;
    QVector<Booking> getBookingsByMember(int memberId) const;
    QVector<Booking> getBookingsByCourt(int courtId) const;
    QVector<Booking> getBookingsByDate(const QDate& date) const;
    QVector<Booking> getBookingsForTimeSlot(int courtId, const QDateTime& startTime, const QDateTime& endTime);
    QVector<Booking> getUserAutoBookings(int userId) const;
    bool isCourtAvailable(int courtId, const QDateTime& startTime, 
                         const QDateTime& endTime) const;

    // VIP management
    double calculateBookingPrice(int courtId, const QDateTime& startTime,
                               const QDateTime& endTime, bool isVIP) const;
    void setVIPPriority(int memberId, bool isVIP);
    bool isVIPMember(int memberId) const;
    int calculatePriority(int memberId) const;

    // Waitlist management
    bool addToWaitlist(int userId, int courtId, const QDateTime& requestedTime,
                   QString& errorMessage);
    bool removeFromWaitlist(int userId, int courtId, QString& errorMessage);
    QVector<WaitlistEntry> getWaitlistForCourt(int courtId) const;
    QVector<WaitlistEntry> getWaitlistForMember(int memberId) const;
    QVector<WaitlistEntry> getWaitlistForUser(int userId) const;
    int getWaitlistPosition(int userId, int courtId) const;
    bool processWaitlist(int courtId, QString& errorMessage);
    bool tryFillSlotFromWaitlist(int courtId, const QDateTime& startTime, const QDateTime& endTime, QString& errorMessage);
    bool isUserInWaitlist(int userId, int courtId, const QDateTime& requestedTime) const;
    QJsonObject getDetailedWaitlistInfo(int courtId, const QDate& date) const;
    void checkBookingStatus();
    void processWaitlistForDate(int courtId, const QDate& date);

    // Time slot management
    bool addTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage);
    bool removeTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage);
    QJsonArray getAvailableTimeSlots(int courtId, const QDate& date, int maxAttendees) const;
    QVector<QTime> getAllTimeSlots(int courtId) const;
    QJsonArray getAllTimeSlotsJson(int courtId) const;

    // Reporting
    QJsonObject generateMonthlyReport(const QDate& month) const;
    QJsonObject generateCourtUtilizationReport(int courtId, const QDate& startDate, 
                                             const QDate& endDate) const;

    // Dependency injection
    void setMemberDataManager(MemberDataManager* memberManager);

    bool userHasBookingAtTime(int userId, int courtId, const QDate& date, const QTime& timeSlot) const;
    bool userHasBookingOnDate(int userId, int courtId, const QDate& date) const;
    QJsonObject getCourtDetails(int courtId) const;

signals:
    void courtAdded(int courtId);
    void courtUpdated(int courtId);
    void courtDeleted(int courtId);
    void bookingCreated(int bookingId, int userId);
    void bookingCancelled(int bookingId, int userId);
    void bookingRescheduled(int bookingId, int userId);
    void waitlistUpdated(int courtId);
    void waitlistPositionChanged(int userId, int courtId, int position);
    void vipStatusChanged(int memberId, bool isVIP);
    void courtAvailabilityChanged(int courtId);
    void waitlistBookingCreated(int userId, int courtId, const QDateTime& startTime);

private slots:
    void safeEmitBookingCreated(int bookingId, int userId);
    void safeEmitBookingCancelled(int bookingId, int userId);

private:
    QString dataDir;
    mutable QMutex mutex;
    std::unordered_map<int, Court> courtsById;
    std::unordered_map<int, Booking> bookingsById;
    std::unordered_map<int, std::queue<WaitlistEntry>> courtWaitlists;
    std::unordered_map<int, bool> vipMembers;
    bool dataModified = false;
    MemberDataManager* memberDataManager;

    // File operations
    QJsonArray readCourtsFromFile(QString& errorMessage) const;
    QJsonArray readBookingsFromFile(QString& errorMessage) const;
    QJsonArray readWaitlistsFromFile(QString& errorMessage) const;
    bool writeBookingsToFile(const QJsonArray& bookings, QString& errorMessage) const;
    bool writeCourtsToFile(const QJsonArray& courts, QString& errorMessage) const;
    bool writeWaitlistsToFile(const QJsonArray& waitlists, QString& errorMessage) const;
    QJsonArray waitlistsToJson() const;
    
    // JSON conversion
    QJsonObject courtToJson(const Court& court) const;
    QJsonObject bookingToJson(const Booking& booking) const;
    static Court jsonToCourt(const QJsonObject& json);
    static Booking jsonToBooking(const QJsonObject& json);
    
    // Helper methods
    [[nodiscard]] int generateCourtId() const;
    [[nodiscard]] int generateBookingId() const;
    void setupTimers();
    bool validateBookingTime(const QDateTime& startTime, const QDateTime& endTime, 
                           QString& errorMessage) const;
    bool validateCourtAvailability(int courtId, const QDateTime& startTime, 
                                 const QDateTime& endTime) const;
    void updateWaitlistPositionsAndNotify(int courtId);
    void removeUserFromAllWaitlists(int userId, int courtId, const QDate& date);
    int countDailyBookings(int courtId, const QDate& date) const;
};

#endif // PADELDATAMANAGER_H 