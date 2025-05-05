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
#include <QMetaObject>

PadelDataManager::PadelDataManager(QObject* parent)
    : QObject(parent), memberDataManager(nullptr) {

    QString projectDir;
    
#ifdef FORCE_SOURCE_DIR
    // Use the source directory path defined in CMake
    projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
    qDebug() << "Padel - Using source directory path:" << projectDir;
#else
    // Fallback to application directory
    projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
    qDebug() << "Padel - Using application directory path:" << projectDir;
#endif
    
    // Set data directory paths
    dataDir = projectDir + "/project code/Data";
    
    qDebug() << "Padel - Data directory path:" << dataDir;
    
    // Create directories if they don't exist
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
    
    // Initialize empty waitlists.json if it doesn't exist
    QFile waitlistsFile(dataDir + "/waitlists.json");
    if (!waitlistsFile.exists()) {
        waitlistsFile.open(QIODevice::WriteOnly);
        waitlistsFile.write("[]");
        waitlistsFile.close();
    }
    
    // Initialize data from files
    if (!initializeFromFile()) {
        qDebug() << "Failed to initialize padel data from files";
    }

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
    QMutexLocker locker(&mutex);

    QJsonArray courtsArray = readCourtsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading courts file:" << errorMessage;
        return false;
    }

    courtsById.clear();
    for (const QJsonValue& courtValue : courtsArray) {
        QJsonObject courtObj = courtValue.toObject();
        Court court = jsonToCourt(courtObj);

        courtsById[court.getId()] = court;

        qDebug() << "Loaded court:" << court.getId() << court.getName() 
                << "Price:" << court.getPricePerHour();
    }

    qDebug() << "Loaded" << courtsById.size() << "courts from file";

    QJsonArray bookingsArray = readBookingsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading bookings file:" << errorMessage;
        return false;
    }

    bookingsById.clear();

    for (const QJsonValue& bookingValue : bookingsArray) {
        QJsonObject bookingObj = bookingValue.toObject();
        Booking booking = jsonToBooking(bookingObj);
        bookingsById[booking.getBookingId()] = booking;
    }

    qDebug() << "Loaded" << bookingsById.size() << "bookings from file";

    QJsonArray waitlistsArray = readWaitlistsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading waitlists file:" << errorMessage;
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

    qDebug() << "Loaded waitlist entries for" << courtWaitlists.size() << "courts from file";

    return true;
}

bool PadelDataManager::saveToFile() {

    qDebug() << "Starting simplified saveToFile function...";

    try {
        if (!dataModified) {
            qDebug() << "No data modifications to save";
            return true;  
        }

        QString errorMessage;

        QDir dataDirectory(dataDir);
        if (!dataDirectory.exists()) {
            qDebug() << "Creating data directory at:" << dataDir;
            bool dirCreated = QDir().mkpath(dataDir);
            if (!dirCreated) {
                qDebug() << "Failed to create data directory at:" << dataDir;
                return false;
            }
        }

        try {
            qDebug() << "Saving waitlists...";
            QJsonArray waitlistsArray = waitlistsToJson();

            QString filePath = QDir(dataDir).filePath("waitlists.json");
            QFile file(filePath);

            if (!file.open(QIODevice::WriteOnly)) {
                qDebug() << "Could not open waitlists file for writing:" << file.errorString();
                return false;
            }

            QJsonDocument doc(waitlistsArray);
            QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

            qint64 bytesWritten = file.write(jsonData);
            if (bytesWritten == -1) {
                qDebug() << "Failed to write to waitlists file:" << file.errorString();
                file.close();
                return false;
            }

            file.flush();
            file.close();
            qDebug() << "Waitlist file saved successfully";
        }
        catch (const std::exception& e) {
            qDebug() << "Exception saving waitlists:" << e.what();

        }

        qDebug() << "Simplified saveToFile completed successfully";
        dataModified = false;
        return true;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in simplified saveToFile:" << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in simplified saveToFile";
        return false;
    }
}

bool PadelDataManager::writeBookingsToFile(const QJsonArray& bookings, QString& errorMessage) const {
    try {
        QString filePath = QDir(dataDir).filePath("bookings.json");
        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        QDir().mkpath(fileInfo.absolutePath());

        qDebug() << "Saving bookings to file:" << filePath;

        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = "Could not open bookings file for writing: " + file.errorString();
            qDebug() << errorMessage;
            return false;
        }

        QJsonDocument doc(bookings);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

        qint64 bytesWritten = file.write(jsonData);
        if (bytesWritten == -1) {
            errorMessage = "Failed to write to bookings file: " + file.errorString();
            file.close();
            qDebug() << errorMessage;
            return false;
        }

        file.flush(); 
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in writeBookingsToFile: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in writeBookingsToFile";
        qDebug() << errorMessage;
        return false;
    }
}

