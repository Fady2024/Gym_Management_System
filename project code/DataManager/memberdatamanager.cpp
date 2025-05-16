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
    QString projectDir;

#ifdef FORCE_SOURCE_DIR
    projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
#else
    projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
#endif

    dataDir = projectDir + "/project code/Data";
    QDir().mkpath(dataDir);

    QFile membersFile(dataDir + "/members.json");
    if (!membersFile.exists()) {
        membersFile.open(QIODevice::WriteOnly);
        membersFile.write("[]");
        membersFile.close();
    }

    if (!initializeFromFile()) {
        qDebug() << "Failed to initialize member data from file";
    }

    if (!loadSavedCards()) {
        qDebug() << "Failed to load saved card data";
    }

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MemberDataManager::checkSubscriptionStatus);
    timer->start(24 * 60 * 60 * 1000);

    setupRenewalTimer();
}

MemberDataManager::~MemberDataManager() {
    handleApplicationClosing();
}

void MemberDataManager::handleApplicationClosing() {
    if (dataModified) {
        if (!saveToFile()) {
            qDebug() << "Failed to save member data before application closing!";
        }

        if (!saveSavedCards()) {
            qDebug() << "Failed to save card data before application closing!";
        }
    }
}

bool MemberDataManager::initializeFromFile() {
    QString errorMessage;
    QJsonArray membersArray = readMembersFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
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

        if (userId > 0) {
            userIdToMemberId[userId] = memberId;
        }
    }

    return loadSavedCards();
}

bool MemberDataManager::saveToFile() {
    QMutexLocker locker(&mutex);

    if (!dataModified) {
        return true;
    }

    QJsonArray membersArray;
    for (const auto& pair : membersById) {
        membersArray.append(memberToJson(pair.second));
    }

    QString errorMessage;
    bool success = writeMembersToFile(membersArray, errorMessage);
    if (!success) {
        return false;
    }

    dataModified = false;
    return true;
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
    QMutexLocker locker(&mutex);

    if (member.getId() <= 0) {
        errorMessage = "Invalid member ID";
        return false;
    }

    auto it = membersById.find(member.getId());
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    const Subscription& subscription = member.getSubscription();
    if (!subscription.getStartDate().isValid()) {
        errorMessage = "Invalid subscription start date";
        return false;
    }

    membersById[member.getId()] = member;
    dataModified = true;

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
        return Member();
    }

    auto it = membersById.find(memberId);
    if (it != membersById.end()) {
        return it->second;
    }

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
    QDate currentDate = timeLogicInstance.getCurrentTime().date();

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
    QDate currentDate = timeLogicInstance.getCurrentTime().date();

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

    renewalCheckTimer->start(12 * 60 * 60 * 1000);

    checkSubscriptionStatus();
    checkEarlyRenewalOffers();
}

void MemberDataManager::checkEarlyRenewalOffers() {
    QDate currentDate = timeLogicInstance.getCurrentTime().date();

    for (const auto& pair : membersById) {
        int memberId = pair.first;
        const Member& member = pair.second;

        if (isEligibleForEarlyRenewal(memberId)) {
            RenewalOffer offer = getRenewalOffer(memberId);
            emit earlyRenewalOfferAvailable(offer);

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

    int daysLeft = subscription.getDaysUntilExpiry();

    SubscriptionType currentType = subscription.getType();
    bool isVip = subscription.isVIP();

    offer.daysUntilExpiry = daysLeft;
    offer.originalPrice = subscription.getTotalPrice();
    offer.isVIP = isVip;

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
        return QJsonArray();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open members file for reading";
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

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
    QFile file(QDir(dataDir).filePath("members.json"));

    QDir dir = QFileInfo(file).dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            errorMessage = "Failed to create data directory";
            return false;
        }
    }

    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open members file for writing: " + file.errorString();
        return false;
    }

    QJsonDocument doc(members);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten == -1) {
        errorMessage = "Failed to write to members file: " + file.errorString();
        file.close();
        return false;
    }

    if (bytesWritten != jsonData.size()) {
        errorMessage = "Incomplete write to members file";
        file.close();
        return false;
    }

    if (!file.flush()) {
        errorMessage = "Failed to flush file buffer: " + file.errorString();
        file.close();
        return false;
    }

    file.close();
    return true;
}

