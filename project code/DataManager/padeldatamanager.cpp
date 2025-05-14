#include "padeldatamanager.h"
#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QTimer>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMutexLocker>
#include <algorithm>
#include <QDateTime>
#include <QMetaObject>

PadelDataManager::PadelDataManager(QObject* parent)
    : QObject(parent), memberDataManager(nullptr) {

    QString projectDir;
    
#ifdef FORCE_SOURCE_DIR
    projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
#else
    projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
#endif

    dataDir = projectDir + "/project code/Data";

    QDir().mkpath(dataDir);

    QFile courtsFile(dataDir + "/courts.json");
    if (!courtsFile.exists()) {
        if (courtsFile.open(QIODevice::WriteOnly)) {
            courtsFile.write("[]");
            courtsFile.close();
        }
    }

    QFile bookingsFile(dataDir + "/bookings.json");
    if (!bookingsFile.exists()) {
        if (bookingsFile.open(QIODevice::WriteOnly)) {
            bookingsFile.write("[]");
            bookingsFile.close();
        }
    }

    QFile waitlistsFile(dataDir + "/waitlists.json");
    if (!waitlistsFile.exists()) {
        if (waitlistsFile.open(QIODevice::WriteOnly)) {
            waitlistsFile.write("[]");
            waitlistsFile.close();
        }
    }

    initializeFromFile();

    setupTimers();
}

PadelDataManager::~PadelDataManager() {
    handleApplicationClosing();
}

void PadelDataManager::handleApplicationClosing() {
    if (dataModified) {
        saveToFile();
    }
}

bool PadelDataManager::initializeFromFile() {
    QString errorMessage;
    QMutexLocker locker(&mutex);

    QJsonArray courtsArray = readCourtsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        return false;
    }

    courtsById.clear();
    for (const QJsonValue& courtValue : courtsArray) {
        QJsonObject courtObj = courtValue.toObject();
        Court court = jsonToCourt(courtObj);
        courtsById[court.getId()] = court;
    }

    QJsonArray bookingsArray = readBookingsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        return false;
    }

    bookingsById.clear();
    for (const QJsonValue& bookingValue : bookingsArray) {
        QJsonObject bookingObj = bookingValue.toObject();
        Booking booking = jsonToBooking(bookingObj);
        bookingsById[booking.getBookingId()] = booking;
    }

    QJsonArray waitlistsArray = readWaitlistsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        return false;
    }

    courtWaitlists.clear();
    for (const QJsonValue& entryValue : waitlistsArray) {
        QJsonObject entryObj = entryValue.toObject();

        WaitlistEntry entry;
        if (entryObj.contains("userId")) {
            entry.memberId = entryObj["userId"].toInt();
        } else {
            entry.memberId = entryObj["memberId"].toInt();
        }

        entry.courtId = entryObj["courtId"].toInt();
        entry.requestedTime = QDateTime::fromString(entryObj["requestedTime"].toString(), Qt::ISODate);
        entry.isVIP = entryObj["isVIP"].toBool();
        entry.priority = entryObj["priority"].toInt();

        int courtId = entry.courtId;
        courtWaitlists[courtId].push(entry);
    }

    return true;
}

bool PadelDataManager::saveToFile() {
    if (!dataModified) {
        return true;
    }

    QString errorMessage;

    QDir dataDirectory(dataDir);
    if (!dataDirectory.exists()) {
        bool dirCreated = QDir().mkpath(dataDir);
        if (!dirCreated) {
            return false;
        }
    }

    QJsonArray waitlistsArray = waitlistsToJson();
    QString filePath = QDir(dataDir).filePath("waitlists.json");
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(waitlistsArray);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten == -1) {
        file.close();
        return false;
    }

    file.flush();
    file.close();
    
    dataModified = false;
    return true;
}

bool PadelDataManager::writeBookingsToFile(const QJsonArray& bookings, QString& errorMessage) const {
    QString filePath = QDir(dataDir).filePath("bookings.json");
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.absolutePath());

    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open bookings file for writing: " + file.errorString();
        return false;
    }

    QJsonDocument doc(bookings);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten == -1) {
        errorMessage = "Failed to write to bookings file: " + file.errorString();
        file.close();
        return false;
    }

    file.flush();
    file.close();
    return true;
}

bool PadelDataManager::writeCourtsToFile(const QJsonArray& courts, QString& errorMessage) const {
    QString filePath = QDir(dataDir).filePath("courts.json");
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.absolutePath());

    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open courts file for writing: " + file.errorString();
        return false;
    }

    QJsonDocument doc(courts);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten == -1) {
        errorMessage = "Failed to write to courts file: " + file.errorString();
        file.close();
        return false;
    }

    file.flush();
    file.close();
    return true;
}

QJsonObject PadelDataManager::courtToJson(const Court& court) const {
    QJsonObject json;
    json["id"] = court.getId();
    json["name"] = court.getName();
    json["location"] = court.getLocation();
    json["isIndoor"] = court.isIndoor();
    json["pricePerHour"] = court.getPricePerHour();
    json["maxAttendees"] = court.getMaxAttendees();

    if (!court.getDescription().isEmpty()) {
        json["description"] = court.getDescription();
    }

    const QStringList& features = court.getFeatures();
    if (!features.isEmpty()) {
        QJsonArray featuresArray;
        for (const QString& feature : features) {
            featuresArray.append(feature);
        }
        json["features"] = featuresArray;
    }

    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();
    QJsonArray timeSlotsArray;
    for (const QTime& time : timeSlots) {
        timeSlotsArray.append(time.toString("HH:mm"));
    }
    json["timeSlots"] = timeSlotsArray;

    return json;
}

QJsonObject PadelDataManager::bookingToJson(const Booking& booking) const {
    QJsonObject json;
    json["id"] = booking.getBookingId();
    json["courtId"] = booking.getCourtId();
    json["userId"] = booking.getUserId();
    json["startTime"] = booking.getStartTime().toString(Qt::ISODate);
    json["endTime"] = booking.getEndTime().toString(Qt::ISODate);
    json["price"] = booking.getPrice();
    json["isVip"] = booking.isVip();
    json["isCancelled"] = booking.isCancelled();
    json["isFromWaitlist"] = booking.isFromWaitlist();

    return json;
}