bool PadelDataManager::writeCourtsToFile(const QJsonArray& courts, QString& errorMessage) const {
    try {
        QString filePath = QDir(dataDir).filePath("courts.json");
        QFile file(filePath);

        QFileInfo fileInfo(filePath);
        QDir().mkpath(fileInfo.absolutePath());

        qDebug() << "Saving courts to file:" << filePath;

        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = "Could not open courts file for writing: " + file.errorString();
            qDebug() << errorMessage;
            return false;
        }

        QJsonDocument doc(courts);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

        qint64 bytesWritten = file.write(jsonData);
        if (bytesWritten == -1) {
            errorMessage = "Failed to write to courts file: " + file.errorString();
            file.close();
            qDebug() << errorMessage;
            return false;
        }

        file.flush(); 
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in writeCourtsToFile: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in writeCourtsToFile";
        qDebug() << errorMessage;
        return false;
    }
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
    try {
        if (courtId <= 0) {
            qDebug() << "Invalid court ID in getCourtById:" << courtId;
            return Court(); 
        }

        QMutex& mutexRef = mutex;
        QMutexLocker locker(&mutexRef);

        if (courtsById.empty()) {
            qDebug() << "Courts map is empty in getCourtById";
            return Court(); 
        }

    auto it = courtsById.find(courtId);
    if (it != courtsById.end()) {
            Court result = it->second; 
            qDebug() << "Found court:" << result.getName() << "with price:" << result.getPricePerHour();
            return result;
    }

        qDebug() << "Court not found with ID:" << courtId;
    return Court();
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in getCourtById:" << e.what();
        return Court(); 
    }
    catch (...) {
        qDebug() << "Unknown exception in getCourtById";
        return Court(); 
    }
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
    try {
        qDebug() << "Creating booking for user" << userId << "court" << courtId
                << "from" << startTime.toString() << "to" << endTime.toString()
                << "isFromWaitlist:" << isFromWaitlist;

        if (userId <= 0) {
            errorMessage = "Invalid user ID";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }

        if (courtId <= 0) {
        errorMessage = "Invalid court ID";
            qDebug() << "Booking failed:" << errorMessage;
        return false;
    }

        QMutexLocker locker(&mutex);

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            errorMessage = "Court not found with ID: " + QString::number(courtId);
            qDebug() << "Booking failed:" << errorMessage;
        return false;
    }

        const Court& court = courtIt->second;
            qDebug() << "Found court:" << court.getName() << "at location:" << court.getLocation();

        int maxAttendees = court.getMaxAttendees();

        qDebug() << "Validating booking time...";
        if (!startTime.isValid() || !endTime.isValid()) {
            errorMessage = "Invalid booking time";
            qDebug() << "Booking failed:" << errorMessage;
        return false;
    }

        if (startTime >= endTime) {
            errorMessage = "Start time must be before end time";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }

        qDebug() << "Booking time is valid";

        qDebug() << "Checking court availability...";

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

        qDebug() << "Checking capacity: Current attendees:" << currentAttendees << "Max attendees:" << maxAttendees;

        if (currentAttendees >= maxAttendees) {
            qDebug() << "Court has reached maximum attendees limit: " << maxAttendees;

            bool isVip = false;

            if (memberDataManager) {
                try {
                    if (memberDataManager->userIsMember(userId)) {
                        int memberId = memberDataManager->getMemberIdByUserId(userId);
                        isVip = memberDataManager->isVIPMember(memberId);
                    }
                } catch (const std::exception& e) {
                    qDebug() << "Exception checking VIP status:" << e.what();
                }
            }

            QString waitlistError;
            if (addToWaitlist(userId, courtId, startTime, waitlistError)) {
                errorMessage = "Court is at maximum capacity. You have been added to the waitlist.";
                qDebug() << "User added to waitlist:" << errorMessage;
                return false; 
            } else {
                errorMessage = "Court is at maximum capacity and could not add to waitlist: " + waitlistError;
                qDebug() << "Booking failed:" << errorMessage;
                return false;
            }
        }

        qDebug() << "Court has space available. Continuing with booking process.";

        for (const auto& pair : bookingsById) {
            const Booking& existingBooking = pair.second;

            if (existingBooking.isCancelled()) {
                continue;
            }

            if (existingBooking.getUserId() == userId && 
                existingBooking.getCourtId() == courtId &&
                existingBooking.getStartTime().date() == startTime.date()) {

                bool timeOverlap = false;
                try {
                    QDateTime bookingStart = existingBooking.getStartTime();
                    QDateTime bookingEnd = existingBooking.getEndTime();

                    if (bookingStart.isValid() && bookingEnd.isValid()) {
                        timeOverlap = (bookingStart < endTime && bookingEnd > startTime);
                        qDebug() << "Time overlap check for user's existing booking:"
                                << "Booking time:" << bookingStart.toString() << "-" << bookingEnd.toString()
                                << "Requested time:" << startTime.toString() << "-" << endTime.toString()
                                << "Result:" << timeOverlap;
                    }
                } catch (const std::exception& e) {
                    qDebug() << "Exception in time overlap check:" << e.what();

                }

                if (timeOverlap) {
                    errorMessage = "You already have a booking at this time on this court";
                    qDebug() << "Booking failed:" << errorMessage;
                    return false;
                }
            }
        }

        qDebug() << "Checking user data...";
        User user;

        if (!memberDataManager) {
            errorMessage = "Member data manager is not set";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }

        qDebug() << "Member data manager is set";

        UserDataManager* userManager = memberDataManager->getUserDataManager();
        if (!userManager) {
            errorMessage = "User data manager is not set";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }

        qDebug() << "User data manager is set, getting user data for ID:" << userId;

        try {
            user = userManager->getUserDataById(userId);
            if (user.getId() <= 0) {
                errorMessage = "User not found with ID: " + QString::number(userId);
                qDebug() << "Booking failed:" << errorMessage;
                return false;
            }
            qDebug() << "Found user: \"" << user.getName() << "\" with email: \"" << user.getEmail() << "\"";
        } catch (const std::exception& e) {
            errorMessage = QString("Exception getting user data: %1").arg(e.what());
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        } catch (...) {
            errorMessage = "Unknown exception getting user data";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }

        qDebug() << "Generating booking ID...";
    int bookingId = generateBookingId();
        qDebug() << "Generated new booking ID:" << bookingId;

        qDebug() << "Creating booking object...";

        Booking newBooking;
        newBooking.setBookingId(bookingId);
        newBooking.setCourtId(courtId);
        newBooking.setStartTime(startTime);
        newBooking.setEndTime(endTime);
        newBooking.setUserId(userId);
        newBooking.setIsFromWaitlist(isFromWaitlist);

        qDebug() << "Checking if user is VIP member...";
        bool isVip = false;

        try {
            qDebug() << "Checking if user ID " << userId << " is a member (safe method)";
            bool isMember = memberDataManager->userIsMember(userId);
            qDebug() << "User ID " << userId << " is member: " << isMember;

            if (isMember) {
                qDebug() << "User is a member";
                int memberId = memberDataManager->getMemberIdByUserId(userId);
                qDebug() << "Member ID: " << memberId;
                isVip = memberDataManager->isVIPMember(memberId);
                qDebug() << "User is VIP member: " << isVip;
            } else {
                qDebug() << "User is not a member";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception checking VIP status:" << e.what();

        }

        qDebug() << "Calculating booking price...";
        double price = 0.0;

        try {
            price = calculateBookingPrice(courtId, startTime, endTime, isVip);
            qDebug() << "Calculated booking price:" << price;
        } catch (const std::exception& e) {
            qDebug() << "Exception calculating price:" << e.what();
            price = court.getPricePerHour(); 
            qDebug() << "Using fallback price:" << price;
        }

        newBooking.setPrice(price);
        newBooking.setVip(isVip);

        qDebug() << "Adding booking to the system...";
        bookingsById[bookingId] = newBooking;
        qDebug() << "Added booking to bookings map";

    dataModified = true;

        qDebug() << "Booking created successfully with ID:" << bookingId;

        QMetaObject::invokeMethod(this, "safeEmitBookingCreated", 
                              Qt::QueuedConnection, 
                              Q_ARG(int, bookingId),
                              Q_ARG(int, userId));

        qDebug() << "Saving changes to file...";
        QString saveError;
        if (!saveToFile()) {
            qDebug() << "Warning: Failed to save booking to file: " << saveError;
        } else {
            qDebug() << "Booking saved to file successfully";
        }

    return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in createBooking: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in createBooking";
        qDebug() << errorMessage;
        return false;
    }
}

void PadelDataManager::safeEmitBookingCreated(int bookingId, int userId) {
    qDebug() << "Safe emit of bookingCreated signal for booking ID:" << bookingId << "user ID:" << userId;
    emit bookingCreated(bookingId, userId);
}

