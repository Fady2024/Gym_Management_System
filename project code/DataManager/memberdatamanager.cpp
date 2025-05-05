#include "memberdatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMutexLocker>

MemberDataManager::MemberDataManager(QObject* parent)
    : QObject(parent), userDataManager(nullptr) {
    // Get the project directory path
    QString projectDir;
    
#ifdef FORCE_SOURCE_DIR
    // Use the source directory path defined in CMake
    projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
    qDebug() << "Member - Using source directory path:" << projectDir;
#else
    // Fallback to application directory
    projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
    qDebug() << "Member - Using application directory path:" << projectDir;
#endif
    
    // Set data directory paths
    dataDir = projectDir + "/project code/Data";
    
    qDebug() << "Member - Data directory path:" << dataDir;
    
    // Create directories if they don't exist
    QDir().mkpath(dataDir);
    
    // Initialize empty members.json if it doesn't exist
    QFile membersFile(dataDir + "/members.json");
    if (!membersFile.exists()) {
        membersFile.open(QIODevice::WriteOnly);
        membersFile.write("[]");
        membersFile.close();
    }
    
    // Initialize data from file
    if (!initializeFromFile()) {
        qDebug() << "Failed to initialize member data from file";
    }
    
    // Load saved card data
    if (!loadSavedCards()) {
        qDebug() << "Failed to load saved card data";
    }

    // Set up timer to check subscription status daily
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MemberDataManager::checkSubscriptionStatus);
    timer->start(24 * 60 * 60 * 1000); // Check every 24 hours
    
    // Set up renewal check timer
    setupRenewalTimer();
}

MemberDataManager::~MemberDataManager() {
    handleApplicationClosing();
}

void MemberDataManager::handleApplicationClosing() {
    if (dataModified) {
        qDebug() << "Saving member data before application closing...";
        if (!saveToFile()) {
            qDebug() << "Failed to save member data before application closing!";
        } else {
            qDebug() << "Member data saved successfully before application closing.";
        }
        
        // Save card data
        if (!saveSavedCards()) {
            qDebug() << "Failed to save card data before application closing!";
        } else {
            qDebug() << "Card data saved successfully before application closing.";
        }
    } else {
        qDebug() << "No changes to member data, skipping save on application exit";
    }
}

bool MemberDataManager::initializeFromFile() {
    QString errorMessage;
    QJsonArray membersArray = readMembersFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading members file:" << errorMessage;
        return false;
    }

    membersById.clear();
    userIdToMemberId.clear();

    for (const QJsonValue& memberValue : membersArray) {
        QJsonObject memberObj = memberValue.toObject();
        Member member = jsonToMember(memberObj);
        int memberId = member.getId();
        int userId = memberObj["userId"].toInt();
        
        membersById[memberId] = member;
        
        // Only map if userId is valid
        if (userId > 0) {
            userIdToMemberId[userId] = memberId;
        }
    }

    // Load saved cards
    if (!loadSavedCards()) {
        qDebug() << "Failed to load saved card data";
    }

    return true;
}

bool MemberDataManager::saveToFile() {
    QMutexLocker locker(&mutex);
    
    if (!dataModified) {
        return true;  // Nothing to save
    }

    try {
        QJsonArray membersArray;
        for (const auto& pair : membersById) {
            membersArray.append(memberToJson(pair.second));
        }

        QString errorMessage;
        bool success = writeMembersToFile(membersArray, errorMessage);
        if (!success) {
            qDebug() << "Error saving members file:" << errorMessage;
            return false;
        }

        dataModified = false;
        return true;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in saveToFile: " << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in saveToFile";
        return false;
    }
}

bool MemberDataManager::addMember(const Member& member, QString& errorMessage) {
    if (member.getId() != 0) {
        errorMessage = "Member already has an ID";
        return false;
    }

    Member newMember = member;
    int newId = generateMemberId();
    newMember.setId(newId);
    membersById[newId] = newMember;
    dataModified = true;
    return true;
}

