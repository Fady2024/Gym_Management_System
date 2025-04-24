#include "padeldatamanager.h"
#include <QObject>
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
#include <algorithm>
#include <QDateTime>

PadelDataManager::PadelDataManager(QObject* parent)
    : QObject(parent), memberDataManager(nullptr) {
    // Get the project directory path
    QString projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
    
    // Set data directory path
    dataDir = projectDir + "/project code/Data";
    
    // Create directory if it doesn't exist
    QDir().mkpath(dataDir);
    
    // Initialize empty courts.json if it doesn't exist
    QFile courtsFile(dataDir + "/courts.json");
    if (!courtsFile.exists()) {
        courtsFile.open(QIODevice::WriteOnly);
        courtsFile.write("[]");
        courtsFile.close();
    }
    
    // Initialize empty bookings.json if it doesn't exist
    QFile bookingsFile(dataDir + "/bookings.json");
    if (!bookingsFile.exists()) {
        bookingsFile.open(QIODevice::WriteOnly);
        bookingsFile.write("[]");
        bookingsFile.close();
    }
    
    // Initialize data from file
    if (!initializeFromFile()) {
        qDebug() << "Failed to initialize padel data from file";
    }
    
    // Set up timers
    setupTimers();
}

PadelDataManager::~PadelDataManager() {
    handleApplicationClosing();
}

void PadelDataManager::handleApplicationClosing() {
    if (dataModified) {
        qDebug() << "Saving padel data before application closing...";
        if (!saveToFile()) {
            qDebug() << "Failed to save padel data before application closing!";
        } else {
            qDebug() << "Padel data saved successfully before application closing.";
        }
    } else {
        qDebug() << "No changes to padel data, skipping save on application exit";
    }
}

bool PadelDataManager::initializeFromFile() {
    QString errorMessage;
    
    // Read courts
    QJsonArray courtsArray = readCourtsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading courts file:" << errorMessage;
        return false;
    }

    courtsById.clear();
    for (const QJsonValue& courtValue : courtsArray) {
        Court court = jsonToCourt(courtValue.toObject());
        courtsById[court.getId()] = court;
    }

    // Read bookings
    QJsonArray bookingsArray = readBookingsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading bookings file:" << errorMessage;
        return false;
    }

    bookingsById.clear();
    for (const QJsonValue& bookingValue : bookingsArray) {
        Booking booking = jsonToBooking(bookingValue.toObject());
        bookingsById[booking.getBookingId()] = booking;
    }

    return true;
}