Booking PadelDataManager::jsonToBooking(const QJsonObject& json) {
    Booking booking;
    booking.setBookingId(json["id"].toInt());
    booking.setCourtId(json["courtId"].toInt());
    booking.setUserId(json["userId"].toInt());
    booking.setStartTime(QDateTime::fromString(json["startTime"].toString(), Qt::ISODate));
    booking.setEndTime(QDateTime::fromString(json["endTime"].toString(), Qt::ISODate));
    booking.setPrice(json["price"].toDouble());
    booking.setVip(json["isVip"].toBool(false));
    booking.setIsFromWaitlist(json["isFromWaitlist"].toBool(false));

    if (json["isCancelled"].toBool(false)) {
        booking.setCancelled(true);
    }

    return booking;
}

bool PadelDataManager::addCourt(const Court& court, QString& errorMessage) {
    QMutexLocker locker(&mutex);

    if (court.getId() != 0) {
        errorMessage = "Court already has an ID";
        return false;
    }

    Court newCourt = court;
    int newId = generateCourtId();
    newCourt.setId(newId);
    courtsById[newId] = newCourt;
    dataModified = true;

    emit courtAdded(newId);
    return true;
}

bool PadelDataManager::updateCourt(const Court& court, QString& errorMessage) {
    QMutexLocker locker(&mutex);

    if (court.getId() <= 0) {
        errorMessage = "Invalid court ID";
        return false;
    }

    auto it = courtsById.find(court.getId());
    if (it == courtsById.end()) {
        errorMessage = "Court not found";
        return false;
    }

    courtsById[court.getId()] = court;
    dataModified = true;

    emit courtUpdated(court.getId());
    return true;
}

bool PadelDataManager::deleteCourt(int courtId, QString& errorMessage) {
    QMutexLocker locker(&mutex);

    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        errorMessage = "Court not found";
        return false;
    }

    for (const auto& pair : bookingsById) {
        if (pair.second.getCourt().getId() == courtId) {
            errorMessage = "Cannot delete court with existing bookings";
            return false;
        }
    }

    courtsById.erase(it);
    dataModified = true;

    emit courtDeleted(courtId);
    return true;
}

Court PadelDataManager::getCourtById(int courtId) const {
    if (courtId <= 0) {
        return Court();
    }

    QMutex& mutexRef = mutex;
    QMutexLocker locker(&mutexRef);

    if (courtsById.empty()) {
        return Court();
    }

    auto it = courtsById.find(courtId);
    if (it != courtsById.end()) {
        return it->second;
    }

    return Court();
}

QVector<Court> PadelDataManager::getAllCourts() const {
    QMutexLocker locker(&mutex);

    QVector<Court> result;
    result.reserve(courtsById.size());
    for (const auto& pair : courtsById) {
        result.append(pair.second);
    }
    return result;
}

QVector<Court> PadelDataManager::getCourtsByLocation(const QString& location) const {
    QMutexLocker locker(&mutex);

    QVector<Court> result;
    for (const auto& pair : courtsById) {
        if (pair.second.getLocation() == location) {
            result.append(pair.second);
        }
    }
    return result;
}

bool PadelDataManager::createBooking(int userId, int courtId, const QDateTime& startTime,
                                   const QDateTime& endTime, QString& errorMessage, bool isFromWaitlist) {
    if (userId <= 0) {
        errorMessage = "Invalid user ID";
        return false;
    }

    if (courtId <= 0) {
        errorMessage = "Invalid court ID";
        return false;
    }

    QMutexLocker locker(&mutex);

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        errorMessage = "Court not found with ID: " + QString::number(courtId);
        return false;
    }

    const Court& court = courtIt->second;
    int maxAttendees = court.getMaxAttendees();

    if (!startTime.isValid() || !endTime.isValid()) {
        errorMessage = "Invalid booking time";
        return false;
    }

    if (startTime >= endTime) {
        errorMessage = "Start time must be before end time";
        return false;
    }

    int currentAttendees = 0;

    for (const auto& pair : bookingsById) {
        const Booking& existingBooking = pair.second;

        if (existingBooking.isCancelled()) {
            continue;
        }

        if (existingBooking.getCourtId() != courtId) {
            continue;
        }

        if (!existingBooking.getStartTime().isValid() ||
            !startTime.isValid() ||
            existingBooking.getStartTime().date() != startTime.date()) {
            continue;
        }

        if (!existingBooking.getStartTime().isValid() ||
            !existingBooking.getEndTime().isValid()) {
            continue;
        }

        bool overlap = (existingBooking.getStartTime() <= endTime &&
                       existingBooking.getEndTime() >= startTime);

        if (overlap) {
            currentAttendees++;
        }
    }

    if (currentAttendees >= maxAttendees) {
        bool isVip = false;

        if (memberDataManager) {
            if (memberDataManager->userIsMember(userId)) {
                int memberId = memberDataManager->getMemberIdByUserId(userId);
                isVip = memberDataManager->isVIPMember(memberId);
            }
        }

        QString waitlistError;
        if (addToWaitlist(userId, courtId, startTime, waitlistError)) {
            errorMessage = "Court is at maximum capacity. You have been added to the waitlist.";
            return false;
        } else {
            errorMessage = "Court is at maximum capacity and could not add to waitlist: " + waitlistError;
            return false;
        }
    }

    for (const auto& pair : bookingsById) {
        const Booking& existingBooking = pair.second;

        if (existingBooking.isCancelled()) {
            continue;
        }

        if (existingBooking.getUserId() == userId &&
            existingBooking.getCourtId() == courtId &&
            existingBooking.getStartTime().date() == startTime.date()) {

            bool timeOverlap = false;
            QDateTime bookingStart = existingBooking.getStartTime();
            QDateTime bookingEnd = existingBooking.getEndTime();

            if (bookingStart.isValid() && bookingEnd.isValid()) {
                timeOverlap = (bookingStart < endTime && bookingEnd > startTime);
            }

            if (timeOverlap) {
                errorMessage = "You already have a booking at this time on this court";
                return false;
            }
        }
    }

    User user;

    if (!memberDataManager) {
        errorMessage = "Member data manager is not set";
        return false;
    }

    UserDataManager* userManager = memberDataManager->getUserDataManager();
    if (!userManager) {
        errorMessage = "User data manager is not set";
        return false;
    }

    user = userManager->getUserDataById(userId);
    if (user.getId() <= 0) {
        errorMessage = "User not found with ID: " + QString::number(userId);
        return false;
    }

    int bookingId = generateBookingId();

    Booking newBooking;
    newBooking.setBookingId(bookingId);
    newBooking.setCourtId(courtId);
    newBooking.setStartTime(startTime);
    newBooking.setEndTime(endTime);
    newBooking.setUserId(userId);
    newBooking.setIsFromWaitlist(isFromWaitlist);

    bool isVip = false;
    if (memberDataManager->userIsMember(userId)) {
        int memberId = memberDataManager->getMemberIdByUserId(userId);
        isVip = memberDataManager->isVIPMember(memberId);
    }

    double price = calculateBookingPrice(courtId, startTime, endTime, isVip);
    newBooking.setPrice(price);
    newBooking.setVip(isVip);

    bookingsById[bookingId] = newBooking;
    dataModified = true;

    QMetaObject::invokeMethod(this, "safeEmitBookingCreated",
                          Qt::QueuedConnection,
                          Q_ARG(int, bookingId),
                          Q_ARG(int, userId));

    saveToFile();
    
    return true;
}