double PadelDataManager::calculateBookingPrice(int courtId, const QDateTime& startTime, 
                                             const QDateTime& endTime, bool isVIP) const {
    qDebug() << "Inside calculateBookingPrice for court:" << courtId;

    try {

        double pricePerHour = 100.0; 

        {

            bool locked = mutex.tryLock(100); 
            if (locked) {

                auto it = courtsById.find(courtId);
                if (it != courtsById.end()) {
                    pricePerHour = it->second.getPricePerHour();
                    qDebug() << "Got price from court object (locked): " << pricePerHour;
                } else {
                    qDebug() << "Court not found, using default price: " << pricePerHour;
                }
                mutex.unlock(); 
            } else {

                qDebug() << "Could not lock mutex, using default price: " << pricePerHour;
            }
        }

        int durationInSeconds = startTime.secsTo(endTime);
        double durationInHours = static_cast<double>(durationInSeconds) / 3600.0;
        qDebug() << "Duration in hours: " << durationInHours;

        double basePrice = pricePerHour * durationInHours;
        double finalPrice = basePrice;

    if (isVIP) {
            finalPrice = basePrice * 0.85; 
            qDebug() << "Applied VIP discount. Final price: " << finalPrice;
        } else {
            qDebug() << "Regular price (no discount): " << finalPrice;
        }

        return finalPrice;
    }
    catch (const std::exception& e) {
        qDebug() << "Error in calculateBookingPrice: " << e.what();
        return 100.0; 
    }
    catch (...) {
        qDebug() << "Unknown error in calculateBookingPrice";
        return 100.0; 
    }
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
        qDebug() << "Warning: Could not lock mutex in cancelBooking, will retry later";
        errorMessage = "System busy, please try again";
        return false;
    }

    bool result = false;
    int courtId = -1;
    QDateTime startTime, endTime;

    try {
        qDebug() << "Attempting to cancel booking with ID:" << bookingId;

        auto it = bookingsById.find(bookingId);
        if (it == bookingsById.end()) {
            errorMessage = "Booking not found";
            qDebug() << "Cancellation failed:" << errorMessage;
            mutex.unlock();
            return false;
        }

        if (it->second.isCancelled()) {
            errorMessage = "Booking is already cancelled";
            qDebug() << "Cancellation failed:" << errorMessage;
            mutex.unlock();
            return false;
        }

        QDateTime now = QDateTime::currentDateTime();
        const Booking& booking = it->second;
        bool canCancel = now.secsTo(booking.getStartTime()) >= 3 * 60 * 60; 

        if (!canCancel) {
            errorMessage = "Cancellation is not allowed within 3 hours of booking time";
            qDebug() << "Cancellation failed:" << errorMessage;
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

        qDebug() << "Emitting bookingCancelled signal using safe method...";
        QMetaObject::invokeMethod(this, "safeEmitBookingCancelled", 
                              Qt::QueuedConnection, 
                              Q_ARG(int, bookingId),
                              Q_ARG(int, memberId));

        qDebug() << "Booking cancelled successfully with ID:" << bookingId;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in cancelBooking: %1").arg(e.what());
        qDebug() << errorMessage;
    }
    catch (...) {
        errorMessage = "Unknown exception in cancelBooking";
        qDebug() << errorMessage;
    }

    mutex.unlock();

    if (result && courtId > 0 && startTime.isValid() && endTime.isValid()) {
        qDebug() << "Attempting to fill cancelled slot from waitlist - Court:" << courtId 
                 << "Time:" << startTime.toString();

        QString waitlistError;
        if (tryFillSlotFromWaitlist(courtId, startTime, endTime, waitlistError)) {
            qDebug() << "Successfully filled cancelled slot from waitlist!";
        } else {
            qDebug() << "No suitable waitlist entry found for cancelled slot:" << waitlistError;
        }
    }

    return result;
}

void PadelDataManager::safeEmitBookingCancelled(int bookingId, int userId) {
    qDebug() << "Safe emit of bookingCancelled signal for booking ID:" << bookingId << "user ID:" << userId;
    emit bookingCancelled(bookingId, userId);
}