bool PadelDataManager::saveToFile() {
    QMutexLocker locker(&mutex);
    
    if (!dataModified) {
        return true;  // Nothing to save
    }

    try {
        // Save courts
        QJsonArray courtsArray;
        for (const auto& pair : courtsById) {
            courtsArray.append(courtToJson(pair.second));
        }

        QString errorMessage;
        bool success = writeCourtsToFile(courtsArray, errorMessage);
        if (!success) {
            qDebug() << "Error saving courts file:" << errorMessage;
            return false;
        }

        // Save bookings
        QJsonArray bookingsArray;
        for (const auto& pair : bookingsById) {
            bookingsArray.append(bookingToJson(pair.second));
        }

        success = writeBookingsToFile(bookingsArray, errorMessage);
        if (!success) {
            qDebug() << "Error saving bookings file:" << errorMessage;
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

// Court management methods
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

    // Check if court has any bookings
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
    QMutexLocker locker(&mutex);
    
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

// Booking management methods
bool PadelDataManager::createBooking(int userId, int courtId, const QDateTime& startTime, 
                                   const QDateTime& endTime, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    // Validate court
    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        errorMessage = "Invalid court ID";
        return false;
    }

    // Check if user exists
    if (!memberDataManager || !memberDataManager->getUserDataManager()) {
        errorMessage = "System error: User verification unavailable";
        return false;
    }

    User user = memberDataManager->getUserDataManager()->getUserDataById(userId);
    if (user.getId() <= 0) {
        errorMessage = "Invalid user ID";
        return false;
    }

    // Check if user is a member and/or VIP
    bool isMember = memberDataManager->userIsMember(userId);
    bool isVIP = isMember && memberDataManager->isVIPMember(memberDataManager->getMemberIdByUserId(userId));

    // Validate booking time
    if (!validateBookingTime(startTime, endTime, errorMessage)) {
        return false;
    }

    // Check court availability
    if (!validateCourtAvailability(courtId, startTime, endTime)) {
        // If court is not available and user is VIP, check if we can prioritize
        if (isVIP) {
            // Look for non-VIP bookings that can be moved to waitlist
            for (const auto& pair : bookingsById) {
                const Booking& existingBooking = pair.second;
                if (existingBooking.getCourt().getId() == courtId &&
                    existingBooking.getStartTime() == startTime) {
                    
                    int existingUserId = existingBooking.getUser().getId();
                    bool existingIsVIP = memberDataManager->userIsMember(existingUserId) &&
                                       memberDataManager->isVIPMember(memberDataManager->getMemberIdByUserId(existingUserId));
                    
                    if (!existingIsVIP) {
                        // Move non-VIP booking to waitlist
                        addToWaitlist(existingUserId, courtId, startTime, errorMessage);
                        bookingsById.erase(pair.first);
                        break;
                    }
                }
            }
        }
        
        // If still not available after VIP priority check
        if (!validateCourtAvailability(courtId, startTime, endTime)) {
            // Add to waitlist with appropriate priority
            addToWaitlist(userId, courtId, startTime, errorMessage);
            errorMessage = "Court is not available at the requested time. Added to waitlist.";
            return false;
        }
    }

    // Calculate booking price based on membership status
    double price = calculateBookingPrice(courtId, startTime, endTime, isVIP);
    
    // Create booking
    int bookingId = generateBookingId();
    Booking booking(bookingId, courtIt->second, startTime, endTime, user);
    booking.setPrice(price);
    booking.setVip(isVIP);
    
    bookingsById[bookingId] = booking;
    dataModified = true;
    
    emit bookingCreated(bookingId, userId);
    return true;
}

double PadelDataManager::calculateBookingPrice(int courtId, const QDateTime& startTime, 
                                             const QDateTime& endTime, bool isVIP) const {
    const Court& court = getCourtById(courtId);
    double basePrice = court.getPricePerHour();
    int hours = startTime.secsTo(endTime) / 3600;
    double totalPrice = basePrice * hours;
    
    // Apply member/VIP discounts
    if (isVIP) {
        totalPrice *= 0.8;  // 20% discount for VIP members
    }
    
    return totalPrice;
}

bool PadelDataManager::cancelBooking(int bookingId, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
        errorMessage = "Booking not found";
        return false;
    }

    const Booking& booking = it->second;
    QDateTime now = QDateTime::currentDateTime();
    
    if (now.secsTo(booking.getStartTime()) < 3 * 60 * 60) { // 3 hours
        errorMessage = "Cannot cancel booking less than 3 hours before start time";
        return false;
    }

    int memberId = booking.getUser().getId();
    bookingsById.erase(it);
    dataModified = true;
    
    emit bookingCancelled(bookingId, memberId);
    return true;
}

bool PadelDataManager::rescheduleBooking(int bookingId, const QDateTime& newStartTime, 
                                       const QDateTime& newEndTime, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
        errorMessage = "Booking not found";
        return false;
    }

    Booking& booking = it->second;
    QDateTime now = QDateTime::currentDateTime();
    
    if (now.secsTo(booking.getStartTime()) < 3 * 60 * 60) { // 3 hours
        errorMessage = "Cannot reschedule booking less than 3 hours before start time";
        return false;
    }

    // Validate new time
    if (!validateBookingTime(newStartTime, newEndTime, errorMessage)) {
        return false;
    }

    // Check court availability
    if (!validateCourtAvailability(booking.getCourt().getId(), newStartTime, newEndTime)) {
        errorMessage = "Court is not available at the requested time";
        return false;
    }

    booking.setStartTime(newStartTime);
    booking.setEndTime(newEndTime);
    dataModified = true;
    
    emit bookingRescheduled(bookingId, booking.getUser().getId());
    return true;
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

bool PadelDataManager::isCourtAvailable(int courtId, const QDateTime& startTime, 
                                      const QDateTime& endTime) const {
    QMutexLocker locker(&mutex);
    return validateCourtAvailability(courtId, startTime, endTime);
}

// Waitlist management methods
bool PadelDataManager::addToWaitlist(int memberId, int courtId, const QDateTime& requestedTime, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    // Validate member
    if (!memberDataManager || !memberDataManager->userIsMember(memberId)) {
        errorMessage = "Invalid member ID";
        return false;
    }

    // Validate court
    if (courtsById.find(courtId) == courtsById.end()) {
        errorMessage = "Invalid court ID";
        return false;
    }

    // Create waitlist entry
    WaitlistEntry entry;
    entry.memberId = memberId;
    entry.requestedTime = requestedTime;
    entry.isVIP = memberDataManager->isVIPMember(memberId);
    entry.priority = calculatePriority(memberId);

    // Add to waitlist
    courtWaitlists[courtId].push(entry);
    dataModified = true;
    
    emit waitlistUpdated(courtId);
    return true;
}

bool PadelDataManager::removeFromWaitlist(int memberId, int courtId, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
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
        if (entry.memberId != memberId) {
            newWaitlist.push(entry);
        } else {
            found = true;
        }
    }

    if (!found) {
        errorMessage = "Member not found in waitlist";
        return false;
    }

    courtWaitlists[courtId] = newWaitlist;
    dataModified = true;
    
    emit waitlistUpdated(courtId);
    return true;
}