void PadelDataManager::safeEmitBookingCreated(int bookingId, int userId) {
    emit bookingCreated(bookingId, userId);
}

double PadelDataManager::calculateBookingPrice(int courtId, const QDateTime& startTime,
                                             const QDateTime& endTime, bool isVIP) const {
    double pricePerHour = 100.0;

    bool locked = mutex.tryLock(100);
    if (locked) {
        auto it = courtsById.find(courtId);
        if (it != courtsById.end()) {
            pricePerHour = it->second.getPricePerHour();
        }
        mutex.unlock();
    }

    int durationInSeconds = startTime.secsTo(endTime);
    double durationInHours = static_cast<double>(durationInSeconds) / 3600.0;
    double basePrice = pricePerHour * durationInHours;
    double finalPrice = basePrice;

    if (isVIP) {
        finalPrice = basePrice * 0.85;
    }

    return finalPrice;
}

bool PadelDataManager::deleteBooking(int bookingId, QString& errorMessage) {
    QMutexLocker locker(&mutex);

    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
        errorMessage = "Booking not found";
        return false;
    }

    bookingsById.erase(it);
    dataModified = true;
    return true;
}

bool PadelDataManager::cancelBooking(int bookingId, QString& errorMessage) {
    bool locked = mutex.tryLock(200);

    if (!locked) {
        errorMessage = "System busy, please try again";
        return false;
    }

    bool result = false;
    int courtId = -1;
    QDateTime startTime, endTime;

    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
        errorMessage = "Booking not found";
        mutex.unlock();
        return false;
    }

    if (it->second.isCancelled()) {
        errorMessage = "Booking is already cancelled";
        mutex.unlock();
        return false;
    }

    QDateTime now = QDateTime::currentDateTime();
    const Booking& booking = it->second;
    bool canCancel = now.secsTo(booking.getStartTime()) >= 3 * 60 * 60;

    if (!canCancel) {
        errorMessage = "Cancellation is not allowed within 3 hours of booking time";
        mutex.unlock();
        return false;
    }

    courtId = booking.getCourtId();
    startTime = booking.getStartTime();
    endTime = booking.getEndTime();

    Booking& bookingRef = it->second;
    bookingRef.cancel();

    int memberId = bookingRef.getUserId();

    dataModified = true;
    result = true;

    QMetaObject::invokeMethod(this, "safeEmitBookingCancelled",
                          Qt::QueuedConnection,
                          Q_ARG(int, bookingId),
                          Q_ARG(int, memberId));

    mutex.unlock();

    if (result && courtId > 0 && startTime.isValid() && endTime.isValid()) {
        QString waitlistError;
        if (tryFillSlotFromWaitlist(courtId, startTime, endTime, waitlistError)) {
        }
    }

    return result;
}

void PadelDataManager::safeEmitBookingCancelled(int bookingId, int userId) {
    emit bookingCancelled(bookingId, userId);
}

bool PadelDataManager::rescheduleBooking(int bookingId, const QDateTime& newStartTime,
                                       const QDateTime& newEndTime, QString& errorMessage) {
    bool locked = mutex.tryLock(200);

    if (!locked) {
        errorMessage = "System busy, please try again";
        return false;
    }

    bool result = false;
    int courtId = -1;
    QDateTime oldStartTime, oldEndTime;

    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
        errorMessage = "Booking not found";
        mutex.unlock();
        return false;
    }

    Booking& booking = it->second;
    QDateTime now = QDateTime::currentDateTime();

    courtId = booking.getCourtId();
    oldStartTime = booking.getStartTime();
    oldEndTime = booking.getEndTime();

    if (now.secsTo(booking.getStartTime()) < 3 * 60 * 60) {
        errorMessage = "Cannot reschedule booking less than 3 hours before start time";
        mutex.unlock();
        return false;
    }

    if (!validateBookingTime(newStartTime, newEndTime, errorMessage)) {
        mutex.unlock();
        return false;
    }

    bool available = true;
    for (const auto& pair : bookingsById) {
        const Booking& existingBooking = pair.second;

        if (existingBooking.isCancelled() || existingBooking.getBookingId() == bookingId) {
            continue;
        }

        if (existingBooking.getCourtId() == courtId) {
            if (!(newEndTime <= existingBooking.getStartTime() ||
                  newStartTime >= existingBooking.getEndTime())) {
                available = false;
                break;
            }
        }
    }

    if (!available) {
        errorMessage = "Court is not available at the requested time";
        mutex.unlock();
        return false;
    }

    booking.setStartTime(newStartTime);
    booking.setEndTime(newEndTime);
    dataModified = true;
    result = true;

    emit bookingRescheduled(bookingId, booking.getUser().getId());

    mutex.unlock();

    if (result && courtId > 0 && oldStartTime.isValid() && oldEndTime.isValid()) {
        QString waitlistError;
        tryFillSlotFromWaitlist(courtId, oldStartTime, oldEndTime, waitlistError);
    }

    return result;
}

QVector<Booking> PadelDataManager::getBookingsByMember(int memberId) const {
    QMutexLocker locker(&mutex);

    QVector<Booking> result;
    for (const auto& pair : bookingsById) {
        if (pair.second.getMemberId() == memberId) {
            result.append(pair.second);
        }
    }
    return result;
}

QVector<Booking> PadelDataManager::getBookingsByCourt(int courtId) const {
    QMutexLocker locker(&mutex);

    QVector<Booking> result;
    for (const auto& pair : bookingsById) {
        if (pair.second.getCourt().getId() == courtId) {
            result.append(pair.second);
        }
    }
    return result;
}

