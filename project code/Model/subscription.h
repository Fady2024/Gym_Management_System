#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <QDate>
#include <QString>
#include <QDebug>

enum class SubscriptionType {
    MONTHLY,
    THREE_MONTHS,
    SIX_MONTHS,
    YEARLY
};

class Subscription {
public:
    Subscription();
    Subscription(SubscriptionType type, const QDate& startDate);
    
    [[nodiscard]] SubscriptionType getType() const;
    [[nodiscard]] QDate getStartDate() const;
    [[nodiscard]] QDate getEndDate() const;
    [[nodiscard]] double getBasePrice() const;
    [[nodiscard]] double getVIPPrice() const;
    [[nodiscard]] double getTotalPrice() const;
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isVIP() const;
    [[nodiscard]] double getEarlyRenewalDiscount() const;
    [[nodiscard]] bool isEligibleForEarlyRenewal() const;
    [[nodiscard]] int getDaysUntilExpiry() const;
    
    void setType(SubscriptionType type);
    void setStartDate(const QDate& date);
    void setVIP(bool isVip);
    void renew();
    void cancel();
    
    static double getPriceForType(SubscriptionType type);
    static double getVIPPriceForType(SubscriptionType type);
    static int getDurationInMonths(SubscriptionType type);
    static QString typeToString(SubscriptionType type);
    static SubscriptionType stringToType(const QString& typeStr);
    static double getEarlyRenewalDiscountPercent(int daysBeforeExpiry);

private:
    SubscriptionType type;
    QDate startDate;
    QDate endDate;
    bool active;
    bool vip;
    
    void calculateEndDate();
    static constexpr int EARLY_RENEWAL_THRESHOLD = 30;  // Days before expiry for early renewal
    static constexpr double MAX_EARLY_RENEWAL_DISCOUNT = 0.15;  // 15% maximum discount
};

#endif 