bool PadelDataManager::rescheduleBooking(int bookingId, const QDateTime& newStartTime, 
                                       const QDateTime& newEndTime, QString& errorMessage) {

    bool locked = mutex.tryLock(200); 

    if (!locked) {
        qDebug() << "Warning: Could not lock mutex in rescheduleBooking, will retry later";
        errorMessage = "System busy, please try again";
        return false;
    }

    bool result = false;
    int courtId = -1;
    QDateTime oldStartTime, oldEndTime;

    try {
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

        int courtId = booking.getCourtId();
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
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in rescheduleBooking: %1").arg(e.what());
        qDebug() << errorMessage;
    }
    catch (...) {
        errorMessage = "Unknown exception in rescheduleBooking";
        qDebug() << errorMessage;
    }

    mutex.unlock();

    if (result && courtId > 0 && oldStartTime.isValid() && oldEndTime.isValid()) {
        qDebug() << "Attempting to fill old slot from waitlist after reschedule - Court:" << courtId 
                 << "Time:" << oldStartTime.toString();

        QString waitlistError;
        if (tryFillSlotFromWaitlist(courtId, oldStartTime, oldEndTime, waitlistError)) {
            qDebug() << "Successfully filled old slot from waitlist after reschedule!";
        } else {
            qDebug() << "No suitable waitlist entry found for old slot:" << waitlistError;
        }
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

bool PadelDataManager::isCourtAvailable(int courtId, const QDateTime& startTime, 
                                      const QDateTime& endTime) const {
    qDebug() << "Inside isCourtAvailable for court:" << courtId;

    try {

        QMutexLocker locker(&mutex); 

    auto courtIt = courtsById.find(courtId);
    if (courtIt == courtsById.end()) {
        qDebug() << "Court not found with ID:" << courtId;
        return false;
    }

    const Court& court = courtIt->second;
    int maxAttendees = court.getMaxAttendees();

    qDebug() << "Court found, checking time validity";

    if (!startTime.isValid() || !endTime.isValid() || startTime >= endTime) {
        qDebug() << "Invalid time range:" << startTime.toString() << " - " << endTime.toString();
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

            QDate bookingDate, requestDate;
            try {
                if (booking.getStartTime().isValid()) {
                    bookingDate = booking.getStartTime().date();
                }
                if (startTime.isValid()) {
                    requestDate = startTime.date();
                }
            } catch (...) {
                qDebug() << "Exception when getting dates, skipping booking";
                continue;
            }

            if (!bookingDate.isValid() || !requestDate.isValid() || bookingDate != requestDate) {
                continue;
            }

            bool overlap = false;
            try {
                if (booking.getStartTime().isValid() && booking.getEndTime().isValid() &&
                    startTime.isValid() && endTime.isValid()) {
                    overlap = (booking.getStartTime() <= endTime && 
                              booking.getEndTime() >= startTime);
                }
            } catch (...) {
                qDebug() << "Exception when checking overlap, skipping booking";
                continue;
            }

            if (overlap) {
                currentBookings++;
            }
        }

    qDebug() << "Court has" << currentBookings << "of maximum" << maxAttendees << "attendees";

        bool isAvailable = (currentBookings < maxAttendees);
    qDebug() << "Court availability result:" << isAvailable;

    return isAvailable;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in isCourtAvailable:" << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in isCourtAvailable";
        return false;
    }
}

bool PadelDataManager::validateCourtAvailability(int courtId, const QDateTime& startTime, 
                                              const QDateTime& endTime) const {

    qDebug() << "Inside validateCourtAvailability for court:" << courtId;

    return isCourtAvailable(courtId, startTime, endTime);
}

bool PadelDataManager::addToWaitlist(int userId, int courtId, const QDateTime& requestedTime, 
                                   QString& errorMessage) {
    try {

        qDebug() << "Starting improved addToWaitlist function...";

        if (userId <= 0 || courtId <= 0) {
            errorMessage = "Invalid user or court ID.";
            qDebug() << "Waitlist error:" << errorMessage;
            return false;
        }

        if (courtsById.find(courtId) == courtsById.end()) {
            errorMessage = "Court not found.";
            qDebug() << "Waitlist error:" << errorMessage;
            return false;
        }

        bool isVIP = false;
        int priority = 0;

        try {
            qDebug() << "Checking VIP status for user ID:" << userId;
            if (memberDataManager) {

                bool isMember = memberDataManager->userIsMember(userId);
                qDebug() << "User" << userId << "is member:" << isMember;

                if (isMember) {

                    int memberId = memberDataManager->getMemberIdByUserId(userId);
                    isVIP = memberDataManager->isVIPMember(memberId);
                    qDebug() << "User" << userId << "is VIP:" << isVIP;

                    if (isVIP) {
                        priority = 100; 
                    } else {
                        priority = 50;  
                    }
                } else {

                    priority = 10;
                }
            } else {
                qDebug() << "Warning: memberDataManager is null, cannot check VIP status";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception checking VIP status:" << e.what();

        }

        qDebug() << "Creating waitlist entry with VIP status:" << isVIP << ", priority:" << priority;
        WaitlistEntry entry;
        entry.memberId = userId;    
        entry.courtId = courtId;
        entry.requestedTime = requestedTime;
        entry.isVIP = isVIP;
        entry.priority = priority;

        qDebug() << "Adding entry directly to waitlist...";

        if (courtWaitlists.find(courtId) == courtWaitlists.end()) {
            qDebug() << "Creating new waitlist queue for court" << courtId;
            courtWaitlists[courtId] = std::queue<WaitlistEntry>();
        }

        courtWaitlists[courtId].push(entry);
        qDebug() << "Entry successfully added to waitlist";

        dataModified = true;

        try {
            saveToFile();
        } catch (const std::exception& e) {
            qDebug() << "Warning: Error saving waitlist to file:" << e.what();

        }

        try {
            updateWaitlistPositionsAndNotify(courtId);
        } catch (const std::exception& e) {
            qDebug() << "Warning: Error updating waitlist positions:" << e.what();

        }

        qDebug() << "Improved waitlist add operation completed successfully";
        return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in addToWaitlist: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in addToWaitlist";
        qDebug() << errorMessage;
        return false;
    }
}

bool PadelDataManager::removeFromWaitlist(int userId, int courtId, QString& errorMessage) {
    try {

        qDebug() << "Removing user" << userId << "from waitlist for court" << courtId;

        auto it = courtWaitlists.find(courtId);
        if (it == courtWaitlists.end()) {
            errorMessage = "No waitlist found for this court";
            qDebug() << "Waitlist error:" << errorMessage;
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
                qDebug() << "Found user" << userId << "in waitlist, removing entry for time" 
                         << entry.requestedTime.toString();
            }
        }

        if (!found) {
            errorMessage = "User not found in waitlist";
            qDebug() << "Waitlist error:" << errorMessage;
            return false;
        }

        courtWaitlists[courtId] = newWaitlist;
        dataModified = true;

        if (newWaitlist.empty()) {
            courtWaitlists.erase(courtId);
            qDebug() << "Waitlist for court" << courtId << "is now empty";
        }

        try {
            saveToFile();
        } catch (const std::exception& e) {
            qDebug() << "Warning: Failed to save waitlist changes to file:" << e.what();

        }

        try {
            emit waitlistUpdated(courtId);
        } catch (const std::exception& e) {
            qDebug() << "Warning: Failed to emit waitlist update signal:" << e.what();

        }

        qDebug() << "User" << userId << "successfully removed from waitlist for court" << courtId;
        return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in removeFromWaitlist: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in removeFromWaitlist";
        qDebug() << errorMessage;
        return false;
    }
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
    try {

        qDebug() << "Processing waitlist for court" << courtId;

        auto it = courtWaitlists.find(courtId);
        if (it == courtWaitlists.end() || it->second.empty()) {
            errorMessage = "No waitlist entries for this court";
            qDebug() << "Waitlist error:" << errorMessage;
            return false;
        }

        WaitlistEntry entry = it->second.front();
        it->second.pop();

        int userId = entry.memberId; 
        qDebug() << "Processing waitlist entry for user ID:" << userId;

        if (isCourtAvailable(courtId, entry.requestedTime, entry.requestedTime.addSecs(3600))) { 
            QString bookingError;
            bool success = createBooking(userId, courtId, 
                                   entry.requestedTime, 
                                   entry.requestedTime.addSecs(3600), 
                                       bookingError,
                                       true);  

            if (success) {
                qDebug() << "Successfully created booking from waitlist for user" << userId;

                if (it->second.empty()) {
                    courtWaitlists.erase(it);
                    qDebug() << "Waitlist for court" << courtId << "is now empty";
                }

                dataModified = true;

                try {
                    saveToFile();
                } catch (const std::exception& e) {
                    qDebug() << "Warning: Failed to save waitlist changes:" << e.what();

                }

                return true;
            } else {
                errorMessage = "Failed to create booking: " + bookingError;
                qDebug() << "Waitlist error:" << errorMessage;

                it->second.push(entry);
                return false;
            }
        }

        it->second.push(entry);
        errorMessage = "Court not available at the requested time";
        qDebug() << "Waitlist error:" << errorMessage;
        return false;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in processWaitlist: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in processWaitlist";
        qDebug() << errorMessage;
        return false;
    }
}

void PadelDataManager::setVIPPriority(int memberId, bool isVIP) {

    vipMembers[memberId] = isVIP;
    dataModified = true;

    try {
        emit vipStatusChanged(memberId, isVIP);
    } catch (const std::exception& e) {
        qDebug() << "Warning: Error emitting VIP status changed signal:" << e.what();
    }
}

bool PadelDataManager::isVIPMember(int memberId) const {
    if (memberId <= 0) {
        qDebug() << "Invalid member ID in isVIPMember:" << memberId;
        return false;
    }

    try {

        qDebug() << "Checking VIP status for member ID:" << memberId;

        auto it = vipMembers.find(memberId);
        bool isVip = (it != vipMembers.end() && it->second);

        qDebug() << "Member" << memberId << "VIP status:" << isVip;
        return isVip;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in isVIPMember for member ID" << memberId << ":" << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in isVIPMember for member ID" << memberId;
        return false;
    }
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
    try {

        if (userId <= 0 || courtId <= 0) {
            qDebug() << "Invalid user ID or court ID in getWaitlistPosition";
            return -1;
        }

        qDebug() << "Getting waitlist position for user" << userId << "on court" << courtId;

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            qDebug() << "Court not found in getWaitlistPosition:" << courtId;
            return -1;
        }

    auto it = courtWaitlists.find(courtId);
    if (it == courtWaitlists.end()) {
            qDebug() << "No waitlist exists for court:" << courtId;
        return -1; 
    }

        if (it->second.empty()) {
            qDebug() << "Waitlist for court " << courtId << " is empty";
            return -1; 
        }

        QVector<WaitlistEntry> entries;
    std::queue<WaitlistEntry> tempQueue = it->second;

    while (!tempQueue.empty()) {
            entries.append(tempQueue.front());
        tempQueue.pop();
        }

        if (entries.isEmpty()) {
            qDebug() << "Empty waitlist for court" << courtId;
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
                qDebug() << "User" << userId << "is at position" << position << "in waitlist";
            return position;
            }
        }

        qDebug() << "User" << userId << "not found in waitlist";
        return -1;  
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in getWaitlistPosition:" << e.what();
        return -1;
    }
    catch (...) {
        qDebug() << "Unknown exception in getWaitlistPosition";
        return -1;
    }
}