QVector<Booking> PadelDataManager::getBookingsByDate(const QDate& date) const {
    QMutexLocker locker(&mutex);

    QVector<Booking> result;
    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;
        if (booking.getStartTime().date() == date) {
            result.append(booking);
        }
    }
    return result;
}

QVector<Booking> PadelDataManager::getBookingsForTimeSlot(int courtId, const QDateTime& startTime, const QDateTime& endTime) {
    QMutexLocker locker(&mutex);
    QVector<Booking> results;

    for (const auto& pair : bookingsById) {
        const Booking& b = pair.second;
        if (!b.isCancelled() &&
            b.getCourtId() == courtId &&
            b.getStartTime() < endTime &&
            b.getEndTime() > startTime) {
            results.append(b);
            }
    }
    return results;
}

bool PadelDataManager::isCourtAvailable(int courtId, const QDateTime& startTime,
                                      const QDateTime& endTime) const {
    QMutexLocker locker(&mutex);

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        return false;
    }

    const Court& court = courtIt->second;
    int maxAttendees = court.getMaxAttendees();

    if (!startTime.isValid() || !endTime.isValid() || startTime >= endTime) {
        return false;
    }

    int currentBookings = 0;

    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;

        if (booking.isCancelled()) {
            continue;
        }

        if (booking.getCourtId() != courtId) {
            continue;
        }

        QDate bookingDate = booking.getStartTime().date();
        QDate requestDate = startTime.date();

        if (!bookingDate.isValid() || !requestDate.isValid() || bookingDate != requestDate) {
            continue;
        }

        bool overlap = (booking.getStartTime() <= endTime && booking.getEndTime() >= startTime);

        if (overlap) {
            currentBookings++;
        }
    }

    return (currentBookings < maxAttendees);
}

bool PadelDataManager::validateCourtAvailability(int courtId, const QDateTime& startTime,
                                              const QDateTime& endTime) const {
    return isCourtAvailable(courtId, startTime, endTime);
}

bool PadelDataManager::addToWaitlist(int userId, int courtId, const QDateTime& requestedTime,
                                   QString& errorMessage) {
    if (userId <= 0 || courtId <= 0) {
        errorMessage = "Invalid user or court ID.";
        return false;
    }

    if (courtsById.find(courtId) == courtsById.end()) {
        errorMessage = "Court not found.";
        return false;
    }

    bool isVIP = false;
    int priority = 0;

    if (memberDataManager) {
        bool isMember = memberDataManager->userIsMember(userId);
        if (isMember) {
            int memberId = memberDataManager->getMemberIdByUserId(userId);
            isVIP = memberDataManager->isVIPMember(memberId);
            
            if (isVIP) {
                priority = 100;
            } else {
                priority = 50;
            }
        } else {
            priority = 10;
        }
    }

    WaitlistEntry entry;
    entry.memberId = userId;
    entry.courtId = courtId;
    entry.requestedTime = requestedTime;
    entry.isVIP = isVIP;
    entry.priority = priority;

    if (courtWaitlists.find(courtId) == courtWaitlists.end()) {
        courtWaitlists[courtId] = std::queue<WaitlistEntry>();
    }

    courtWaitlists[courtId].push(entry);
    dataModified = true;

    saveToFile();
    updateWaitlistPositionsAndNotify(courtId);

    return true;
}

bool PadelDataManager::removeFromWaitlist(int userId, int courtId, QString& errorMessage) {
    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end()) {
        errorMessage = "No waitlist found for this court";
        return false;
    }

    std::queue<WaitlistEntry>& waitlist = it->second;
    std::queue<WaitlistEntry> newWaitlist;

    bool found = false;
    while (!waitlist.empty()) {
        WaitlistEntry entry = waitlist.front();
        waitlist.pop();

        if (entry.memberId != userId) {
            newWaitlist.push(entry);
        } else {
            found = true;
        }
    }

    if (!found) {
        errorMessage = "User not found in waitlist";
        return false;
    }

    courtWaitlists[courtId] = newWaitlist;
    dataModified = true;

    if (newWaitlist.empty()) {
        courtWaitlists.erase(courtId);
    }

    saveToFile();
    emit waitlistUpdated(courtId);

    return true;
}

QVector<WaitlistEntry> PadelDataManager::getWaitlistForCourt(int courtId) const {

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end()) {
        return QVector<WaitlistEntry>();
    }

    QVector<WaitlistEntry> result;
    std::queue<WaitlistEntry> tempQueue = it->second;

    while (!tempQueue.empty()) {
        result.append(tempQueue.front());
        tempQueue.pop();
    }

    return result;
}

bool PadelDataManager::processWaitlist(int courtId, QString& errorMessage) {
    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end() || it->second.empty()) {
        errorMessage = "No waitlist entries for this court";
        return false;
    }

    WaitlistEntry entry = it->second.front();
    it->second.pop();

    int userId = entry.memberId;

    if (isCourtAvailable(courtId, entry.requestedTime, entry.requestedTime.addSecs(3600))) {
        QString bookingError;
        bool success = createBooking(userId, courtId,
                               entry.requestedTime,
                               entry.requestedTime.addSecs(3600),
                                   bookingError,
                                   true);

        if (success) {
            if (it->second.empty()) {
                courtWaitlists.erase(it);
            }

            dataModified = true;
            saveToFile();

            return true;
        } else {
            errorMessage = "Failed to create booking: " + bookingError;
            it->second.push(entry);
            return false;
        }
    }

    it->second.push(entry);
    errorMessage = "Court not available at the requested time";
    return false;
}

void PadelDataManager::setVIPPriority(int memberId, bool isVIP) {
    vipMembers[memberId] = isVIP;
    dataModified = true;
    emit vipStatusChanged(memberId, isVIP);
}

bool PadelDataManager::isVIPMember(int memberId) const {
    if (memberId <= 0) {
        return false;
    }

    auto it = vipMembers.find(memberId);
    bool isVip = (it != vipMembers.end() && it->second);
    return isVip;
}

int PadelDataManager::calculatePriority(int memberId) const {
    int priority = 0;
    if (isVIPMember(memberId)) {
        priority += 100;
    }

    return priority;
}