bool MemberDataManager::updateMember(const Member& member, QString& errorMessage) {
    // Use local scope QMutexLocker to prevent deadlocks
    {
        QMutexLocker locker(&mutex);

        if (member.getId() <= 0) {
            errorMessage = "Invalid member ID";
            return false;
        }

        // Find existing member
        auto it = membersById.find(member.getId());
        if (it == membersById.end()) {
            errorMessage = "Member not found";
            return false;
        }

        // Validate subscription data
        const Subscription& subscription = member.getSubscription();
        if (!subscription.getStartDate().isValid()) {
            errorMessage = "Invalid subscription start date";
            return false;
        }

        // Update member data
        membersById[member.getId()] = member;
        dataModified = true;
    }
    qDebug() << "Member data updated and marked for saving at application exit";

    return true;
}

bool MemberDataManager::deleteMember(int memberId, QString& errorMessage) {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    membersById.erase(it);
    dataModified = true;
    return true;
}

Member MemberDataManager::getMemberById(int memberId) const {
    QMutexLocker locker(&mutex);
    
    if (memberId <= 0) {
        qDebug() << "Invalid member ID requested:" << memberId;
        return Member();
    }

    auto it = membersById.find(memberId);
    if (it != membersById.end()) {
        return it->second;
    }

    qDebug() << "Member not found with ID:" << memberId;
    return Member();
}

QVector<Member> MemberDataManager::getAllMembers() const {
    QVector<Member> result;
    result.reserve(membersById.size());
    for (const auto& pair : membersById) {
        result.append(pair.second);
    }
    return result;
}

bool MemberDataManager::addSubscription(int memberId, const Subscription& subscription, QString& errorMessage) {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    Member& member = it->second;
    member.setSubscription(subscription);
    dataModified = true;
    return true;
}

bool MemberDataManager::renewSubscription(int memberId, SubscriptionType newType, bool isVIP, QString& errorMessage) {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    Member& member = it->second;
    const Subscription& oldSubscription = member.getSubscription();
    QDate currentEnd = oldSubscription.getEndDate();
    Subscription newSubscription(newType, currentEnd);
    newSubscription.setVIP(isVIP);
    
    // Apply early renewal discount if eligible
    if (isEligibleForEarlyRenewal(memberId)) {
        double discount = calculateRenewalDiscount(memberId, newType);
    }
    
    member.setSubscription(newSubscription);
    dataModified = true;
    
    emit vipStatusChanged(memberId, isVIP);
    return true;
}

bool MemberDataManager::cancelSubscription(int memberId, QString& errorMessage) {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    Member& member = it->second;
    Subscription& subscription = const_cast<Subscription&>(member.getSubscription());
    subscription.cancel();
    dataModified = true;
    return true;
}

QVector<Member> MemberDataManager::getMembersNeedingRenewal(int daysThreshold) const {
    QVector<Member> result;
    QDate currentDate = QDate::currentDate();
    
    for (const auto& pair : membersById) {
        const Member& member = pair.second;
        const Subscription& subscription = member.getSubscription();
        QDate endDate = subscription.getEndDate();
        int daysLeft = currentDate.daysTo(endDate);
        
        if (daysLeft <= daysThreshold && daysLeft >= 0) {
            result.append(member);
        }
    }
    
    return result;
}

bool MemberDataManager::isSubscriptionActive(int memberId) const {
    auto it = membersById.find(memberId);
    if (it != membersById.end()) {
        const Member& member = it->second;
        return member.getSubscription().isActive();
    }
    return false;
}

bool MemberDataManager::isVIPMember(int memberId) const {
    auto it = membersById.find(memberId);
    if (it != membersById.end()) {
        return it->second.getSubscription().isVIP();
    }
    return false;
}

bool MemberDataManager::addWorkoutToHistory(int memberId, const QString& workoutType, const QDate& date, QString& errorMessage) {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    Member& member = it->second;
    member.addClassToHistory(date);
    dataModified = true;
    return true;
}

QVector<QPair<int, QDate>> MemberDataManager::getWorkoutHistory(int memberId) const {
    auto it = membersById.find(memberId);
    if (it != membersById.end()) {
        return it->second.getHistory().toVector();
    }
    return QVector<QPair<int, QDate>>();
}