void PadelDataManager::removeUserFromAllWaitlists(int userId, int courtId, const QDate& date) {
    qDebug() << "Removing user" << userId << "from all waitlists for court" << courtId << "on date" << date.toString();

    try {
        if (userId <= 0 || courtId <= 0 || !date.isValid()) {
            qDebug() << "Invalid parameters in removeUserFromAllWaitlists";
            return;
        }

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            qDebug() << "Court not found in removeUserFromAllWaitlists:" << courtId;
            return;
        }

        auto it = courtWaitlists.find(courtId);
        if (it == courtWaitlists.end() || it->second.empty()) {
            qDebug() << "No waitlists found for court" << courtId;
            return;
        }

        std::queue<WaitlistEntry> newQueue;
        std::queue<WaitlistEntry> tempQueue = it->second;

        if (tempQueue.empty()) {
            qDebug() << "Waitlist queue is empty after copy in removeUserFromAllWaitlists";
            return;
        }

        while (!tempQueue.empty()) {
            WaitlistEntry entry = tempQueue.front();
            tempQueue.pop();

            try {
                if (entry.memberId != userId || entry.courtId != courtId || entry.requestedTime.date() != date) {
                    newQueue.push(entry);
                } else {
                    qDebug() << "Removing waitlist entry for user" << userId << "at time" << entry.requestedTime.toString();
                }
            } catch (const std::exception& e) {
                qDebug() << "Exception processing waitlist entry:" << e.what();

            } catch (...) {
                qDebug() << "Unknown exception processing waitlist entry";

            }
        }

        try {
            if (newQueue.empty()) {
                courtWaitlists.erase(courtId);
                qDebug() << "Removed entire waitlist for court" << courtId;
            } else {
                courtWaitlists[courtId] = newQueue;
                qDebug() << "Updated waitlist for court" << courtId << ", now has" << newQueue.size() << "entries";
            }

            emit waitlistUpdated(courtId);
            dataModified = true;
        } catch (const std::exception& e) {
            qDebug() << "Exception updating waitlist:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception updating waitlist";
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in removeUserFromAllWaitlists:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in removeUserFromAllWaitlists";
    }
}