QJsonObject MemberDataManager::memberToJson(const Member& member) const {
    QJsonObject json;

    int memberId = member.getId();

    json["id"] = memberId;
    json["userId"] = member.getUserId();
    json["classId"] = member.getClassId();

    const Subscription& subscription = member.getSubscription();
    QJsonObject subscriptionObj;

    subscriptionObj["type"] = static_cast<int>(subscription.getType());
    subscriptionObj["isVIP"] = subscription.isVIP();
    subscriptionObj["active"] = subscription.isActive();

    QDate subStartDate = subscription.getStartDate();
    QDate subEndDate = subscription.getEndDate();

    if (subStartDate.isValid()) {
        subscriptionObj["startDate"] = subStartDate.toString(Qt::ISODate);
    } else {
        subscriptionObj["startDate"] = "";
    }

    if (subEndDate.isValid()) {
        subscriptionObj["endDate"] = subEndDate.toString(Qt::ISODate);
    } else {
        subscriptionObj["endDate"] = "";
    }

    json["subscription"] = subscriptionObj;

    return json;
}

Member MemberDataManager::jsonToMember(const QJsonObject& json) {
    int memberId = json["id"].toInt();
    int userId = json["userId"].toInt();
    int classId = json["classId"].toInt(-1);

    Member member(memberId, userId, classId);

    if (json.contains("subscription")) {
        QJsonObject subscriptionJson = json["subscription"].toObject();
        SubscriptionType type = static_cast<SubscriptionType>(subscriptionJson["type"].toInt(0));

        QDate startDate;
        if (subscriptionJson.contains("startDate") && !subscriptionJson["startDate"].toString().isEmpty()) {
            startDate = QDate::fromString(subscriptionJson["startDate"].toString(), Qt::ISODate);
        } else {
            startDate = timeLogicInstance.getCurrentTime().date();
        }

        Subscription subscription(type, startDate);
        subscription.setVIP(subscriptionJson["isVIP"].toBool(false));
        if (!subscriptionJson["active"].toBool(true)) {
            subscription.cancel();
        }

        member.setSubscription(subscription);
    }

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

    auto it = membersById.find(memberId);
    if (it == membersById.end()) {
        errorMessage = "Member not found";
        return false;
    }

    SavedCardData cardData;
    cardData.memberId = memberId;
    cardData.fullCardNumber = cardNumber;
    cardData.expiryDate = expiryDate;
    cardData.cardholderName = cardholderName;
    cardData.cvc = cvc;

    savedCards[memberId] = cardData;
    dataModified = true;

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

    SavedCardData emptyData;
    emptyData.memberId = -1;
    return emptyData;
}

QString MemberDataManager::maskCardNumber(const QString& cardNumber) const {
    if (cardNumber.length() < 4) {
        return "****";
    }

    QString lastFour = cardNumber.right(4);
    return QString("**** **** **** ").append(lastFour);
}