QVector<QPair<int, QDate>> MemberDataManager::getRecentWorkouts(int memberId, int count) const {
    auto it = membersById.find(memberId);
    if (it != membersById.end()) {
        QList<QPair<int, QDate>> history = it->second.getHistory();
        QVector<QPair<int, QDate>> recentWorkouts;
        
        int startIndex = qMax(0, history.size() - count);
        for (int i = startIndex; i < history.size(); ++i) {
            recentWorkouts.append(history[i]);
        }
        
        return recentWorkouts;
    }
    return QVector<QPair<int, QDate>>();
}

void MemberDataManager::checkSubscriptionStatus() {
    QDate currentDate = QDate::currentDate();
    
    for (const auto& pair : membersById) {
        int memberId = pair.first;
        const Member& member = pair.second;
        const Subscription& subscription = member.getSubscription();
        QDate endDate = subscription.getEndDate();
        int daysLeft = currentDate.daysTo(endDate);
        
        if (daysLeft <= 7 && daysLeft > 0) {
            emit subscriptionNearingExpiry(memberId, daysLeft);
        } else if (daysLeft <= 0) {
            emit subscriptionExpired(memberId);
        }
    }
}

void MemberDataManager::setupRenewalTimer() {
    renewalCheckTimer = new QTimer(this);
    connect(renewalCheckTimer, &QTimer::timeout, this, [this]() {
        checkSubscriptionStatus();
        checkEarlyRenewalOffers();
    });
    
    // Check every 12 hours
    renewalCheckTimer->start(12 * 60 * 60 * 1000);
    
    // Initial check
    checkSubscriptionStatus();
    checkEarlyRenewalOffers();
}

void MemberDataManager::checkEarlyRenewalOffers() {
    QDate currentDate = QDate::currentDate();
    
    for (const auto& pair : membersById) {
        int memberId = pair.first;
        const Member& member = pair.second;
        
        if (isEligibleForEarlyRenewal(memberId)) {
            RenewalOffer offer = getRenewalOffer(memberId);
            emit earlyRenewalOfferAvailable(offer);
            
            // If discount is about to expire (3 days left)
            if (offer.daysUntilExpiry <= 3) {
                emit renewalDiscountExpiring(memberId, offer.daysUntilExpiry);
            }
        }
    }
}

RenewalOffer MemberDataManager::getRenewalOffer(int memberId) const {
    RenewalOffer offer{memberId, 0.0, 0.0, 0, false};
    
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        return offer;
    }
    
    const Member& member = it->second;
    const Subscription& subscription = member.getSubscription();
    
    // Get days until expiry directly from subscription
    int daysLeft = subscription.getDaysUntilExpiry();
    
    // Get current subscription type and VIP status
    SubscriptionType currentType = subscription.getType();
    bool isVip = subscription.isVIP();
    
    // Populate offer data
    offer.daysUntilExpiry = daysLeft;
    offer.originalPrice = subscription.getTotalPrice();
    offer.isVIP = isVip;
    
    // Calculate discounted price based on early renewal
    if (daysLeft > 0 && daysLeft <= 30) {
        double discountPercent = Subscription::getEarlyRenewalDiscountPercent(daysLeft);
        offer.discountedPrice = offer.originalPrice * (1.0 - discountPercent);
    } else {
        offer.discountedPrice = offer.originalPrice;
    }
    
    return offer;
}

QVector<RenewalOffer> MemberDataManager::getAllRenewalOffers() const {
    QVector<RenewalOffer> offers;
    
    for (const auto& pair : membersById) {
        int memberId = pair.first;
        if (isEligibleForEarlyRenewal(memberId)) {
            offers.append(getRenewalOffer(memberId));
        }
    }
    
    return offers;
}

double MemberDataManager::calculateRenewalDiscount(int memberId, SubscriptionType newType) const {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        return 0.0;
    }
    
    const Member& member = it->second;
    const Subscription& subscription = member.getSubscription();
    
    if (!isEligibleForEarlyRenewal(memberId)) {
        return 0.0;
    }
    
    // Calculate the early renewal discount
    double basePrice = Subscription::getPriceForType(newType);
    if (subscription.isVIP()) {
        basePrice += Subscription::getVIPPriceForType(newType);
    }
    
    int daysLeft = subscription.getDaysUntilExpiry();
    double discountPercent = Subscription::getEarlyRenewalDiscountPercent(daysLeft);
    return basePrice * discountPercent;
}