bool PadelDataManager::addTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage) {

    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        errorMessage = "Invalid court ID";
        return false;
    }

    Court& court = it->second;
    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();

    for (const QTime& slot : timeSlots) {
        if (slot == timeSlot) {
            errorMessage = "Time slot already exists";
            return false;
        }
    }

    court.getAllTimeSlots().push_back(timeSlot);
    dataModified = true;
    return true;
}

bool PadelDataManager::removeTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage) {

    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        errorMessage = "Invalid court ID";
        return false;
    }

    Court& court = it->second;
    std::vector<QTime>& timeSlots = court.getAllTimeSlots();

    auto slotIt = std::find(timeSlots.begin(), timeSlots.end(), timeSlot);
    if (slotIt == timeSlots.end()) {
        errorMessage = "Time slot not found";
        return false;
    }

    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;
        if (booking.getCourt().getId() == courtId &&
            booking.getStartTime().time() == timeSlot) {
            errorMessage = "Cannot remove time slot with existing bookings";
            return false;
        }
    }

    timeSlots.erase(slotIt);
    dataModified = true;
    return true;
}

QVector<WaitlistEntry> PadelDataManager::getWaitlistForMember(int memberId) const {

    QVector<WaitlistEntry> result;

    for (const auto& pair : courtWaitlists) {
        int courtId = pair.first;
        std::queue<WaitlistEntry> tempQueue = pair.second;

        while (!tempQueue.empty()) {
            WaitlistEntry entry = tempQueue.front();
            tempQueue.pop();

            if (entry.memberId == memberId) {

                entry.courtId = courtId;
                result.append(entry);
            }
        }
    }

    return result;
}

int PadelDataManager::getWaitlistPosition(int userId, int courtId) const {
    if (userId <= 0 || courtId <= 0) {
        return -1;
    }

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        return -1;
    }

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end()) {
        return -1;
    }

    if (it->second.empty()) {
        return -1;
    }

    QVector<WaitlistEntry> entries;
    std::queue<WaitlistEntry> tempQueue = it->second;

    while (!tempQueue.empty()) {
        entries.append(tempQueue.front());
        tempQueue.pop();
    }

    if (entries.isEmpty()) {
        return -1;
    }

    std::sort(entries.begin(), entries.end(), [](const WaitlistEntry& a, const WaitlistEntry& b) {
        if (a.isVIP != b.isVIP) {
            return a.isVIP;
        }
        return a.priority > b.priority;
    });

    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].memberId == userId) {
            int position = i + 1;
            return position;
        }
    }

    return -1;
}

void PadelDataManager::removeUserFromAllWaitlists(int userId, int courtId, const QDate& date) {
    if (userId <= 0 || courtId <= 0 || !date.isValid()) {
        return;
    }

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        return;
    }

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end() || it->second.empty()) {
        return;
    }

    std::queue<WaitlistEntry> newQueue;
    std::queue<WaitlistEntry> tempQueue = it->second;

    if (tempQueue.empty()) {
        return;
    }

    while (!tempQueue.empty()) {
        WaitlistEntry entry = tempQueue.front();
        tempQueue.pop();

        if (entry.memberId != userId || entry.courtId != courtId || entry.requestedTime.date() != date) {
            newQueue.push(entry);
        }
    }

    if (newQueue.empty()) {
        courtWaitlists.erase(courtId);
    } else {
        courtWaitlists[courtId] = newQueue;
    }

    emit waitlistUpdated(courtId);
    dataModified = true;
}

void PadelDataManager::updateWaitlistPositionsAndNotify(int courtId) {
    if (courtId <= 0) {
        return;
    }

    if (courtsById.find(courtId) == courtsById.end()) {
        return;
    }

    auto waitlistIt = courtWaitlists.find(courtId);
    if (waitlistIt == courtWaitlists.end()) {
        return;
    }

    if (waitlistIt->second.empty()) {
        return;
    }

    QVector<WaitlistEntry> entries;
    std::queue<WaitlistEntry> tempQueue = waitlistIt->second;
    
    while (!tempQueue.empty()) {
        entries.append(tempQueue.front());
        tempQueue.pop();
    }

    dataModified = true;

    if (entries.isEmpty()) {
        return;
    }

    std::sort(entries.begin(), entries.end(), [](const WaitlistEntry& a, const WaitlistEntry& b) {
        if (a.isVIP != b.isVIP) {
            return a.isVIP > b.isVIP;
        }
        return a.priority > b.priority;
    });

    for (int i = 0; i < entries.size(); i++) {
        int userId = entries[i].memberId;
        int position = i + 1;
        emit waitlistPositionChanged(userId, courtId, position);
    }
}

bool PadelDataManager::userHasBookingAtTime(int userId, int courtId, const QDate& date, const QTime& timeSlot) const {
    if (userId <= 0 || courtId <= 0 || !date.isValid() || !timeSlot.isValid()) {
        return false;
    }

    QMutexLocker locker(&mutex);

    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;

        if (booking.isCancelled()) {
            continue;
        }

        if (booking.getUserId() == userId &&
            booking.getCourtId() == courtId &&
            booking.getStartTime().date() == date) {

            if (booking.getStartTime().time() == timeSlot) {
                return true;
            }
        }
    }

    return false;
}

bool PadelDataManager::userHasBookingOnDate(int userId, int courtId, const QDate& date) const {
    if (userId <= 0 || !date.isValid()) {
        return false;
    }

    QMutexLocker locker(&mutex);

    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;

        if (booking.isCancelled()) {
            continue;
        }

        if (booking.getUserId() == userId &&
            (courtId <= 0 || booking.getCourtId() == courtId) &&
            booking.getStartTime().date() == date) {
            return true;
        }
    }

    return false;
}

QVector<Booking> PadelDataManager::getUserAutoBookings(int userId) const {
    QVector<Booking> result;
    QMutexLocker locker(&mutex);

    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;

        if (booking.isCancelled()) {
            continue;
        }

        if (booking.getUserId() == userId && booking.isFromWaitlist()) {
            result.append(booking);
        }
    }

    return result;
}

