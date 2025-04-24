#ifndef PADELDATAMANAGER_H
#define PADELDATAMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonParseError>
#include <QMutex>
#include <QDateTime>
#include <QVector>
#include <unordered_map>
#include <QDir>
#include "../Model/Padel/Court.h"
#include "../Model/Padel/Booking.h"
#include "../Model/member.h"
#include "../DataManager/memberdatamanager.h"
#include <QString>
#include <QVector>
#include <unordered_map>
#include <QDir>
#include <QTimer>
#include <queue>
#include <set>

// Structure to store waitlist entries
struct WaitlistEntry {
    int memberId;
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
    Court getCourtById(int courtId) const;
    QVector<Court> getAllCourts() const;
    QVector<Court> getCourtsByLocation(const QString& location) const;

    // Booking management
    bool addBooking(const Booking& booking, QString& errorMessage);
    bool updateBooking(const Booking& booking, QString& errorMessage);
    bool deleteBooking(int bookingId, QString& errorMessage);
    bool createBooking(int userId, int courtId, const QDateTime& startTime, 
                      const QDateTime& endTime, QString& errorMessage);
    bool cancelBooking(int bookingId, QString& errorMessage);
    bool rescheduleBooking(int bookingId, const QDateTime& newStartTime, 
                          const QDateTime& newEndTime, QString& errorMessage);
    Booking getBookingById(int bookingId) const;
    QVector<Booking> getAllBookings() const;
    QVector<Booking> getBookingsByMember(int memberId) const;
    QVector<Booking> getBookingsByCourt(int courtId) const;
    QVector<Booking> getBookingsByDate(const QDate& date) const;
    bool isCourtAvailable(int courtId, const QDateTime& startTime, 
                         const QDateTime& endTime) const;

    // Availability and search
    bool isBooked(int courtId, const QDateTime& time) const;
    QVector<Court> searchAvailableCourts(const QDateTime& time, const QString& location) const;
    QVector<QDateTime> suggestNextSlots(int courtId, const QDateTime& fromTime) const;
    bool canCancelOrReschedule(int bookingId) const;

    // VIP management
    bool isVIPBooking(int bookingId) const;
    double calculateBookingPrice(int courtId, const QDateTime& startTime, 
                               const QDateTime& endTime, bool isVIP) const;
    void setVIPPriority(int memberId, bool isVIP);
    bool isVIPMember(int memberId) const;
    int calculatePriority(int memberId) const;

    // Waitlist management
    bool addToWaitlist(int memberId, int courtId, const QDateTime& requestedTime, 
                      QString& errorMessage);
    bool removeFromWaitlist(int memberId, int courtId, QString& errorMessage);
    QVector<WaitlistEntry> getWaitlistForCourt(int courtId) const;
    bool processWaitlist(int courtId, QString& errorMessage);

    // Time slot management
    bool addTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage);
    bool removeTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage);
    QVector<QTime> getAvailableTimeSlots(int courtId, const QDate& date) const;

    // Reporting
    QJsonObject generateMonthlyReport(const QDate& month) const;
    QJsonObject generateCourtUtilizationReport(int courtId, const QDate& startDate, 
                                             const QDate& endDate) const;

    // Dependency injection
    void setMemberDataManager(MemberDataManager* memberManager);

signals:
    void bookingCreated(int bookingId, int userId);
    void bookingUpdated(int bookingId);
    void bookingCancelled(int bookingId, int userId);
    void bookingRescheduled(int bookingId, int userId);
    void courtAdded(int courtId);
    void courtUpdated(int courtId);
    void courtDeleted(int courtId);
    void waitlistUpdated(int courtId);
    void courtAvailabilityChanged(int courtId);
    void vipStatusChanged(int memberId, bool isVIP);

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
    bool writeCourtsToFile(const QJsonArray& courts, QString& errorMessage) const;
    bool writeBookingsToFile(const QJsonArray& bookings, QString& errorMessage) const;
    
    // JSON conversion
    QJsonObject courtToJson(const Court& court) const;
    QJsonObject bookingToJson(const Booking& booking) const;
    static Court jsonToCourt(const QJsonObject& json);
    static Booking jsonToBooking(const QJsonObject& json);
    
    // Helper methods
    [[nodiscard]] int generateCourtId() const;
    [[nodiscard]] int generateBookingId() const;
    void checkBookingStatus();
    void setupTimers();
    bool validateBookingTime(const QDateTime& startTime, const QDateTime& endTime, 
                           QString& errorMessage) const;
    bool validateCourtAvailability(int courtId, const QDateTime& startTime, 
                                 const QDateTime& endTime) const;
};

#endif // PADELDATAMANAGER_H 