bool MemberDataManager::isEligibleForEarlyRenewal(int memberId) const {
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        return false;
    }
    
    const Member& member = it->second;
    return member.getSubscription().isEligibleForEarlyRenewal();
}

QJsonArray MemberDataManager::readMembersFromFile(QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("members.json"));
    if (!file.exists()) {
        qDebug() << "Members file does not exist, will be created on first save";
        return QJsonArray();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open members file for reading";
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    // If file is empty, return empty array
    if (data.isEmpty() || data.trimmed().isEmpty()) {
        return QJsonArray();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing members file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}

bool MemberDataManager::writeMembersToFile(const QJsonArray& members, QString& errorMessage) const {
    try {
        QFile file(QDir(dataDir).filePath("members.json"));
        
        // Make sure the directory exists
        QDir dir = QFileInfo(file).dir();
        if (!dir.exists()) {
            qDebug() << "Creating directory: " << dir.path();
            if (!dir.mkpath(".")) {
                errorMessage = "Failed to create data directory";
                return false;
            }
        }

        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = "Could not open members file for writing: " + file.errorString();
            qDebug() << errorMessage;
            return false;
        }

        QJsonDocument doc(members);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
        
        qint64 bytesWritten = file.write(jsonData);
        if (bytesWritten == -1) {
            errorMessage = "Failed to write to members file: " + file.errorString();
            qDebug() << errorMessage;
            file.close();
            return false;
        }
        
        if (bytesWritten != jsonData.size()) {
            errorMessage = "Incomplete write to members file";
            qDebug() << errorMessage;
            file.close();
            return false;
        }

        // Make sure data is written to disk
        if (!file.flush()) {
            errorMessage = "Failed to flush file buffer: " + file.errorString();
            qDebug() << errorMessage;
            file.close();
            return false;
        }

        file.close();
        return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in writeMembersToFile: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in writeMembersToFile";
        qDebug() << errorMessage;
        return false;
    }
}

QJsonObject MemberDataManager::memberToJson(const Member& member) const {
    try {
        QJsonObject json;
        
        // Validate member ID before proceeding
        int memberId = member.getId();
        if (memberId <= 0) {
            qDebug() << "Warning: Invalid member ID in memberToJson";
            // Still proceed, but with a warning
        }
        
        json["id"] = memberId;
        json["userId"] = member.getUserId();
        json["classId"] = member.getClassId();
        
        // Add subscription data
        try {
            const Subscription& subscription = member.getSubscription();
            QJsonObject subscriptionObj;
            
            // Basic subscription data
            subscriptionObj["type"] = static_cast<int>(subscription.getType());
            subscriptionObj["isVIP"] = subscription.isVIP();
            subscriptionObj["active"] = subscription.isActive();
            
            // Safe date handling for subscription
            QDate subStartDate = subscription.getStartDate();
            QDate subEndDate = subscription.getEndDate();
            
            if (subStartDate.isValid()) {
                subscriptionObj["startDate"] = subStartDate.toString(Qt::ISODate);
            } else {
                subscriptionObj["startDate"] = "";
                qDebug() << "Warning: Invalid subscription start date for member " << memberId;
            }
            
            if (subEndDate.isValid()) {
                subscriptionObj["endDate"] = subEndDate.toString(Qt::ISODate);
            } else {
                subscriptionObj["endDate"] = "";
            }
            
            json["subscription"] = subscriptionObj;
        }
        catch (const std::exception& e) {
            qDebug() << "Exception processing subscription data: " << e.what();
            // Continue without subscription data
            json["subscription"] = QJsonObject();
        }
        catch (...) {
            qDebug() << "Unknown exception processing subscription data";
            json["subscription"] = QJsonObject();
        }
        
        return json;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in memberToJson: " << e.what();
        return QJsonObject(); // Return empty object on error
    }
    catch (...) {
        qDebug() << "Unknown exception in memberToJson";
        return QJsonObject(); // Return empty object on error
    }
}