bool MemberDataManager::loadSavedCards() {
    QFile file(QDir(dataDir).filePath("saved_cards.json"));
    if (!file.exists()) {
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonArray cardsArray = doc.array();
    savedCards.clear();

    for (const QJsonValue& cardValue : cardsArray) {
        QJsonObject cardObj = cardValue.toObject();

        SavedCardData cardData;
        cardData.memberId = cardObj["memberId"].toInt();

        if (cardObj.contains("fullCardNumber")) {
            cardData.fullCardNumber = cardObj["fullCardNumber"].toString();
        } else if (cardObj.contains("maskedCardNumber")) {
            cardData.fullCardNumber = cardObj["maskedCardNumber"].toString();
        }

        cardData.expiryDate = cardObj["expiryDate"].toString();
        cardData.cardholderName = cardObj["cardholderName"].toString();

        if (cardObj.contains("cvc")) {
            cardData.cvc = cardObj["cvc"].toString();
        } else {
            cardData.cvc = "";
        }

        savedCards[cardData.memberId] = cardData;
    }

    return true;
}

bool MemberDataManager::saveSavedCards() const {
    QFile file(QDir(dataDir).filePath("saved_cards.json"));

    if (!file.open(QIODevice::WriteOnly)) {
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
        cardObj["cvc"] = cardData.cvc;

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

    if (userIsMember(userId)) {
        errorMessage = "User is already a member";
        return false;
    }

    User user = userDataManager->getUserDataById(userId);
    if (user.getId() == 0) {
        errorMessage = "User not found";
        return false;
    }

    return createMemberFromUser(user, errorMessage);
}

bool MemberDataManager::createMemberFromUser(const User& user, QString& errorMessage) {
    QMutexLocker locker(&mutex);

    if (user.getId() == 0) {
        errorMessage = "Invalid user data: ID is 0";
        return false;
    }

    int userId = user.getId();
    bool isMemberAlready = false;

    isMemberAlready = (userIdToMemberId.find(userId) != userIdToMemberId.end());

    if (isMemberAlready) {
        int memberId = userIdToMemberId[userId];
        errorMessage = QString("User is already a member (Member ID: %1)").arg(memberId);
        return false;
    }

    int memberId = generateMemberId();

    Member member(memberId, userId);

    membersById[memberId] = member;
    userIdToMemberId[userId] = memberId;

    dataModified = true;

    emit memberCreated(memberId, userId);

    return true;
}

bool MemberDataManager::userIsMember(int userId) const {
    QMutexLocker locker(&mutex);

    if (userId <= 0) {
        return false;
    }

    return (userIdToMemberId.find(userId) != userIdToMemberId.end());
}

int MemberDataManager::getMemberIdByUserId(int userId) const {
    QMutexLocker locker(&mutex);

    auto it = userIdToMemberId.find(userId);
    if (it != userIdToMemberId.end()) {
        return it->second;
    }

    return -1;
}

Member MemberDataManager::getMemberByUserId(int userId) const {
    QMutexLocker locker(&mutex);

    int memberId = getMemberIdByUserId(userId);
    if (memberId >= 0) {
        return getMemberById(memberId);
    }

    return Member();
}

bool MemberDataManager::savePaymentData(int memberId, int planId, bool isVIP, double amount,
                                        const QString& cardNumber, const QString& expiryDate, const QString& cardholderName,
                                        QString& errorMessage) const
{
    QMutexLocker locker(&mutex);

    if (memberId <= 0) {
        errorMessage = "Invalid member ID";
        return false;
    }

    QString paymentsFilePath = QDir(dataDir).filePath("payments.json");

    QJsonArray paymentsArray;
    QFile file(paymentsFilePath);

    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            errorMessage = "Could not open payments file for reading";
            return false;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();

        if (parseError.error != QJsonParseError::NoError) {
            errorMessage = "Error parsing payments file: " + parseError.errorString();
            return false;
        }

        paymentsArray = doc.array();
    }

    QJsonObject payment;
    payment["memberId"] = memberId;
    payment["timestamp"] = timeLogicInstance.getFormattedTime();
    payment["planId"] = planId;
    payment["isVIP"] = isVIP;
    payment["amount"] = amount;

    QString maskedCard = maskCardNumber(cardNumber);
    payment["cardNumber"] = maskedCard;

    payment["expiryDate"] = expiryDate;
    payment["cardholderName"] = cardholderName;

    paymentsArray.append(payment);

    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open payments file for writing";
        return false;
    }

    QJsonDocument doc(paymentsArray);
    file.write(doc.toJson());
    file.close();

    return true;
} 