QVector<WaitlistEntry> PadelDataManager::getWaitlistForCourt(int courtId) const {
    QMutexLocker locker(&mutex);
    
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
    QMutexLocker locker(&mutex);
    
    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end() || it->second.empty()) {
        errorMessage = "No waitlist entries for this court";
        return false;
    }
    
    // Get the next entry from the waitlist
    WaitlistEntry entry = it->second.front();
    it->second.pop();
    
    // Try to create a booking for this entry
    if (isCourtAvailable(courtId, entry.requestedTime, entry.requestedTime.addSecs(3600))) { // 1-hour booking
        QString bookingError;
        bool success = createBooking(entry.memberId, courtId, 
                                   entry.requestedTime, 
                                   entry.requestedTime.addSecs(3600), 
                                   bookingError);
        
        if (success) {
            // Remove empty waitlist
            if (it->second.empty()) {
                courtWaitlists.erase(it);
            }
            dataModified = true;
            return true;
        } else {
            errorMessage = "Failed to create booking: " + bookingError;
            // Put the entry back in the queue
            it->second.push(entry);
            return false;
        }
    }
    
    // Court not available, put the entry back in the queue
    it->second.push(entry);
    errorMessage = "Court not available at the requested time";
    return false;
}

// VIP and priority management
void PadelDataManager::setVIPPriority(int memberId, bool isVIP) {
    QMutexLocker locker(&mutex);
    vipMembers[memberId] = isVIP;
    dataModified = true;
    emit vipStatusChanged(memberId, isVIP);
}

bool PadelDataManager::isVIPMember(int memberId) const {
    QMutexLocker locker(&mutex);
    auto it = vipMembers.find(memberId);
    return it != vipMembers.end() && it->second;
}

int PadelDataManager::calculatePriority(int memberId) const {
    int priority = 0;
    if (isVIPMember(memberId)) {
        priority += 100; // VIP members get higher priority
    }
    
    // Add more priority factors here if needed
    return priority;
}

// Time slot management
bool PadelDataManager::addTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        errorMessage = "Invalid court ID";
        return false;
    }

    Court& court = it->second;
    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();
    
    // Check if time slot already exists
    for (const QTime& slot : timeSlots) {
        if (slot == timeSlot) {
            errorMessage = "Time slot already exists";
            return false;
        }
    }

    // Add time slot
    court.getAllTimeSlots().push_back(timeSlot);
    dataModified = true;
    return true;
}

