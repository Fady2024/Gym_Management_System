#ifndef MEMBERDATAMANAGER_H
#define MEMBERDATAMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonParseError>
#include <QMutex>
#include "../Model/member.h"
#include "../Model/subscription.h"
#include "../DataManager/userdatamanager.h"
#include <QString>
#include <QVector>
#include <unordered_map>
#include <QDir>
#include <QTimer>
#include <QCryptographicHash>

// Structure to store card data with basic encryption
struct SavedCardData {
    QString fullCardNumber;  // Store the complete card number (Note: This is not secure for production)
    QString expiryDate;
    QString cardholderName;
    int memberId;
};

struct RenewalOffer {
    int memberId;
    double originalPrice;
    double discountedPrice;
    int daysUntilExpiry;
    bool isVIP;
};

class MemberDataManager : public QObject {
    Q_OBJECT

public:
    explicit MemberDataManager(QObject* parent = nullptr);
    ~MemberDataManager() override;

    bool initializeFromFile();
    bool saveToFile();
    void handleApplicationClosing();

    // Member management
    bool addMember(const Member& member, QString& errorMessage);
    bool updateMember(const Member& member, QString& errorMessage);
    bool deleteMember(int memberId, QString& errorMessage);
    Member getMemberById(int memberId) const;
    QVector<Member> getAllMembers() const;
    
    // User-to-Member conversion
    bool createMemberFromUser(int userId, QString& errorMessage);
    bool createMemberFromUser(const User& user, QString& errorMessage);
    bool userIsMember(int userId) const;
    int getMemberIdByUserId(int userId) const;
    Member getMemberByUserId(int userId) const;

    // Subscription management
    bool addSubscription(int memberId, const Subscription& subscription, QString& errorMessage);
    bool renewSubscription(int memberId, SubscriptionType newType, bool isVIP, QString& errorMessage);
    bool cancelSubscription(int memberId, QString& errorMessage);
    QVector<Member> getMembersNeedingRenewal(int daysThreshold = 7) const;
    bool isSubscriptionActive(int memberId) const;
    bool isVIPMember(int memberId) const;

    // Early renewal and discounts
    RenewalOffer getRenewalOffer(int memberId) const;
    QVector<RenewalOffer> getAllRenewalOffers() const;
    double calculateRenewalDiscount(int memberId, SubscriptionType newType) const;
    bool isEligibleForEarlyRenewal(int memberId) const;

    // Workout history
    bool addWorkoutToHistory(int memberId, const QString& workoutType, const QDate& date, QString& errorMessage);
    QVector<QPair<int, QDate>> getWorkoutHistory(int memberId) const;
    QVector<QPair<int, QDate>> getRecentWorkouts(int memberId, int count = 5) const;
    
    // Payment card management
    bool saveCardData(int memberId, const QString& cardNumber, const QString& expiryDate, 
                      const QString& cardholderName, QString& errorMessage);
    bool hasStoredCard(int memberId) const;
    SavedCardData getStoredCard(int memberId) const;
    
    // Payment data storage
    bool savePaymentData(int memberId, int planId, bool isVIP, double amount, 
                         const QString& cardNumber, const QString& expiryDate, const QString& cardholderName,
                         QString& errorMessage);

    // Dependency injection
    void setUserDataManager(UserDataManager* userManager);

signals:
    void subscriptionNearingExpiry(int memberId, int daysLeft);
    void subscriptionExpired(int memberId);
    void vipStatusChanged(int memberId, bool isVIP);
    void earlyRenewalOfferAvailable(const RenewalOffer& offer);
    void renewalDiscountExpiring(int memberId, int daysLeft);
    void memberCreated(int memberId, int userId);
    void memberUpdated(int memberId);

private:
    QString dataDir;
    mutable QMutex mutex;
    std::unordered_map<int, Member> membersById;
    std::unordered_map<int, int> userIdToMemberId; // Maps user IDs to member IDs
    std::unordered_map<int, SavedCardData> savedCards;
    bool dataModified = false;
    QTimer* renewalCheckTimer;
    UserDataManager* userDataManager;

    QJsonArray readMembersFromFile(QString& errorMessage) const;
    bool writeMembersToFile(const QJsonArray& members, QString& errorMessage) const;
    QJsonObject memberToJson(const Member& member) const;
    static Member jsonToMember(const QJsonObject& json);
    [[nodiscard]] int generateMemberId() const;
    void checkSubscriptionStatus();
    void checkEarlyRenewalOffers();
    void setupRenewalTimer();
    
    // Private card data methods
    bool loadSavedCards();
    bool saveSavedCards() const;
    QString maskCardNumber(const QString& cardNumber) const;
};

#endif 