Member MemberDataManager::jsonToMember(const QJsonObject& json) {
    try {
        int memberId = json["id"].toInt();
        int userId = json["userId"].toInt();
        int classId = json["classId"].toInt(-1);
        
        // Create a base member
        Member member(memberId, userId, classId);

        // Load subscription if it exists
        if (json.contains("subscription")) {
            QJsonObject subscriptionJson = json["subscription"].toObject();
            SubscriptionType type = static_cast<SubscriptionType>(subscriptionJson["type"].toInt(0));
            
            // Get start date from subscription object
            QDate startDate;
            if (subscriptionJson.contains("startDate") && !subscriptionJson["startDate"].toString().isEmpty()) {
                startDate = QDate::fromString(subscriptionJson["startDate"].toString(), Qt::ISODate);
            } else {
                startDate = QDate::currentDate(); // Fallback
            }
            
            // Create and configure the subscription
            Subscription subscription(type, startDate);
            subscription.setVIP(subscriptionJson["isVIP"].toBool(false));
            if (!subscriptionJson["active"].toBool(true)) {
                subscription.cancel();
            }
            
            member.setSubscription(subscription);
        }

        // Parse workout history if exists
        if (json.contains("history") && json["history"].isArray()) {
            QJsonArray historyArray = json["history"].toArray();
            for (const QJsonValue& value : historyArray) {
                QJsonObject entry = value.toObject();
                QDate workoutDate = QDate::fromString(entry["date"].toString(), Qt::ISODate);
                if (workoutDate.isValid()) {
                    member.addClassToHistory(workoutDate);
                }
            }
        }

        return member;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in jsonToMember: " << e.what();
        // Return a default member on error
        return Member();
    }
    catch (...) {
        qDebug() << "Unknown exception in jsonToMember";
        // Return a default member on error
        return Member();
    }
}

int MemberDataManager::generateMemberId() const {
    int maxId = 0;
    for (const auto& pair : membersById) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }
    return maxId + 1;
}

bool MemberDataManager::saveCardData(int memberId, const QString& cardNumber, const QString& expiryDate, 
                                    const QString& cardholderName, const QString& cvc, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    if (memberId <= 0) {
        errorMessage = "Invalid member ID";
        return false;
    }
    
    // Ensure member exists
    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }
    
    // Store card data with full card number
    SavedCardData cardData;
    cardData.memberId = memberId;
    cardData.fullCardNumber = cardNumber;  // Store full card number
    cardData.expiryDate = expiryDate;
    cardData.cardholderName = cardholderName;
    cardData.cvc = cvc;  // Store CVC code for future use
    
    savedCards[memberId] = cardData;
    dataModified = true;
    
    qDebug() << "Card data updated and marked for saving at application exit";
    
    return true;
}

bool MemberDataManager::hasStoredCard(int memberId) const {
    QMutexLocker locker(&mutex);
    return savedCards.find(memberId) != savedCards.end();
}

SavedCardData MemberDataManager::getStoredCard(int memberId) const {
    QMutexLocker locker(&mutex);
    
    auto it = savedCards.find(memberId);
    if (it != savedCards.end()) {
        return it->second;
    }
    
    // Return empty data if not found
    SavedCardData emptyData;
    emptyData.memberId = -1;
    return emptyData;
}

QString MemberDataManager::maskCardNumber(const QString& cardNumber) const {
    if (cardNumber.length() < 4) {
        return "****";
    }
    
    // Only store the last 4 digits, mask the rest
    QString lastFour = cardNumber.right(4);
    return QString("**** **** **** ").append(lastFour);
}

