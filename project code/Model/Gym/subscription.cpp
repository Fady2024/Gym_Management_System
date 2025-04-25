#include "../Gym/subscription.h"

Subscription::Subscription()
    : type(SubscriptionType::MONTHLY), active(false), vip(false) {
}

Subscription::Subscription(SubscriptionType type, const QDate& startDate)
    : type(type), startDate(startDate), active(true), vip(false) {
    calculateEndDate();
}

// Call this to return a snapshot of the current date (time logic)
QDate Subscription::current_date() {
    return timeLogicInstance.getCurrentTime().date();
}

SubscriptionType Subscription::getType() const {
    return type;
}

QDate Subscription::getStartDate() const {
    return startDate;
}

QDate Subscription::getEndDate() const {
    return endDate;
}

double Subscription::getBasePrice() const {
    return getPriceForType(type);
}

double Subscription::getVIPPrice() const {
    return getVIPPriceForType(type);
}

double Subscription::getTotalPrice() const {
    double basePrice = getBasePrice();
    if (vip) {
        basePrice += getVIPPrice();
    }

    // Apply early renewal discount if eligible
    if (isEligibleForEarlyRenewal()) {
        double discountPercent = getEarlyRenewalDiscountPercent(getDaysUntilExpiry());
        basePrice *= (1.0 - discountPercent);
    }

    return basePrice;
}


bool Subscription::isActive() const {
    return active && endDate >= current_date();
}

bool Subscription::isVIP() const {
    return vip;
}

void Subscription::setType(SubscriptionType type) {
    this->type = type;
    calculateEndDate();
}

void Subscription::setStartDate(const QDate& date) {
    startDate = date;
    calculateEndDate();
}

void Subscription::setVIP(bool isVip) {
    vip = isVip;
}

void Subscription::renew() {
    startDate = endDate;
    calculateEndDate();
    active = true;
}

void Subscription::cancel() {
    active = false;
}

double Subscription::getPriceForType(SubscriptionType type) {
    switch (type) {
        case SubscriptionType::MONTHLY: return 29.99;
        case SubscriptionType::THREE_MONTHS: return 79.99;
        case SubscriptionType::SIX_MONTHS: return 149.99;
        case SubscriptionType::YEARLY: return 249.99;
        default: return 0.0;
    }
}

double Subscription::getVIPPriceForType(SubscriptionType type) {
    switch (type) {
        case SubscriptionType::MONTHLY: return 9.99;
        case SubscriptionType::THREE_MONTHS: return 24.99;
        case SubscriptionType::SIX_MONTHS: return 44.99;
        case SubscriptionType::YEARLY: return 74.99;
        default: return 0.0;
    }
}

int Subscription::getDurationInMonths(SubscriptionType type) {
    switch (type) {
        case SubscriptionType::MONTHLY: return 1;
        case SubscriptionType::THREE_MONTHS: return 3;
        case SubscriptionType::SIX_MONTHS: return 6;
        case SubscriptionType::YEARLY: return 12;
        default: return 0;
    }
}

QString Subscription::typeToString(SubscriptionType type) {
    switch (type) {
        case SubscriptionType::MONTHLY: return "Monthly";
        case SubscriptionType::THREE_MONTHS: return "Three Months";
        case SubscriptionType::SIX_MONTHS: return "Six Months";
        case SubscriptionType::YEARLY: return "Yearly";
        default: return "Unknown";
    }
}

SubscriptionType Subscription::stringToType(const QString& typeStr) {
    if (typeStr == "Monthly") return SubscriptionType::MONTHLY;
    if (typeStr == "Three Months") return SubscriptionType::THREE_MONTHS;
    if (typeStr == "Six Months") return SubscriptionType::SIX_MONTHS;
    if (typeStr == "Yearly") return SubscriptionType::YEARLY;
    return SubscriptionType::MONTHLY;
}

double Subscription::getEarlyRenewalDiscountPercent(int daysBeforeExpiry) {
    if (daysBeforeExpiry <= 0 || daysBeforeExpiry > EARLY_RENEWAL_THRESHOLD) {
        return 0.0;
    }

    // Calculate discount percentage based on how early they renew
    // Earlier renewal = bigger discount, up to MAX_EARLY_RENEWAL_DISCOUNT
    double factor = static_cast<double>(EARLY_RENEWAL_THRESHOLD - daysBeforeExpiry) / EARLY_RENEWAL_THRESHOLD;
    return MAX_EARLY_RENEWAL_DISCOUNT * (1.0 - factor);
}

double Subscription::getEarlyRenewalDiscount() const {
    if (!isEligibleForEarlyRenewal()) {
        return 0.0;
    }

    return getBasePrice() * getEarlyRenewalDiscountPercent(getDaysUntilExpiry());
}

bool Subscription::isEligibleForEarlyRenewal() const {
    int daysLeft = getDaysUntilExpiry();
    return daysLeft > 0 && daysLeft <= EARLY_RENEWAL_THRESHOLD;
}

int Subscription::getDaysUntilExpiry() const {
    return current_date().daysTo(endDate);
}

void Subscription::calculateEndDate() {
    // Validate start date first
    if (!startDate.isValid()) {
        qDebug() << "WARNING: Invalid start date in subscription, using current date";
        startDate = current_date();

        // If still invalid, use a fixed fallback date
        if (!startDate.isValid()) {
            qDebug() << "ERROR: Current date is also invalid, using explicit date";
            startDate = QDate(2023, 11, 15);
        }
    }

    // Check if year is reasonable (between 2023 and 2026) - expanded range to include 2025
    if (startDate.year() < 2023 || startDate.year() > 2026) {
        qDebug() << "ERROR: Unreasonable year in start date: " << startDate.year()
                << " - using fixed date instead";
        startDate = QDate(2023, 11, 15);
    }

    // Calculate end date safely
    try {
        endDate = startDate.addMonths(getDurationInMonths(type));

        // Validate the calculated end date
        if (!endDate.isValid()) {
            qDebug() << "ERROR: Invalid end date calculated, using fallback";
            // Fallback: set end date to 1 year from start date
            endDate = QDate(startDate.year() + 1, startDate.month(), startDate.day());
        }
    } catch (...) {
        qDebug() << "ERROR: Exception during end date calculation";
        // Set a reasonable fallback
        endDate = startDate.addDays(365);
    }
}