bool PadelDataManager::isUserInWaitlist(int userId, int courtId, const QDateTime& requestedTime) const {
    if (userId <= 0 || courtId <= 0 || !requestedTime.isValid()) {
        return false;
    }

    bool locked = mutex.tryLock(100);
    if (!locked) {
        return false;
    }

    struct ScopedUnlock {
        QMutex& m;
        ScopedUnlock(QMutex& mutex) : m(mutex) {}
        ~ScopedUnlock() { m.unlock(); }
    } unlocker(mutex);

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        return false;
    }

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end()) {
        return false;
    }

    if (it->second.empty()) {
        return false;
    }

    std::queue<WaitlistEntry> tempQueue = it->second;

    if (tempQueue.empty()) {
        return false;
    }

    while (!tempQueue.empty()) {
        WaitlistEntry entry = tempQueue.front();
        tempQueue.pop();

        if (entry.memberId != userId) continue;

        if (!entry.requestedTime.isValid() ||
            !requestedTime.isValid() ||
            entry.requestedTime.date() != requestedTime.date()) {
            continue;
        }

        if (requestedTime.time().isValid() && entry.requestedTime.time().isValid()) {
            int hourDiff = abs(entry.requestedTime.time().hour() - requestedTime.time().hour());
            if (hourDiff <= 1) {
                return true;
            }
        } else {
            return true;
        }
    }

    return false;
}

QJsonObject PadelDataManager::getDetailedWaitlistInfo(int courtId, const QDate& date) const {
    QJsonObject result;

    if (courtId <= 0) {
        result["error"] = "Invalid court ID";
        result["waitlistCount"] = 0;
        result["entries"] = QJsonArray();
        return result;
    }

    QVector<WaitlistEntry> entries;
    QVector<User> users;

    if (courtsById.find(courtId) == courtsById.end()) {
        result["error"] = "Court not found";
        result["waitlistCount"] = 0;
        result["entries"] = QJsonArray();
        return result;
    }

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end() || it->second.empty()) {
        result["waitlistCount"] = 0;
        result["entries"] = QJsonArray();
        return result;
    }

    std::queue<WaitlistEntry> tempQueue = it->second;
    while (!tempQueue.empty()) {
        WaitlistEntry entry = tempQueue.front();
        tempQueue.pop();

        if (entry.requestedTime.date() == date) {
            entries.append(entry);

            if (memberDataManager) {
                UserDataManager* userManager = memberDataManager->getUserDataManager();
                if (userManager) {
                    User user = userManager->getUserDataById(entry.memberId);
                    if (user.getId() > 0) {
                        users.append(user);
                    }
                }
            }
        }
    }

    QVector<int> indices;
    for (int i = 0; i < entries.size(); i++) {
        indices.append(i);
    }

    std::sort(indices.begin(), indices.end(), [&entries](int a, int b) {
        const WaitlistEntry& entryA = entries[a];
        const WaitlistEntry& entryB = entries[b];

        if (entryA.isVIP != entryB.isVIP) {
            return entryA.isVIP;
        }

        if (entryA.priority != entryB.priority) {
            return entryA.priority > entryB.priority;
        }

        return entryA.requestedTime < entryB.requestedTime;
    });

    result["waitlistCount"] = entries.size();

    QJsonArray entriesArray;
    for (int i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        const WaitlistEntry& entry = entries[idx];

        QJsonObject entryObj;
        entryObj["position"] = i + 1;
        entryObj["userId"] = entry.memberId;
        entryObj["isVIP"] = entry.isVIP;
        entryObj["priority"] = entry.priority;
        entryObj["requestedTime"] = entry.requestedTime.toString("HH:mm");

        if (idx < users.size() && users[idx].getId() > 0) {
            entryObj["userName"] = users[idx].getName();
            entryObj["photoUrl"] = users[idx].getUserPhotoPath();
            entryObj["email"] = users[idx].getEmail();
        }

        entriesArray.append(entryObj);
    }

    result["entries"] = entriesArray;
    return result;
}

QVector<WaitlistEntry> PadelDataManager::getWaitlistForUser(int userId) const {
    QMutexLocker locker(&mutex);

    QVector<WaitlistEntry> result;

    for (const auto& pair : courtWaitlists) {
        int courtId = pair.first;
        std::queue<WaitlistEntry> tempQueue = pair.second;

        while (!tempQueue.empty()) {
            WaitlistEntry entry = tempQueue.front();
            tempQueue.pop();

            if (entry.memberId == userId) {

                entry.courtId = courtId;
                result.append(entry);
            }
        }
    }

    return result;
}

QVector<QTime> PadelDataManager::getAllTimeSlots(int courtId) const {
    QMutexLocker locker(&mutex);

    if (courtId <= 0) {
        return QVector<QTime>();
    }

    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        return QVector<QTime>();
    }

    const Court& court = it->second;
    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();

    QVector<QTime> result;
    result.reserve(timeSlots.size());

    for (const QTime& slot : timeSlots) {
        if (slot.isValid()) {
            result.append(slot);
        }
    }

    return result;
}

QJsonArray PadelDataManager::getAllTimeSlotsJson(int courtId) const {
    QJsonArray timeSlotsArray;

    if (courtId <= 0) {
        return timeSlotsArray;
    }

    Court court = getCourtById(courtId);
    if (court.getId() <= 0) {
        return timeSlotsArray;
    }

    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();

    for (const QTime& time : timeSlots) {
        if (time.isValid()) {
            QJsonObject slotObject;
            slotObject["startTime"] = time.toString("HH:mm");

            QTime endTime = time.addSecs(3600);
            slotObject["endTime"] = endTime.toString("HH:mm");

            timeSlotsArray.append(slotObject);
        }
    }

    return timeSlotsArray;
}

QJsonArray PadelDataManager::waitlistsToJson() const {
    QJsonArray waitlistsArray;

    for (const auto& pair : courtWaitlists) {
        int courtId = pair.first;
        std::queue<WaitlistEntry> tempQueue = pair.second;

        while (!tempQueue.empty()) {
            WaitlistEntry entry = tempQueue.front();
            tempQueue.pop();

            QJsonObject entryObj;
            entryObj["userId"] = entry.memberId;
            entryObj["courtId"] = courtId;
            entryObj["requestedTime"] = entry.requestedTime.toString(Qt::ISODate);
            entryObj["isVIP"] = entry.isVIP;
            entryObj["priority"] = entry.priority;

            waitlistsArray.append(entryObj);
        }
    }

    return waitlistsArray;
}

QJsonArray PadelDataManager::readWaitlistsFromFile(QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("waitlists.json"));
    if (!file.exists()) {
        return QJsonArray();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open waitlists file for reading";
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing waitlists file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}