bool PadelDataManager::removeTimeSlot(int courtId, const QTime& timeSlot, QString& errorMessage) {
    QMutexLocker locker(&mutex);
    
    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        errorMessage = "Invalid court ID";
        return false;
    }

    Court& court = it->second;
    std::vector<QTime>& timeSlots = court.getAllTimeSlots();
    
    // Check if time slot exists
    auto slotIt = std::find(timeSlots.begin(), timeSlots.end(), timeSlot);
    if (slotIt == timeSlots.end()) {
        errorMessage = "Time slot not found";
        return false;
    }

    // Check if time slot has any bookings
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

QVector<QTime> PadelDataManager::getAvailableTimeSlots(int courtId, const QDate& date) const {
    QMutexLocker locker(&mutex);
    
    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {
        return QVector<QTime>();
    }

    const Court& court = it->second;
    const std::vector<QTime>& allTimeSlots = court.getAllTimeSlots();
    QVector<QTime> availableSlots;
    
    for (const QTime& slot : allTimeSlots) {
        QDateTime slotDateTime(date, slot);
        bool isAvailable = true;
        
        for (const auto& pair : bookingsById) {
            const Booking& booking = pair.second;
            if (booking.getCourt().getId() == courtId && 
                booking.getStartTime() == slotDateTime) {
                isAvailable = false;
                break;
            }
        }
        
        if (isAvailable) {
            availableSlots.append(slot);
        }
    }
    
    return availableSlots;
}

// Reporting methods
QJsonObject PadelDataManager::generateMonthlyReport(const QDate& month) const {
    QMutexLocker locker(&mutex);
    
    QJsonObject report;
    int totalBookings = 0;
    int vipBookings = 0;
    double totalRevenue = 0.0;
    
    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;
        if (booking.getStartTime().date().month() == month.month() && 
            booking.getStartTime().date().year() == month.year()) {
            totalBookings++;
            if (isVIPMember(booking.getUser().getId())) {
                vipBookings++;
            }
            totalRevenue += booking.getPrice();
        }
    }
    
    report["totalBookings"] = totalBookings;
    report["vipBookings"] = vipBookings;
    report["totalRevenue"] = totalRevenue;
    
    return report;
}

QJsonObject PadelDataManager::generateCourtUtilizationReport(int courtId, const QDate& startDate, 
                                                           const QDate& endDate) const {
    QMutexLocker locker(&mutex);
    
    QJsonObject report;
    int totalBookings = 0;
    int totalHours = 0;
    
    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;
        if (booking.getCourt().getId() == courtId && 
            booking.getStartTime().date() >= startDate && 
            booking.getStartTime().date() <= endDate) {
            totalBookings++;
            totalHours += booking.getStartTime().secsTo(booking.getEndTime()) / 3600;
        }
    }
    
    report["totalBookings"] = totalBookings;
    report["totalHours"] = totalHours;
    
    return report;
}

// Dependency injection
void PadelDataManager::setMemberDataManager(MemberDataManager* memberManager) {
    memberDataManager = memberManager;
}