void PadelDataManager::updateWaitlistPositionsAndNotify(int courtId) {

    if (courtId <= 0) {
        qDebug() << "Invalid court ID in updateWaitlistPositionsAndNotify";
        return;
    }

    try {
        qDebug() << "Updating waitlist positions for court" << courtId;

        if (courtsById.find(courtId) == courtsById.end()) {
            qDebug() << "Court not found in updateWaitlistPositionsAndNotify:" << courtId;
            return;
        }

        auto waitlistIt = courtWaitlists.find(courtId);
        if (waitlistIt == courtWaitlists.end()) {
            qDebug() << "No waitlist exists for court" << courtId << " - nothing to update";
            return;
        }

        if (waitlistIt->second.empty()) {
            qDebug() << "Waitlist for court " << courtId << " is empty - nothing to update";
            return;
        }

        QVector<WaitlistEntry> entries;
        try {
            std::queue<WaitlistEntry> tempQueue = waitlistIt->second;
            while (!tempQueue.empty()) {
                entries.append(tempQueue.front());
                tempQueue.pop();
            }

            dataModified = true;
        } 
        catch (const std::exception& e) {
            qDebug() << "Exception copying waitlist entries:" << e.what();
            return;
        } catch (...) {
            qDebug() << "Unknown exception copying waitlist entries";
            return;
        }

        if (entries.isEmpty()) {
            qDebug() << "No waitlist entries to process after copy";
            return;
        }

        try {
            std::sort(entries.begin(), entries.end(), [](const WaitlistEntry& a, const WaitlistEntry& b) {
                if (a.isVIP != b.isVIP) {
                    return a.isVIP > b.isVIP;
                }
                return a.priority > b.priority;
            });
        } catch (const std::exception& e) {
            qDebug() << "Exception during waitlist sorting:" << e.what();
            return;
        } catch (...) {
            qDebug() << "Unknown exception during waitlist sorting";
            return;
        }

        for (int i = 0; i < entries.size(); i++) {
            try {
                int userId = entries[i].memberId;
                int position = i + 1;

                qDebug() << "User" << userId << "is now position" << position << "in waitlist for court" << courtId;
                emit waitlistPositionChanged(userId, courtId, position);
            } catch (const std::exception& e) {
                qDebug() << "Exception emitting waitlist position signal:" << e.what();

            } catch (...) {
                qDebug() << "Unknown exception emitting waitlist position signal";

            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in updateWaitlistPositionsAndNotify:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in updateWaitlistPositionsAndNotify";
    }
}

bool PadelDataManager::userHasBookingAtTime(int userId, int courtId, const QDate& date, const QTime& timeSlot) const {
    if (userId <= 0 || courtId <= 0 || !date.isValid() || !timeSlot.isValid()) {
        return false;
    }

    try {
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
    catch (const std::exception& e) {
        qDebug() << "Exception in userHasBookingAtTime:" << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in userHasBookingAtTime";
        return false;
    }
}

bool PadelDataManager::userHasBookingOnDate(int userId, int courtId, const QDate& date) const {
    if (userId <= 0 || !date.isValid()) {
        return false;
    }

    try {
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
    catch (const std::exception& e) {
        qDebug() << "Exception in userHasBookingOnDate:" << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in userHasBookingOnDate";
        return false;
    }
}

QVector<Booking> PadelDataManager::getUserAutoBookings(int userId) const {
    QVector<Booking> result;

    try {
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
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in getUserAutoBookings:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in getUserAutoBookings";
    }

    return result;
}

bool PadelDataManager::isUserInWaitlist(int userId, int courtId, const QDateTime& requestedTime) const {

    if (userId <= 0 || courtId <= 0 || !requestedTime.isValid()) {
        qDebug() << "Invalid parameters in isUserInWaitlist - UserId:" << userId 
                 << "CourtId:" << courtId 
                 << "Time valid:" << requestedTime.isValid();
        return false;
    }

    try {

        bool locked = mutex.tryLock(100); 
        if (!locked) {
            qDebug() << "Could not lock mutex in isUserInWaitlist, returning false";
            return false;
        }

        struct ScopedUnlock {
            QMutex& m;
            ScopedUnlock(QMutex& mutex) : m(mutex) {}
            ~ScopedUnlock() { m.unlock(); }
        } unlocker(mutex);

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            qDebug() << "Court not found in isUserInWaitlist:" << courtId;
            return false;
        }

        auto it = courtWaitlists.find(courtId);
        if (it == courtWaitlists.end()) {
            qDebug() << "No waitlist for court" << courtId << "in isUserInWaitlist";
            return false;
        }

        if (it->second.empty()) {
            qDebug() << "Waitlist for court" << courtId << "is empty";
            return false;
        }

        std::queue<WaitlistEntry> tempQueue;
        try {
            tempQueue = it->second;
        } catch (const std::exception& e) {
            qDebug() << "Exception copying waitlist in isUserInWaitlist:" << e.what();
            return false;
        } catch (...) {
            qDebug() << "Unknown exception copying waitlist in isUserInWaitlist";
            return false;
        }

        if (tempQueue.empty()) {
            qDebug() << "Empty waitlist queue for court" << courtId << " after copying";
            return false;
        }

        while (!tempQueue.empty()) {

            WaitlistEntry entry;
            try {
                entry = tempQueue.front();
                tempQueue.pop();
            } catch (const std::exception& e) {
                qDebug() << "Exception accessing waitlist entry in isUserInWaitlist:" << e.what();
                continue; 
            } catch (...) {
                qDebug() << "Exception accessing waitlist entry in isUserInWaitlist";
                continue; 
            }

            try {

                if (entry.memberId != userId) continue;

                if (!entry.requestedTime.isValid() || 
                    !requestedTime.isValid() || 
                    entry.requestedTime.date() != requestedTime.date()) {
                    continue;
                }

                if (requestedTime.time().isValid() && entry.requestedTime.time().isValid()) {

                    int hourDiff = abs(entry.requestedTime.time().hour() - requestedTime.time().hour());
                    if (hourDiff <= 1) {
                        qDebug() << "User" << userId << "found in waitlist for court" << courtId;
                        return true;
                    }
                } else {

                    qDebug() << "User" << userId << "found in waitlist for court" << courtId << "on date" << requestedTime.date().toString();
                    return true;
                }
            } catch (const std::exception& e) {
                qDebug() << "Exception comparing waitlist entry in isUserInWaitlist:" << e.what();
                continue; 
            } catch (...) {
                qDebug() << "Exception comparing waitlist entry in isUserInWaitlist";
                continue; 
            }
        }

        qDebug() << "User" << userId << "not found in waitlist for court" << courtId;
        return false;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in isUserInWaitlist:" << e.what();
        return false;
    }
    catch (...) {
        qDebug() << "Unknown exception in isUserInWaitlist";
        return false;
    }
}

QJsonObject PadelDataManager::getDetailedWaitlistInfo(int courtId, const QDate& date) const {
    QJsonObject result;

    try {
        if (courtId <= 0) {
            qDebug() << "Invalid court ID in getDetailedWaitlistInfo:" << courtId;
            result["error"] = "Invalid court ID";
            result["waitlistCount"] = 0;
            result["entries"] = QJsonArray();
            return result;
        }

        QVector<WaitlistEntry> entries;
        QVector<User> users;

        try {

            if (courtsById.find(courtId) == courtsById.end()) {
                qDebug() << "Court not found in getDetailedWaitlistInfo:" << courtId;
                result["error"] = "Court not found";
                result["waitlistCount"] = 0;
                result["entries"] = QJsonArray();
                return result;
            }

            auto it = courtWaitlists.find(courtId);
            if (it == courtWaitlists.end() || it->second.empty()) {
                qDebug() << "No waitlist entries for court:" << courtId << "on date:" << date.toString("yyyy-MM-dd");
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
        } catch (const std::exception& e) {
            qDebug() << "Exception copying waitlist entries:" << e.what();

        }

        QVector<int> indices;
        for (int i = 0; i < entries.size(); i++) {
            indices.append(i);
        }

        try {
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

            qDebug() << "Sorted waitlist entries by priority: VIP first, then members, then others";

            for (int i = 0; i < indices.size() && i < 5; i++) {
                int idx = indices[i];
                qDebug() << "Position" << (i+1) << "- User:" << entries[idx].memberId
                         << "VIP:" << entries[idx].isVIP
                         << "Priority:" << entries[idx].priority;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception during waitlist sorting:" << e.what();

        }

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
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in getDetailedWaitlistInfo:" << e.what();
        result["error"] = QString("Error getting waitlist info: %1").arg(e.what());
    }
    catch (...) {
        qDebug() << "Unknown exception in getDetailedWaitlistInfo";
        result["error"] = "Unknown error getting waitlist info";
    }

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
    try {
    QMutexLocker locker(&mutex);

        if (courtId <= 0) {
            qDebug() << "Invalid court ID in getAllTimeSlots:" << courtId;
            return QVector<QTime>();
        }

    auto it = courtsById.find(courtId);
    if (it == courtsById.end()) {

            qDebug() << "Court not found in getAllTimeSlots, ID:" << courtId;
        return QVector<QTime>();
    }

    const Court& court = it->second;

    const std::vector<QTime>& timeSlots = court.getAllTimeSlots();

        qDebug() << "Court" << courtId << "has" << timeSlots.size() << "time slots";

    QVector<QTime> result;
    result.reserve(timeSlots.size());

    for (const QTime& slot : timeSlots) {
            if (slot.isValid()) {
        result.append(slot);
                qDebug() << "Adding valid time slot:" << slot.toString("HH:mm");
            } else {
                qDebug() << "Skipping invalid time slot in court" << courtId;
            }
    }

        qDebug() << "Returning" << result.size() << "valid time slots for court" << courtId;
    return result;
    } catch (const std::exception& e) {
        qDebug() << "Exception in getAllTimeSlots:" << e.what();
        return QVector<QTime>();
    } catch (...) {
        qDebug() << "Unknown exception in getAllTimeSlots";
        return QVector<QTime>();
    }
}

QJsonArray PadelDataManager::getAllTimeSlotsJson(int courtId) const {
    QJsonArray timeSlotsArray;

    try {
        if (courtId <= 0) {
            qDebug() << "Invalid court ID in getAllTimeSlotsJson:" << courtId;
            return timeSlotsArray;
        }

        Court court = getCourtById(courtId);
        if (court.getId() <= 0) {
            qDebug() << "Court not found in getAllTimeSlots (JSON version), ID:" << courtId;
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

        qDebug() << "Retrieved" << timeSlotsArray.size() << "time slots for court" << courtId;
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in getAllTimeSlots (JSON version):" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in getAllTimeSlots (JSON version)";
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
    try {
        QString filePath = QDir(dataDir).filePath("waitlists.json");
        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        QDir().mkpath(fileInfo.absolutePath());

        qDebug() << "Saving waitlists to file:" << filePath;

        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = "Could not open waitlists file for writing: " + file.errorString();
            qDebug() << errorMessage;
            return false;
        }

        QJsonDocument doc(waitlists);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

        qint64 bytesWritten = file.write(jsonData);
        if (bytesWritten == -1) {
            errorMessage = "Failed to write to waitlists file: " + file.errorString();
            file.close();
            qDebug() << errorMessage;
            return false;
        }

        file.flush(); 
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in writeWaitlistsToFile: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in writeWaitlistsToFile";
        qDebug() << errorMessage;
        return false;
    }
}

bool PadelDataManager::tryFillSlotFromWaitlist(int courtId, const QDateTime& startTime, const QDateTime& endTime, QString& errorMessage) {
    try {
        if (courtId <= 0 || !startTime.isValid() || !endTime.isValid()) {
            errorMessage = "Invalid parameters in tryFillSlotFromWaitlist";
            qDebug() << errorMessage;
            return false;
        }

        qDebug() << "Trying to fill slot from waitlist for court" << courtId 
                << "at time" << startTime.toString("yyyy-MM-dd HH:mm");

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            errorMessage = "Court not found in tryFillSlotFromWaitlist";
            qDebug() << errorMessage;
            return false;
        }

        auto it = courtWaitlists.find(courtId);
        if (it == courtWaitlists.end() || it->second.empty()) {

            qDebug() << "No waitlist entries found for court" << courtId;
            return false;
        }

        qDebug() << "Found waitlist for court" << courtId << " - checking entries...";

        QVector<WaitlistEntry> entries;
        std::queue<WaitlistEntry> tempQueue = it->second;

        while (!tempQueue.empty()) {
            entries.append(tempQueue.front());
            tempQueue.pop();
        }

        if (entries.isEmpty()) {
            qDebug() << "No waitlist entries found for court" << courtId << " after copy";
            return false;
        }

        qDebug() << "Found" << entries.size() << "entries in waitlist for court" << courtId;

        try {
            std::sort(entries.begin(), entries.end(), [](const WaitlistEntry& a, const WaitlistEntry& b) {
                if (a.isVIP != b.isVIP) {
                    return a.isVIP > b.isVIP;  
                }
                return a.priority > b.priority;  
            });

            int logLimit = qMin(3, entries.size());
            for (int i = 0; i < logLimit; i++) {
                qDebug() << "Waitlist entry" << i+1 << ": UserId=" << entries[i].memberId
                         << "VIP=" << entries[i].isVIP
                         << "Priority=" << entries[i].priority
                         << "Requested=" << entries[i].requestedTime.toString("yyyy-MM-dd HH:mm");
            }
        } catch (const std::exception& e) {
            errorMessage = QString("Exception during waitlist sorting: %1").arg(e.what());
            qDebug() << errorMessage;
            return false;
        }

        for (const WaitlistEntry& entry : entries) {
            try {

                qDebug() << "Considering waitlist entry: UserId=" << entry.memberId
                         << "RequestedTime=" << entry.requestedTime.toString()
                         << "AvailableTime=" << startTime.toString();

                if (entry.requestedTime.date() != startTime.date()) {
                    qDebug() << "Skipping entry - date mismatch:" 
                             << entry.requestedTime.date().toString() << "vs" 
                             << startTime.date().toString();
                    continue;
                }

                double hourDiff = std::abs(entry.requestedTime.secsTo(startTime)) / 3600.0;
                qDebug() << "Time difference in hours:" << hourDiff;

                if (hourDiff > 3.0) {
                    qDebug() << "Skipping entry - time difference too large (" << hourDiff << " hours)";
                    continue;
                }

                int userId = entry.memberId;  
                qDebug() << "Found suitable entry for user" << userId 
                         << "- requested:" << entry.requestedTime.toString()
                         << "available:" << startTime.toString();

                if (userHasBookingOnDate(userId, courtId, startTime.date())) {
                    qDebug() << "User" << userId << "already has a booking on this date and court, skipping";
                    continue;
                }

                qDebug() << "Creating booking from waitlist for user" << userId;

                QString bookingError;
                bool success = createBooking(userId, courtId, 
                                          startTime, 
                                          endTime, 
                                          bookingError,
                                          true);  

                if (success) {
                    qDebug() << "Successfully created booking from waitlist for user" << userId;

                    QString removeError;
                    if (removeFromWaitlist(userId, courtId, removeError)) {
                        qDebug() << "Successfully removed user" << userId << "from waitlist";
                    } else {
                        qDebug() << "Warning: Could not remove user from waitlist:" << removeError;
                    }

                    try {
                        emit waitlistBookingCreated(userId, courtId, startTime);
                        qDebug() << "Sent notification to user" << userId << "about waitlist booking";
                    } catch (const std::exception& e) {
                        qDebug() << "Error sending notification:" << e.what();
                    }

                    return true;
                }
                else {
                    qDebug() << "Failed to create booking from waitlist:" << bookingError;

                }
            } catch (const std::exception& e) {
                qDebug() << "Exception processing waitlist entry:" << e.what();

            } catch (...) {
                qDebug() << "Unknown exception processing waitlist entry";

            }
        }

        qDebug() << "No suitable waitlist entry found for the slot after checking all entries";
        return false;
    }
    catch (const std::exception& e) {
        errorMessage = QString("Exception in tryFillSlotFromWaitlist: %1").arg(e.what());
        qDebug() << errorMessage;
        return false;
    }
    catch (...) {
        errorMessage = "Unknown exception in tryFillSlotFromWaitlist";
        qDebug() << errorMessage;
        return false;
    }
}

QJsonObject PadelDataManager::getCourtDetails(int courtId) const {
    QJsonObject courtDetails;

    try {
        if (courtId <= 0) {
            qDebug() << "Invalid court ID in getCourtDetails:" << courtId;
            return courtDetails;
        }

        Court court = getCourtById(courtId);
        if (court.getId() <= 0) {
            qDebug() << "Court not found in getCourtDetails:" << courtId;
            return courtDetails;
        }

        courtDetails = courtToJson(court);

        courtDetails["availableTimeSlots"] = static_cast<int>(court.getAllTimeSlots().size());

        qDebug() << "Retrieved details for court" << courtId << "named" << court.getName();
    } 
    catch (const std::exception& e) {
        qDebug() << "Exception in getCourtDetails:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in getCourtDetails";
    }

    return courtDetails;
} 

void PadelDataManager::setMemberDataManager(MemberDataManager* memberManager) {
    memberDataManager = memberManager;
    qDebug() << "Member data manager has been set";
}

QVector<Court> PadelDataManager::getAvailableCourts(const QDateTime& startTime, const QDateTime& endTime, 
                                                  const QString& location) const {
    QMutexLocker locker(&mutex);
    QVector<Court> availableCourts;

    try {
        for (const auto& pair : courtsById) {
            const Court& court = pair.second;

            if (!location.isEmpty() && court.getLocation() != location) {
                continue;
            }

            if (isCourtAvailable(court.getId(), startTime, endTime)) {
                availableCourts.append(court);
            }
        }

        qDebug() << "Found" << availableCourts.size() << "available courts at" 
                << startTime.toString() << "to" << endTime.toString()
                << (location.isEmpty() ? "" : QString(" in %1").arg(location));
    } catch (const std::exception& e) {
        qDebug() << "Exception in getAvailableCourts:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in getAvailableCourts";
    }

    return availableCourts;
}

int PadelDataManager::getCurrentAttendees(int courtId, const QDateTime& startTime, const QDateTime& endTime) const {
    QMutexLocker locker(&mutex);
    int attendeeCount = 0;

    try {

        if (courtId <= 0 || !startTime.isValid() || !endTime.isValid()) {
            qDebug() << "Invalid parameters in getCurrentAttendees - courtId:" << courtId
                    << "startTime:" << startTime.toString()
                    << "endTime:" << endTime.toString();
            return 0;
        }

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            qDebug() << "Court not found in getCurrentAttendees:" << courtId;
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

        qDebug() << "Court" << courtId << "has" << attendeeCount << "attendees at" 
                << startTime.toString() << "to" << endTime.toString();
    } catch (const std::exception& e) {
        qDebug() << "Exception in getCurrentAttendees:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in getCurrentAttendees";
    }

    return attendeeCount;
}

QVector<Booking> PadelDataManager::getAllBookings() const {
    QMutexLocker locker(&mutex);
    QVector<Booking> allBookings;

    try {
        allBookings.reserve(bookingsById.size());
        for (const auto& pair : bookingsById) {
            allBookings.append(pair.second);
        }

        qDebug() << "Retrieved" << allBookings.size() << "bookings";
    } catch (const std::exception& e) {
        qDebug() << "Exception in getAllBookings:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in getAllBookings";
    }

    return allBookings;
}

QJsonArray PadelDataManager::getAvailableTimeSlots(int courtId, const QDate& date, int maxAttendees) const {

    bool locked = mutex.tryLock(200); 
    if (!locked) {
        qDebug() << "WARNING: Could not lock mutex in getAvailableTimeSlots, returning empty array";
        return QJsonArray();
    }

    QJsonArray availableSlots;

    try {
        qDebug() << "Getting available slots for court" << courtId << "on" << date.toString();

        if (courtId <= 0) {
            qDebug() << "ERROR: Invalid court ID in getAvailableTimeSlots:" << courtId;
            mutex.unlock();
            return availableSlots;
        }

        if (!date.isValid()) {
            qDebug() << "ERROR: Invalid date in getAvailableTimeSlots";
            mutex.unlock();
            return availableSlots;
        }

        int safeMaxAttendees = maxAttendees;
        if (safeMaxAttendees <= 0) {
            qDebug() << "ERROR: Invalid maxAttendees in getAvailableTimeSlots:" << maxAttendees;
            safeMaxAttendees = 2; 
            qDebug() << "Using default maxAttendees value: 2";
        }

        auto courtIt = courtsById.find(courtId);
        if (courtIt == courtsById.end()) {
            qDebug() << "ERROR: Court not found in getAvailableTimeSlots:" << courtId;
            mutex.unlock();
            return availableSlots;
        }

        Court court = courtIt->second;
        mutex.unlock(); 

        qDebug() << "Found court:" << court.getName() << "with price:" << court.getPricePerHour();

        std::vector<QTime> timeSlotsCopy = court.getAllTimeSlots();
        if (timeSlotsCopy.empty()) {
            qDebug() << "No time slots defined for court:" << courtId;
            return availableSlots;
        }

        qDebug() << "Retrieved" << timeSlotsCopy.size() << "time slots for court" << courtId;

        for (const QTime& time : timeSlotsCopy) {

            if (!time.isValid()) {
                qDebug() << "Skipping invalid time slot for court" << courtId;
                continue;
            }

            try {

                QDateTime startTime(date, time);
                QDateTime endTime = startTime.addSecs(3600); 

                int attendees = 0;
                try {
                    attendees = getCurrentAttendees(courtId, startTime, endTime);
                } catch (const std::exception& e) {
                    qDebug() << "ERROR: Exception getting current attendees:" << e.what();
                    continue; 
                } catch (...) {
                    qDebug() << "ERROR: Unknown exception getting current attendees";
                    continue; 
                }

                if (attendees < safeMaxAttendees) {

                    QJsonObject slotObj;
                    slotObj["startTime"] = time.toString("HH:mm");
                    slotObj["endTime"] = endTime.time().toString("HH:mm");
                    slotObj["currentAttendees"] = attendees;
                    slotObj["maxAttendees"] = safeMaxAttendees;
                    slotObj["availableSpots"] = safeMaxAttendees - attendees;

                    availableSlots.append(slotObj);
                    qDebug() << "Found available slot at" << time.toString("HH:mm") 
                            << "with" << attendees << "out of" << safeMaxAttendees << "attendees";
                }
            } catch (const std::exception& e) {
                qDebug() << "ERROR: Exception checking slot availability:" << e.what();

            } catch (...) {
                qDebug() << "ERROR: Unknown exception checking slot availability";

            }
        }

        qDebug() << "Found" << availableSlots.size() << "available time slots for court" 
                << courtId << "on" << date.toString();

        for (int i = 0; i < availableSlots.size() && i < 10; ++i) { 
            QJsonObject slot = availableSlots[i].toObject();
            qDebug() << "Slot" << i << ":" << slot["startTime"].toString() 
                    << "with" << slot["currentAttendees"].toInt() 
                    << "out of" << slot["maxAttendees"].toInt() << "attendees";
        }
    } catch (const std::exception& e) {
        qDebug() << "ERROR: Exception in getAvailableTimeSlots:" << e.what();

        if (mutex.tryLock(0)) {
            mutex.unlock();
        }
    } catch (...) {
        qDebug() << "ERROR: Unknown exception in getAvailableTimeSlots";

        if (mutex.tryLock(0)) {
            mutex.unlock();
        }
    }

    return availableSlots;
}

QJsonArray PadelDataManager::readCourtsFromFile(QString& errorMessage) const {
    QJsonArray courtsArray;
    QString filePath = dataDir + "/courts.json";

    try {
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
    } catch (const std::exception& e) {
        errorMessage = QString("Exception reading courts file: %1").arg(e.what());
        qDebug() << errorMessage;
    } catch (...) {
        errorMessage = "Unknown exception reading courts file";
        qDebug() << errorMessage;
    }

    return courtsArray;
}

QJsonArray PadelDataManager::readBookingsFromFile(QString& errorMessage) const {
    QJsonArray bookingsArray;
    QString filePath = dataDir + "/bookings.json";

    try {
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
    } catch (const std::exception& e) {
        errorMessage = QString("Exception reading bookings file: %1").arg(e.what());
        qDebug() << errorMessage;
    } catch (...) {
        errorMessage = "Unknown exception reading bookings file";
        qDebug() << errorMessage;
    }

    return bookingsArray;
}

Court PadelDataManager::jsonToCourt(const QJsonObject& json) {
    Court court;

    try {

        if (!json.contains("id") || !json.contains("name") || !json.contains("location")) {
            qDebug() << "Missing required fields in court JSON";
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
    } catch (const std::exception& e) {
        qDebug() << "Exception in jsonToCourt:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in jsonToCourt";
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

    qDebug() << "Timers set up for periodic booking status checks";
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
    try {
        qDebug() << "Checking booking status and processing waitlists...";

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

        qDebug() << "Processed waitlists for" << processedCourts.size() << "courts";

        if (dataModified) {
            try {
                saveToFile();
                qDebug() << "Saved waitlist changes to file";
            } catch (const std::exception& e) {
                qDebug() << "Error saving data in checkBookingStatus:" << e.what();
            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in checkBookingStatus:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in checkBookingStatus";
    }
}

void PadelDataManager::processWaitlistForDate(int courtId, const QDate& date) {
    try {
        qDebug() << "Processing waitlist for court" << courtId << "on date" << date.toString("yyyy-MM-dd");

        QVector<QTime> timeSlots = getAllTimeSlots(courtId);
        if (timeSlots.isEmpty()) {
            qDebug() << "No time slots defined for court" << courtId;
            return;
        }

        qDebug() << "Found" << timeSlots.size() << "time slots for court" << courtId;

        int filledSlots = 0;

        for (const QTime& time : timeSlots) {
            QDateTime startTime(date, time);
            QDateTime endTime = startTime.addSecs(3600); 

            if (startTime < QDateTime::currentDateTime()) {
                continue;
            }

            if (isCourtAvailable(courtId, startTime, endTime)) {
                qDebug() << "Slot available at" << startTime.toString() << "- checking waitlist";

                QString error;
                if (tryFillSlotFromWaitlist(courtId, startTime, endTime, error)) {
                    qDebug() << "Successfully filled slot from waitlist!";
                    filledSlots++;
                } else {
                    qDebug() << "No suitable waitlist entry for slot at" << startTime.toString() << ": " << error;
                }
            }
        }

        qDebug() << "Filled" << filledSlots << "slots from waitlist for court" << courtId << "on" << date.toString();
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in processWaitlistForDate:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in processWaitlistForDate";
    }
}