bool PadelDataManager::writeWaitlistsToFile(const QJsonArray& waitlists, QString& errorMessage) const {
    QString filePath = QDir(dataDir).filePath("waitlists.json");
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.absolutePath());

    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open waitlists file for writing: " + file.errorString();
        return false;
    }

    QJsonDocument doc(waitlists);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten == -1) {
        errorMessage = "Failed to write to waitlists file: " + file.errorString();
        file.close();
        return false;
    }

    file.flush();
    file.close();
    return true;
}

bool PadelDataManager::tryFillSlotFromWaitlist(int courtId, const QDateTime& startTime, const QDateTime& endTime, QString& errorMessage) {
    if (courtId <= 0 || !startTime.isValid() || !endTime.isValid()) {
        errorMessage = "Invalid parameters in tryFillSlotFromWaitlist";
        return false;
    }

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        errorMessage = "Court not found in tryFillSlotFromWaitlist";
        return false;
    }

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end() || it->second.empty()) {
        return false;
    }

    QVector<WaitlistEntry> entries;
    std::queue<WaitlistEntry> tempQueue = it->second;

    while (!tempQueue.empty()) {
        entries.append(tempQueue.front());
        tempQueue.pop();
    }

    if (entries.isEmpty()) {
        return false;
    }

    std::sort(entries.begin(), entries.end(), [](const WaitlistEntry& a, const WaitlistEntry& b) {
        if (a.isVIP != b.isVIP) {
            return a.isVIP > b.isVIP;
        }
        return a.priority > b.priority;
    });

    for (const WaitlistEntry& entry : entries) {
        if (entry.requestedTime.date() != startTime.date()) {
            continue;
        }

        double hourDiff = std::abs(entry.requestedTime.secsTo(startTime)) / 3600.0;
        if (hourDiff > 3.0) {
            continue;
        }

        int userId = entry.memberId;

        if (userHasBookingOnDate(userId, courtId, startTime.date())) {
            continue;
        }

        QString bookingError;
        bool success = createBooking(userId, courtId,
                                  startTime,
                                  endTime,
                                  bookingError,
                                  true);

        if (success) {
            QString removeError;
            removeFromWaitlist(userId, courtId, removeError);
            emit waitlistBookingCreated(userId, courtId, startTime);
            return true;
        }
    }

    return false;
}

QJsonObject PadelDataManager::getCourtDetails(int courtId) const {
    QJsonObject courtDetails;

    if (courtId <= 0) {
        return courtDetails;
    }

    Court court = getCourtById(courtId);
    if (court.getId() <= 0) {
        return courtDetails;
    }

    courtDetails = courtToJson(court);
    courtDetails["availableTimeSlots"] = static_cast<int>(court.getAllTimeSlots().size());

    return courtDetails;
}

void PadelDataManager::setMemberDataManager(MemberDataManager* memberManager) {
    memberDataManager = memberManager;
}

QVector<Court> PadelDataManager::getAvailableCourts(const QDateTime& startTime, const QDateTime& endTime,
                                                  const QString& location) const {
    QMutexLocker locker(&mutex);
    QVector<Court> availableCourts;

    for (const auto& pair : courtsById) {
        const Court& court = pair.second;

        if (!location.isEmpty() && court.getLocation() != location) {
            continue;
        }

        if (isCourtAvailable(court.getId(), startTime, endTime)) {
            availableCourts.append(court);
        }
    }

    return availableCourts;
}

int PadelDataManager::getCurrentAttendees(int courtId, const QDateTime& startTime, const QDateTime& endTime) const {
    QMutexLocker locker(&mutex);
    int attendeeCount = 0;

    if (courtId <= 0 || !startTime.isValid() || !endTime.isValid()) {
        return 0;
    }

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        return 0;
    }

    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;

        if (booking.isCancelled()) {
            continue;
        }

        if (booking.getCourtId() != courtId) {
            continue;
        }

        QDateTime bookingStart = booking.getStartTime();
        QDateTime bookingEnd = booking.getEndTime();

        bool overlaps = (bookingStart < endTime && startTime < bookingEnd);

        if (overlaps) {
            attendeeCount++;
        }
    }

    return attendeeCount;
}

QVector<Booking> PadelDataManager::getAllBookings() const {
    QMutexLocker locker(&mutex);
    QVector<Booking> allBookings;

    allBookings.reserve(bookingsById.size());
    for (const auto& pair : bookingsById) {
        allBookings.append(pair.second);
    }

    return allBookings;
}

QJsonArray PadelDataManager::getAvailableTimeSlots(int courtId, const QDate& date, int maxAttendees) const {
    bool locked = mutex.tryLock(200);
    if (!locked) {
        return QJsonArray();
    }

    QJsonArray availableSlots;

    if (courtId <= 0) {
        mutex.unlock();
        return availableSlots;
    }

    if (!date.isValid()) {
        mutex.unlock();
        return availableSlots;
    }

    int safeMaxAttendees = maxAttendees;
    if (safeMaxAttendees <= 0) {
        safeMaxAttendees = 2;
    }

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        mutex.unlock();
        return availableSlots;
    }

    Court court = courtIt->second;
    mutex.unlock();

    std::vector<QTime> timeSlotsCopy = court.getAllTimeSlots();
    if (timeSlotsCopy.empty()) {
        return availableSlots;
    }

    QDate currentDate = QDate::currentDate();
    QTime currentTime = QTime::currentTime();
    bool isToday = (date == currentDate);

    for (const QTime& time : timeSlotsCopy) {
        if (!time.isValid()) {
            continue;
        }

        if (isToday && time <= currentTime) {
            continue;
        }

        QDateTime startTime(date, time);
        QDateTime endTime = startTime.addSecs(3600);

        int attendees = getCurrentAttendees(courtId, startTime, endTime);

        if (attendees < safeMaxAttendees) {
            QJsonObject slotObj;
            slotObj["startTime"] = time.toString("HH:mm");
            slotObj["endTime"] = endTime.time().toString("HH:mm");
            slotObj["currentAttendees"] = attendees;
            slotObj["maxAttendees"] = safeMaxAttendees;
            slotObj["availableSpots"] = safeMaxAttendees - attendees;

            availableSlots.append(slotObj);
        }
    }

    return availableSlots;
}

QJsonArray PadelDataManager::readCourtsFromFile(QString& errorMessage) const {
    QJsonArray courtsArray;
    QString filePath = dataDir + "/courts.json";

    QFile file(filePath);
    if (!file.exists()) {
        errorMessage = "Courts file not found at: " + filePath;
        return courtsArray;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Failed to open courts file: " + file.errorString();
        return courtsArray;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Failed to parse courts file: " + parseError.errorString();
        return courtsArray;
    }

    if (!doc.isArray()) {
        errorMessage = "Courts file does not contain a valid JSON array";
        return courtsArray;
    }

    courtsArray = doc.array();
    return courtsArray;
}