bool MemberDataManager::loadSavedCards() {
    QFile file(QDir(dataDir).filePath("saved_cards.json"));
    if (!file.exists()) {
        // No saved cards yet, not an error
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open saved cards file for reading";
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Error parsing saved cards file:" << parseError.errorString();
        return false;
    }
    
    QJsonArray cardsArray = doc.array();
    savedCards.clear();
    
    for (const QJsonValue& cardValue : cardsArray) {
        QJsonObject cardObj = cardValue.toObject();
        
        SavedCardData cardData;
        cardData.memberId = cardObj["memberId"].toInt();
        
        // Check if we have the full card number or just the masked version
        if (cardObj.contains("fullCardNumber")) {
            cardData.fullCardNumber = cardObj["fullCardNumber"].toString();
        } else if (cardObj.contains("maskedCardNumber")) {
            // For backward compatibility with old saved data
            cardData.fullCardNumber = cardObj["maskedCardNumber"].toString();
        }
        
        cardData.expiryDate = cardObj["expiryDate"].toString();
        cardData.cardholderName = cardObj["cardholderName"].toString();
        
        // Load CVC if available (for backward compatibility with existing files)
        if (cardObj.contains("cvc")) {
            cardData.cvc = cardObj["cvc"].toString();
        } else {
            cardData.cvc = ""; // Empty string if not available
        }
        
        savedCards[cardData.memberId] = cardData;
    }
    
    return true;
}

bool MemberDataManager::saveSavedCards() const {
    QFile file(QDir(dataDir).filePath("saved_cards.json"));
    
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open saved cards file for writing";
        return false;
    }
    
    QJsonArray cardsArray;
    
    for (const auto& pair : savedCards) {
        const SavedCardData& cardData = pair.second;
        
        QJsonObject cardObj;
        cardObj["memberId"] = cardData.memberId;
        cardObj["fullCardNumber"] = cardData.fullCardNumber;
        cardObj["expiryDate"] = cardData.expiryDate;
        cardObj["cardholderName"] = cardData.cardholderName;
        cardObj["cvc"] = cardData.cvc;  // Include CVC in saved data
        
        cardsArray.append(cardObj);
    }
    
    QJsonDocument doc(cardsArray);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

void MemberDataManager::setUserDataManager(UserDataManager* userManager) {
    userDataManager = userManager;
}

bool MemberDataManager::createMemberFromUser(int userId, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    if (userDataManager == nullptr) {
        errorMessage = "User data manager not initialized";
        return false;
    }
    
    // Check if already a member
    if (userIsMember(userId)) {
        errorMessage = "User is already a member";
        return false;
    }
    
    // Get user data
    User user = userDataManager->getUserDataById(userId);
    if (user.getId() == 0) {
        errorMessage = "User not found";
        return false;
    }
    
    return createMemberFromUser(user, errorMessage);
}