// Private helper methods
QJsonArray PadelDataManager::readCourtsFromFile(QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("courts.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open courts file for reading";
        return QJsonArray();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing courts file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}

QJsonArray PadelDataManager::readBookingsFromFile(QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("bookings.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open bookings file for reading";
        return QJsonArray();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing bookings file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}

bool PadelDataManager::writeCourtsToFile(const QJsonArray& courts, QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("courts.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open courts file for writing";
        return false;
    }

    QJsonDocument doc(courts);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool PadelDataManager::writeBookingsToFile(const QJsonArray& bookings, QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("bookings.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open bookings file for writing";
        return false;
    }

    QJsonDocument doc(bookings);
    file.write(doc.toJson());
    file.close();
    return true;
}

QJsonObject PadelDataManager::courtToJson(const Court& court) const {
    QJsonObject json;
    json["id"] = court.getId();
    json["name"] = court.getName();
    json["location"] = court.getLocation();
    
    QJsonArray timeSlotsArray;
    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();
    for (const QTime& slot : timeSlots) {
        timeSlotsArray.append(slot.toString("hh:mm"));
    }
    json["timeSlots"] = timeSlotsArray;
    
    return json;
}

QJsonObject PadelDataManager::bookingToJson(const Booking& booking) const {
    QJsonObject json;
    json["id"] = booking.getBookingId();
    json["courtId"] = booking.getCourt().getId();
    json["memberId"] = booking.getUser().getId();
    json["startTime"] = booking.getStartTime().toString(Qt::ISODate);
    json["endTime"] = booking.getEndTime().toString(Qt::ISODate);
    return json;
}

Court PadelDataManager::jsonToCourt(const QJsonObject& json) {
    Court court;
    court.setId(json["id"].toInt());
    court.setName(json["name"].toString());
    court.setLocation(json["location"].toString());
    
    QJsonArray timeSlotsArray = json["timeSlots"].toArray();
    std::vector<QTime> timeSlots;
    for (const QJsonValue& slot : timeSlotsArray) {
        timeSlots.push_back(QTime::fromString(slot.toString(), "hh:mm"));
    }
    court.getAllTimeSlots() = timeSlots;
    
    return court;
}

Booking PadelDataManager::jsonToBooking(const QJsonObject& json) {
    int bookingId = json["id"].toInt();
    int courtId = json["courtId"].toInt();
    int memberId = json["memberId"].toInt();
    QDateTime startTime = QDateTime::fromString(json["startTime"].toString(), Qt::ISODate);
    QDateTime endTime = QDateTime::fromString(json["endTime"].toString(), Qt::ISODate);
    
    Court court;
    User user;
    return Booking(bookingId, court, startTime, endTime, user);
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

int PadelDataManager::generateCourtId() const {
    int maxId = 0;
    for (const auto& pair : courtsById) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }
    return maxId + 1;
}

void PadelDataManager::checkBookingStatus() {
    QDateTime now = QDateTime::currentDateTime();
    
    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;
        if (now > booking.getEndTime()) {
            // Handle expired bookings if needed
        }
    }
}

void PadelDataManager::setupTimers() {
    // Set up timer to check booking status daily
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PadelDataManager::checkBookingStatus);
    timer->start(24 * 60 * 60 * 1000); // Check every 24 hours
}

bool PadelDataManager::validateBookingTime(const QDateTime& startTime, const QDateTime& endTime, 
                                         QString& errorMessage) const {
    if (!startTime.isValid() || !endTime.isValid()) {
        errorMessage = "Invalid date/time";
        return false;
    }

    if (startTime >= endTime) {
        errorMessage = "End time must be after start time";
        return false;
    }

    if (startTime.date() < QDate::currentDate()) {
        errorMessage = "Cannot book for past dates";
        return false;
    }

    // Check if booking is within operating hours (e.g., 8 AM to 10 PM)
    QTime startHour = startTime.time();
    QTime endHour = endTime.time();
    if (startHour < QTime(8, 0) || endHour > QTime(22, 0)) {
        errorMessage = "Bookings must be between 8 AM and 10 PM";
        return false;
    }

    return true;
}

bool PadelDataManager::validateCourtAvailability(int courtId, const QDateTime& startTime, 
                                               const QDateTime& endTime) const {
    for (const auto& pair : bookingsById) {
        const Booking& booking = pair.second;
        if (booking.getCourt().getId() == courtId) {
            if ((startTime >= booking.getStartTime() && startTime < booking.getEndTime()) ||
                (endTime > booking.getStartTime() && endTime <= booking.getEndTime()) ||
                (startTime <= booking.getStartTime() && endTime >= booking.getEndTime())) {
                return false;
            }
        }
    }
    return true;
} 