QJsonArray PadelDataManager::readBookingsFromFile(QString& errorMessage) const {
    QJsonArray bookingsArray;
    QString filePath = dataDir + "/bookings.json";

    QFile file(filePath);
    if (!file.exists()) {
        errorMessage = "Bookings file not found at: " + filePath;
        return bookingsArray;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Failed to open bookings file: " + file.errorString();
        return bookingsArray;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Failed to parse bookings file: " + parseError.errorString();
        return bookingsArray;
    }

    if (!doc.isArray()) {
        errorMessage = "Bookings file does not contain a valid JSON array";
        return bookingsArray;
    }

    bookingsArray = doc.array();
    return bookingsArray;
}

Court PadelDataManager::jsonToCourt(const QJsonObject& json) {
    Court court;

    if (!json.contains("id") || !json.contains("name") || !json.contains("location")) {
        return court;
    }

    int id = json["id"].toInt();
    QString name = json["name"].toString();
    QString location = json["location"].toString();

    court.setId(id);
    court.setName(name);
    court.setLocation(location);

    if (json.contains("pricePerHour")) {
        double price = json["pricePerHour"].toDouble();
        court.setPricePerHour(price);
    }

    if (json.contains("description")) {
        QString description = json["description"].toString();
        court.setDescription(description);
    }

    if (json.contains("maxAttendees")) {
        int maxAttendees = json["maxAttendees"].toInt();
        if (maxAttendees > 0) {
            court.setMaxAttendees(maxAttendees);
        } else {
            court.setMaxAttendees(2);
        }
    } else {
        court.setMaxAttendees(2);
    }

    if (json.contains("features") && json["features"].isArray()) {
        QJsonArray featuresArray = json["features"].toArray();
        QStringList features;

        for (const QJsonValue& featureValue : featuresArray) {
            if (featureValue.isString()) {
                features.append(featureValue.toString());
            }
        }

        court.setFeatures(features);
    }

    if (json.contains("timeSlots") && json["timeSlots"].isArray()) {
        QJsonArray timeSlotsArray = json["timeSlots"].toArray();

        std::vector<QTime>& timeSlots = court.getAllTimeSlots();

        timeSlots.clear();

        for (const QJsonValue& slotValue : timeSlotsArray) {
            if (slotValue.isString()) {
                QTime time = QTime::fromString(slotValue.toString(), "HH:mm");
                if (time.isValid()) {
                    timeSlots.push_back(time);
                }
            }
        }

        if (timeSlots.empty()) {
            court.initializeDefaultTimeSlots();
        }
    } else {
        court.initializeDefaultTimeSlots();
    }

    return court;
}

int PadelDataManager::generateCourtId() const {
    int maxId = 0;

    for (const auto& pair : courtsById) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }

    return maxId + 1;
}

int PadelDataManager::generateBookingId() const {
    int maxId = 0;

    for (const auto& pair : bookingsById) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }

    return maxId + 1;
}

void PadelDataManager::setupTimers() {
    QTimer* statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &PadelDataManager::checkBookingStatus);
    statusTimer->start(300000);
}

bool PadelDataManager::validateBookingTime(const QDateTime& startTime, const QDateTime& endTime,
                                         QString& errorMessage) const {

    if (!startTime.isValid() || !endTime.isValid()) {
        errorMessage = "Invalid booking time specified";
        return false;
    }

    if (startTime >= endTime) {
        errorMessage = "End time must be after start time";
        return false;
    }

    QDateTime now = QDateTime::currentDateTime();

    if (startTime < now) {
        errorMessage = "Booking time must be in the future";
        return false;
    }

    QDateTime maxFutureDate = now.addMonths(3);
    if (startTime > maxFutureDate) {
        errorMessage = "Booking cannot be more than 3 months in advance";
        return false;
    }

    int durationMinutes = startTime.secsTo(endTime) / 60;
    if (durationMinutes != 60) {
        errorMessage = "Booking duration must be exactly 1 hour";
        return false;
    }

    int startMinute = startTime.time().minute();
    if (startMinute != 0 && startMinute != 30) {
        errorMessage = "Booking must start on the hour or half hour";
        return false;
    }

    return true;
}

void PadelDataManager::checkBookingStatus() {
    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    QVector<int> processedCourts;

    for (const auto& courtEntry : courtsById) {
        int courtId = courtEntry.first;
        
        if (courtId <= 0) continue;
        
        processWaitlistForDate(courtId, today);
        processWaitlistForDate(courtId, today.addDays(1));
        
        processedCourts.append(courtId);
    }

    if (dataModified) {
        saveToFile();
    }
}

void PadelDataManager::processWaitlistForDate(int courtId, const QDate& date) {
    QVector<QTime> timeSlots = getAllTimeSlots(courtId);
    if (timeSlots.isEmpty()) {
        return;
    }

    for (const QTime& time : timeSlots) {
        QDateTime startTime(date, time);
        QDateTime endTime = startTime.addSecs(3600);

        if (startTime < QDateTime::currentDateTime()) {
            continue;
        }

        if (isCourtAvailable(courtId, startTime, endTime)) {
            QString error;
            tryFillSlotFromWaitlist(courtId, startTime, endTime, error);
        }
    }
}

Court PadelDataManager::findClosestAvailableCourt(int originalCourtId, const QDateTime& startTime, const QDateTime& endTime) const {
    if (originalCourtId <= 0 || !startTime.isValid() || !endTime.isValid()) {
        return Court();
    }

    auto originalCourtIt = courtsById.find(originalCourtId);
    if (originalCourtIt == courtsById.end()) {
        return Court();
    }

    const Court& originalCourt = originalCourtIt->second;
    QString originalLocation = originalCourt.getLocation();

    QVector<Court> availableCourts = getAvailableCourts(startTime, endTime, originalLocation);
    
    for (const Court& court : availableCourts) {
        if (court.getId() != originalCourtId) {
            return court;
        }
    }

    if (availableCourts.isEmpty()) {
        availableCourts = getAvailableCourts(startTime, endTime);
        
        for (const Court& court : availableCourts) {
            if (court.getId() != originalCourtId) {
                return court;
            }
        }
    }

    return Court();
}