bool MemberDataManager::createMemberFromUser(const User& user, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    qDebug() << "MemberDataManager::createMemberFromUser called for user ID: " << user.getId();
    
    if (user.getId() == 0) {
        errorMessage = "Invalid user data: ID is 0";
        qDebug() << "Failed to create member: " << errorMessage;
        return false;
    }
    
    try {
        // Check if already a member - DIRECTLY CHECK THE MAP TO AVOID RECURSION
        qDebug() << "Checking if user is already a member (direct map check)";
        int userId = user.getId();
        bool isMemberAlready = false;
        
        // Safe direct access to the map
        isMemberAlready = (userIdToMemberId.find(userId) != userIdToMemberId.end());
        
        if (isMemberAlready) {
            int memberId = userIdToMemberId[userId];
            errorMessage = QString("User is already a member (Member ID: %1)").arg(memberId);
            qDebug() << "Failed to create member: " << errorMessage;
            return false;
        }
        
        qDebug() << "Creating new member from user data: " << user.getName() << " (" << user.getEmail() << ")";
        
        // Generate a new member ID
        int memberId = generateMemberId();
        qDebug() << "Generated new member ID: " << memberId << " for user ID: " << userId;
        
        // Create new member with generated ID and user ID
        Member member(memberId, userId);
        
        // Add to maps - wrap in try blocks for safety
        try {
            qDebug() << "Adding member to membersById map";
            membersById[memberId] = member;
        } catch (const std::exception& e) {
            errorMessage = QString("Exception adding to membersById: %1").arg(e.what());
            qDebug() << errorMessage;
            return false;
        }
        
        try {
            qDebug() << "Adding user-member mapping to userIdToMemberId map";
            userIdToMemberId[userId] = memberId;
        } catch (const std::exception& e) {
            // Rollback the previous insertion if this one fails
            membersById.erase(memberId);
            errorMessage = QString("Exception adding to userIdToMemberId: %1").arg(e.what());
            qDebug() << errorMessage;
            return false;
        }
        
        dataModified = true;
        
        // Emit signal
        qDebug() << "Emitting memberCreated signal for member ID: " << memberId;
        emit memberCreated(memberId, userId);
        qDebug() << "Member created successfully with ID: " << memberId;
        
        return true;
    } catch (const std::exception& e) {
        errorMessage = QString("Exception creating member: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    } catch (...) {
        errorMessage = "Unknown exception creating member";
        qDebug() << errorMessage;
        return false;
    }
}

bool MemberDataManager::userIsMember(int userId) const {
    try {
        QMutexLocker locker(&mutex);
        qDebug() << "Checking if user ID " << userId << " is a member (safe method)";
        
        if (userId <= 0) {
            qDebug() << "Invalid user ID: " << userId;
            return false;
        }
        
        // Safely check if the key exists in the map
        bool result = (userIdToMemberId.find(userId) != userIdToMemberId.end());
        qDebug() << "User ID " << userId << " is member: " << result;
        return result;
    } catch (const std::exception& e) {
        qDebug() << "Exception in userIsMember: " << e.what();
        return false;
    } catch (...) {
        qDebug() << "Unknown exception in userIsMember";
        return false;
    }
}

int MemberDataManager::getMemberIdByUserId(int userId) const {
    QMutexLocker locker(&mutex);
    
    auto it = userIdToMemberId.find(userId);
    if (it != userIdToMemberId.end()) {
        return it->second;
    }
    
    return -1;  // Not found
}

Member MemberDataManager::getMemberByUserId(int userId) const {
    QMutexLocker locker(&mutex);
    
    int memberId = getMemberIdByUserId(userId);
    if (memberId >= 0) {
        return getMemberById(memberId);
    }
    
    return Member();  // Return empty member if not found
}

bool MemberDataManager::savePaymentData(int memberId, int planId, bool isVIP, double amount, 
                                        const QString& cardNumber, const QString& expiryDate, const QString& cardholderName,
                                        QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    qDebug() << "Saving payment data for member ID:" << memberId;
    
    // Validate memberId
    if (memberId <= 0) {
        errorMessage = "Invalid member ID";
        qDebug() << "Failed to save payment data:" << errorMessage;
        return false;
    }
    
    try {
        // Get the payments file path
        QString paymentsFilePath = QDir(dataDir).filePath("payments.json");
        
        // Read existing payments if the file exists
        QJsonArray paymentsArray;
        QFile file(paymentsFilePath);
        
        if (file.exists()) {
            if (!file.open(QIODevice::ReadOnly)) {
                errorMessage = "Could not open payments file for reading";
                qDebug() << "Failed to save payment data:" << errorMessage;
                return false;
            }
            
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
            file.close();
            
            if (parseError.error != QJsonParseError::NoError) {
                errorMessage = "Error parsing payments file: " + parseError.errorString();
                qDebug() << "Failed to save payment data:" << errorMessage;
                return false;
            }
            
            paymentsArray = doc.array();
        }
        
        // Create payment record
        QJsonObject payment;
        payment["memberId"] = memberId;
        payment["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        payment["planId"] = planId;
        payment["isVIP"] = isVIP;
        payment["amount"] = amount;
        
        // Reuse maskCardNumber function instead of duplicating logic
        QString maskedCard = maskCardNumber(cardNumber);
        payment["cardNumber"] = maskedCard;
        
        payment["expiryDate"] = expiryDate;
        payment["cardholderName"] = cardholderName;
        
        // Add payment to array
        paymentsArray.append(payment);
        
        // Write updated payments to file
        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = "Could not open payments file for writing";
            qDebug() << "Failed to save payment data:" << errorMessage;
            return false;
        }
        
        QJsonDocument doc(paymentsArray);
        file.write(doc.toJson());
        file.close();
        
        qDebug() << "Payment data saved successfully for member ID:" << memberId;
        return true;
        
    } catch (const std::exception& e) {
        errorMessage = QString("Exception saving payment data: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    } catch (...) {
        errorMessage = "Unknown exception saving payment data";
        qDebug() << errorMessage;
        return false;
    }
} 