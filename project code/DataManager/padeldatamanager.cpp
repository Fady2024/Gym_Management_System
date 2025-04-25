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
    // Get the project directory path
    QString projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
    
    // Initialize data directory
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
    initializeFromFile();
    
    // Setup periodic checks
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
    
    // Read courts data
    QJsonArray courtsArray = readCourtsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading courts file:" << errorMessage;
        return false;
    }

    // Clear current data
    courtsById.clear();
    for (const QJsonValue& courtValue : courtsArray) {
        QJsonObject courtObj = courtValue.toObject();
        Court court = jsonToCourt(courtObj);
        
        // Store in map
        courtsById[court.getId()] = court;
        
        qDebug() << "Loaded court:" << court.getId() << court.getName() 
                << "Price:" << court.getPricePerHour();
    }

    qDebug() << "Loaded" << courtsById.size() << "courts from file";

    // Read bookings
    QJsonArray bookingsArray = readBookingsFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading bookings file:" << errorMessage;
        return false;
    }

    // Clear current bookings
    bookingsById.clear();
    
    // Process bookings
    for (const QJsonValue& bookingValue : bookingsArray) {
        QJsonObject bookingObj = bookingValue.toObject();
        Booking booking = jsonToBooking(bookingObj);
        bookingsById[booking.getBookingId()] = booking;
    }
    
    qDebug() << "Loaded" << bookingsById.size() << "bookings from file";

    return true;
}

bool PadelDataManager::saveToFile() {
    // Use tryLock instead of QMutexLocker to avoid deadlocks
    bool locked = mutex.tryLock(200);
    
    if (!locked) {
        qDebug() << "Warning: Could not lock mutex in saveToFile, will retry later";
        return true;
    }
    
    bool result = true;
    
    try {
    if (!dataModified) {
            mutex.unlock();
        return true;  // Nothing to save
    }

        QString errorMessage;
        QDir dataDirectory(dataDir);
        
        // Make sure the directory exists
        if (!dataDirectory.exists()) {
            qDebug() << "Creating data directory at:" << dataDir;
            QDir().mkpath(dataDir);
        }
            
        // Save courts
        QJsonArray courtsArray;
        for (const auto& pair : courtsById) {
            courtsArray.append(courtToJson(pair.second));
        }

        if (!writeCourtsToFile(courtsArray, errorMessage)) {
            qDebug() << "Error saving courts:" << errorMessage;
            result = false;
        }
        else {
        // Save bookings
        QJsonArray bookingsArray;
        for (const auto& pair : bookingsById) {
            bookingsArray.append(bookingToJson(pair.second));
        }

            if (!writeBookingsToFile(bookingsArray, errorMessage)) {
                qDebug() << "Error saving bookings:" << errorMessage;
                result = false;
            }
            else {
                dataModified = false;
                qDebug() << "Successfully saved padel data to files";
            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in saveToFile:" << e.what();
        result = false;
    }
    catch (...) {
        qDebug() << "Unknown exception in saveToFile";
        result = false;
    }
    mutex.unlock();
    return result;
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
        
        file.flush(); // Make sure data is written to disk
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
        
        // Make sure the directory exists
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
        
        file.flush(); // Make sure data is written to disk
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
    
    // Add description if not empty
    if (!court.getDescription().isEmpty()) {
        json["description"] = court.getDescription();
    }
    
    // Add features if available
    const QStringList& features = court.getFeatures();
    if (!features.isEmpty()) {
        QJsonArray featuresArray;
        for (const QString& feature : features) {
            featuresArray.append(feature);
        }
        json["features"] = featuresArray;
    }
    
    // Add time slots
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
    
    return json;
}

Booking PadelDataManager::jsonToBooking(const QJsonObject& json) {
    int bookingId = json["id"].toInt();
    int courtId = json["courtId"].toInt();
    int userId = json["userId"].toInt();
    
    // Create an empty court (we'll set just the ID)
    Court court;
    court.setId(courtId);
    
    // Create user object with just the ID
    User user;
    user.setId(userId);
    
    // Parse date/time
    QDateTime startTime = QDateTime::fromString(json["startTime"].toString(), Qt::ISODate);
    QDateTime endTime = QDateTime::fromString(json["endTime"].toString(), Qt::ISODate);
    
    // Create booking
    Booking booking(bookingId, court, startTime, endTime, user);
    
    // Set additional properties
    booking.setPrice(json["price"].toDouble());
    booking.setVip(json["isVip"].toBool(false));
    
    if (json["isCancelled"].toBool(false)) {
        booking.cancel();
    }
    
    return booking;
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
    try {
        if (courtId <= 0) {
            qDebug() << "Invalid court ID in getCourtById:" << courtId;
            return Court(); // Return empty court
        }
        // Create a safe mutex lock
        QMutex& mutexRef = mutex;
        QMutexLocker locker(&mutexRef);
        
        // First check if the map is empty to avoid unnecessary operations
        if (courtsById.empty()) {
            qDebug() << "Courts map is empty in getCourtById";
            return Court(); // Return empty court
        }
        
        // Safe court lookup
    auto it = courtsById.find(courtId);
    if (it != courtsById.end()) {
            Court result = it->second; // Make a copy
            qDebug() << "Found court:" << result.getName() << "with price:" << result.getPricePerHour();
            return result;
    }
        
        // Return an empty court object if not found
        qDebug() << "Court not found with ID:" << courtId;
    return Court();
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in getCourtById:" << e.what();
        return Court(); // Return empty court on error
    }
    catch (...) {
        qDebug() << "Unknown exception in getCourtById";
        return Court(); // Return empty court on error
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

// Booking management methods
bool PadelDataManager::createBooking(int userId, int courtId, const QDateTime& startTime, 
                                   const QDateTime& endTime, QString& errorMessage) {
    try {
        qDebug() << "Creating booking for user" << userId << "court" << courtId
                << "from" << startTime.toString() << "to" << endTime.toString();
    
        // Validate user ID
        if (userId <= 0) {
            errorMessage = "Invalid user ID";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }
        
        // Validate court ID
        if (courtId <= 0) {
        errorMessage = "Invalid court ID";
            qDebug() << "Booking failed:" << errorMessage;
        return false;
    }

        // Use mutex lock to protect access to data
        QMutexLocker locker(&mutex);
        
        // Check court exists
        bool courtExists = false;
        {
            courtExists = (courtsById.find(courtId) != courtsById.end());
        }
        
        if (!courtExists) {
            errorMessage = "Court not found with ID: " + QString::number(courtId);
            qDebug() << "Booking failed:" << errorMessage;
        return false;
    }

        // Get the court (safely)
        Court court;
        try {
            court = courtsById.at(courtId);
            qDebug() << "Found court:" << court.getName() << "at location:" << court.getLocation();
        }
        catch (const std::exception& e) {
            errorMessage = QString("Exception getting court: %1").arg(e.what());
            qDebug() << "Booking failed:" << errorMessage;
        return false;
    }

    // Validate booking time
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
        
        // Check if the court is available during the requested time
        qDebug() << "Checking court availability...";
        
        bool available = true;
            for (const auto& pair : bookingsById) {
                const Booking& existingBooking = pair.second;
            
            // Skip cancelled bookings
            if (existingBooking.isCancelled()) {
                continue;
            }
            
            if (existingBooking.getCourtId() == courtId) {
                qDebug() << "Found existing booking:" << existingBooking.getBookingId();
                
                // Check time overlap using simple condition
                if (!(endTime <= existingBooking.getStartTime() || 
                      startTime >= existingBooking.getEndTime())) {
                    qDebug() << "Conflict with booking ID:" << existingBooking.getBookingId();
                    available = false;
                        break;
                    }
                }
            }
        
        if (!available) {
            errorMessage = "Court is not available at the selected time";
            qDebug() << "Booking failed:" << errorMessage;
            return false;
        }
        
        qDebug() << "Court is available for the requested time";
        
        // Check if user exists and get user data
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
        
        // Generate new booking ID
        qDebug() << "Generating booking ID...";
    int bookingId = generateBookingId();
        qDebug() << "Generated new booking ID:" << bookingId;
        
        // Create new booking
        qDebug() << "Creating booking object...";
        
        Booking newBooking;
        newBooking.setBookingId(bookingId);
        newBooking.setCourt(court);
        newBooking.setStartTime(startTime);
        newBooking.setEndTime(endTime);
        newBooking.setUser(user);
        
        // Check if user is VIP
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
            // Non-critical error, continue with isVip = false
        }
        
        // Calculate price - use a try-catch to handle potential errors
        qDebug() << "Calculating booking price...";
        double price = 0.0;
        
        try {
            price = calculateBookingPrice(courtId, startTime, endTime, isVip);
            qDebug() << "Calculated booking price:" << price;
        } catch (const std::exception& e) {
            qDebug() << "Exception calculating price:" << e.what();
            price = 100.0; // Default price on error
        }
        
        newBooking.setPrice(price);
        newBooking.setVip(isVip);
    
        // Add booking to the system
        qDebug() << "Adding booking to the system...";
        bookingsById[bookingId] = newBooking;
        qDebug() << "Added booking to bookings map";
        
        // Update data
    dataModified = true;
    
        qDebug() << "Booking created successfully with ID:" << bookingId;
        
        // Save changes to file
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

// Safe emit method that will be called through QMetaObject::invokeMethod
void PadelDataManager::safeEmitBookingCreated(int bookingId, int userId) {
    qDebug() << "Safe emit of bookingCreated signal for booking ID:" << bookingId << "user ID:" << userId;
    emit bookingCreated(bookingId, userId);
}

double PadelDataManager::calculateBookingPrice(int courtId, const QDateTime& startTime, 
                                             const QDateTime& endTime, bool isVIP) const {
    qDebug() << "Inside calculateBookingPrice for court:" << courtId;
    
    try {
        // Get the court price information BEFORE any calculations
        double pricePerHour = 100.0; // Default price if court not found
        
        {
            // Use a try-lock approach to avoid deadlock
            bool locked = mutex.tryLock(100); // Try to lock with timeout
            if (locked) {
                // Safe access to the map
                auto it = courtsById.find(courtId);
                if (it != courtsById.end()) {
                    pricePerHour = it->second.getPricePerHour();
                    qDebug() << "Got price from court object (locked): " << pricePerHour;
                } else {
                    qDebug() << "Court not found, using default price: " << pricePerHour;
                }
                mutex.unlock(); // Explicitly unlock
            } else {
                // Couldn't lock, use default price
                qDebug() << "Could not lock mutex, using default price: " << pricePerHour;
            }
        }
        
        // Calculate duration in hours (outside of any locks)
        int durationInSeconds = startTime.secsTo(endTime);
        double durationInHours = static_cast<double>(durationInSeconds) / 3600.0;
        qDebug() << "Duration in hours: " << durationInHours;
        
        // Calculate final price
        double basePrice = pricePerHour * durationInHours;
        double finalPrice = basePrice;
        
    if (isVIP) {
            finalPrice = basePrice * 0.85; // 15% discount
            qDebug() << "Applied VIP discount. Final price: " << finalPrice;
        } else {
            qDebug() << "Regular price (no discount): " << finalPrice;
        }
        
        return finalPrice;
    }
    catch (const std::exception& e) {
        qDebug() << "Error in calculateBookingPrice: " << e.what();
        return 100.0; // Safe default on error
    }
    catch (...) {
        qDebug() << "Unknown error in calculateBookingPrice";
        return 100.0; // Safe default on error
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
    // Use tryLock instead of QMutexLocker to avoid deadlocks
    bool locked = mutex.tryLock(200); // Try to lock with timeout
    
    if (!locked) {
        qDebug() << "Warning: Could not lock mutex in cancelBooking, will retry later";
        errorMessage = "System busy, please try again";
        return false;
    }
    
    bool result = false;
    
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
        
        // Check if cancellation is allowed without calling other mutex-locking functions
    QDateTime now = QDateTime::currentDateTime();
        const Booking& booking = it->second;
        bool canCancel = now.secsTo(booking.getStartTime()) >= 3 * 60 * 60; // 3 hours
        
        if (!canCancel) {
            errorMessage = "Cancellation is not allowed within 3 hours of booking time";
            qDebug() << "Cancellation failed:" << errorMessage;
            mutex.unlock();
        return false;
    }

        Booking& bookingRef = it->second;
        bookingRef.cancel();
        
        int memberId = bookingRef.getUserId();
        
    dataModified = true;
        result = true;
        
        // Emit signal safely
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
    
    // Always release the lock
    mutex.unlock();
    return result;
}

// Safe emit method for bookingCancelled signal
void PadelDataManager::safeEmitBookingCancelled(int bookingId, int userId) {
    qDebug() << "Safe emit of bookingCancelled signal for booking ID:" << bookingId << "user ID:" << userId;
    emit bookingCancelled(bookingId, userId);
}

bool PadelDataManager::rescheduleBooking(int bookingId, const QDateTime& newStartTime, 
                                       const QDateTime& newEndTime, QString& errorMessage) {
    // Use tryLock instead of QMutexLocker to avoid deadlocks
    bool locked = mutex.tryLock(200); // Try to lock with timeout
    
    if (!locked) {
        qDebug() << "Warning: Could not lock mutex in rescheduleBooking, will retry later";
        errorMessage = "System busy, please try again";
        return false;
    }
    
    bool result = false;
    
    try {
    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
        errorMessage = "Booking not found";
            mutex.unlock();
        return false;
    }

    Booking& booking = it->second;
    QDateTime now = QDateTime::currentDateTime();
    
        // Check if rescheduling is allowed
    if (now.secsTo(booking.getStartTime()) < 3 * 60 * 60) { // 3 hours
        errorMessage = "Cannot reschedule booking less than 3 hours before start time";
            mutex.unlock();
        return false;
    }

    // Validate new time
    if (!validateBookingTime(newStartTime, newEndTime, errorMessage)) {
            mutex.unlock();
        return false;
    }

        // Check court availability without calling functions that lock the mutex
        int courtId = booking.getCourtId();
        bool available = true;
        for (const auto& pair : bookingsById) {
            const Booking& existingBooking = pair.second;
            
            // Skip cancelled bookings and the booking being rescheduled
            if (existingBooking.isCancelled() || existingBooking.getBookingId() == bookingId) {
                continue;
            }
            
            if (existingBooking.getCourtId() == courtId) {
                // Check time overlap
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
    
    // Always unlock the mutex
    mutex.unlock();
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
    // Using a simplified version of availability check without QMutexLocker to reduce code complexity
    qDebug() << "Inside isCourtAvailable for court:" << courtId;
    
    // Check if court exists
    if (courtsById.find(courtId) == courtsById.end()) {
        qDebug() << "Court not found with ID:" << courtId;
        return false;
    }
    
    qDebug() << "Court found, checking time validity";
    
    // Verify time validity
    if (!startTime.isValid() || !endTime.isValid() || startTime >= endTime) {
        qDebug() << "Invalid time range:" << startTime.toString() << " - " << endTime.toString();
        return false;
    }
    
    qDebug() << "Time is valid, checking for conflicts with existing bookings";
    
    // Check for absence of overlapping bookings - using a simpler approach
    bool isAvailable = true;
    
    for (const auto& pair : bookingsById) {
        const Booking& existingBooking = pair.second;
        
        // Skip cancelled bookings
        if (existingBooking.isCancelled()) {
            continue;
        }
        
        // Check only bookings for this court, using getCourtId instead of getCourt().getId()
        if (existingBooking.getCourtId() == courtId) {
            qDebug() << "Found existing booking:" << existingBooking.getBookingId() 
                    << "at" << existingBooking.getStartTime().toString();
            
            // Very simple check - is there overlap in times
            if (!(endTime <= existingBooking.getStartTime() || 
                  startTime >= existingBooking.getEndTime())) {
                qDebug() << "Conflict found with booking ID:" << existingBooking.getBookingId();
                isAvailable = false;
                break;
            }
        }
    }
    
    qDebug() << "Court availability result:" << isAvailable;
    return isAvailable;
}

// Update validateCourtAvailability function to be simpler as well
bool PadelDataManager::validateCourtAvailability(int courtId, const QDateTime& startTime, 
                                              const QDateTime& endTime) const {
    // Use simplified version to reduce possibility of errors
    qDebug() << "Inside validateCourtAvailability for court:" << courtId;
    
    // Use isCourtAvailable directly to avoid code duplication
    return isCourtAvailable(courtId, startTime, endTime);
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
    if (memberId <= 0) {
        qDebug() << "Invalid member ID in isVIPMember:" << memberId;
        return false;
    }
    
    try {
        // Create a copy of mutex reference to avoid potential issues
        QMutex& mutexRef = mutex;
        QMutexLocker locker(&mutexRef);
        
        qDebug() << "Checking VIP status for member ID:" << memberId;
        
        // Use find() instead of direct access to avoid potential exceptions
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
    if (!file.exists()) {
        qDebug() << "Courts file does not exist at:" << QDir(dataDir).filePath("courts.json");
        return QJsonArray();
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open courts file for reading: " + file.errorString();
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing courts file: " + parseError.errorString();
        return QJsonArray();
    }
    
    if (!doc.isArray()) {
        errorMessage = "Courts file does not contain a valid JSON array";
        return QJsonArray();
    }

    return doc.array();
}

QJsonArray PadelDataManager::readBookingsFromFile(QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("bookings.json"));
    if (!file.exists()) {
        // No bookings yet, not an error
        return QJsonArray();
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open bookings file for reading";
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing bookings file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
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

// Add implementation for getAllBookings method
QVector<Booking> PadelDataManager::getAllBookings() const {
    QMutexLocker locker(&mutex);
    QVector<Booking> result;
    
    for (const auto& pair : bookingsById) {
        result.append(pair.second);
    }
    
    return result;
}

// Add implementation for suggestNextSlots method
QVector<QDateTime> PadelDataManager::suggestNextSlots(int courtId, const QDateTime& fromTime) const {
    QMutexLocker locker(&mutex);
    QVector<QDateTime> suggestions;
    
    // Try to find the court
    if (!courtsById.count(courtId)) {
        return suggestions;
    }
    
    const Court& court = courtsById.at(courtId);
    QDate date = fromTime.date();
    
    // Check availability for the next 7 days
    for (int day = 0; day < 7; ++day) {
        QDate checkDate = date.addDays(day);
        
        // Get available slots for this court on this date
        QVector<QTime> availableSlots = getAvailableTimeSlots(courtId, checkDate);
        
        // Add each available slot as a suggestion
        for (const QTime& time : availableSlots) {
            suggestions.append(QDateTime(checkDate, time));
            
            // If we have enough suggestions, return
            if (suggestions.size() >= 5) {
                return suggestions;
            }
        }
    }
    
    return suggestions;
}

Court PadelDataManager::jsonToCourt(const QJsonObject& json) {
    int id = json["id"].toInt();
    QString name = json["name"].toString();
    QString location = json["location"].toString();
    bool isIndoor = json["isIndoor"].toBool();
    double pricePerHour = json["pricePerHour"].toDouble();
    
    Court court(id, name, location, isIndoor, pricePerHour);
    
    // Load description if available
    if (json.contains("description")) {
        court.setDescription(json["description"].toString());
    }
    
    // Load features if available
    if (json.contains("features") && json["features"].isArray()) {
        QJsonArray featuresArray = json["features"].toArray();
        QStringList features;
        
        for (const QJsonValue& value : featuresArray) {
            features.append(value.toString());
        }
        
        court.setFeatures(features);
    }
    
    // Load time slots if available
    if (json.contains("timeSlots") && json["timeSlots"].isArray()) {
        QJsonArray slotsArray = json["timeSlots"].toArray();
        std::vector<QTime> timeSlots;
        
        for (const QJsonValue& value : slotsArray) {
            QString timeStr = value.toString();
            QTime time = QTime::fromString(timeStr, "HH:mm");
            
            if (time.isValid()) {
                timeSlots.push_back(time);
            }
        }
        
        // Get the time slots vector by reference and add all slots
        auto& courtTimeSlots = court.getAllTimeSlots();
        courtTimeSlots = timeSlots;
    }
    
    return court;
}

bool PadelDataManager::canCancelOrReschedule(int bookingId) const {
    QMutexLocker locker(&mutex);
    
    auto it = bookingsById.find(bookingId);
    if (it == bookingsById.end()) {
                return false;
            }
    
    const Booking& booking = it->second;
    QDateTime now = QDateTime::currentDateTime();
    
    // Allow cancellation or rescheduling at least 3 hours before start time
    return now.secsTo(booking.getStartTime()) >= 3 * 60 * 60; // 3 hours
}

bool PadelDataManager::findNextAvailableSlot(int courtId, QDateTime& suggestedTime, QString& errorMessage) {
    qDebug() << "Inside findNextAvailableSlot for court:" << courtId << "from time:" << suggestedTime.toString();
    
    QMutexLocker locker(&this->mutex);
    
    // Validate court exists
    if (this->courtsById.find(courtId) == this->courtsById.end()) {
        errorMessage = "Court not found";
        qDebug() << "Court not found with ID:" << courtId;
        return false;
    }
    
    // Get the court
    const Court& court = this->courtsById.at(courtId);
    
    QDateTime currentTime = suggestedTime;
    QDateTime maxSearchTime = suggestedTime.addDays(7);
    
    // Determine the slot duration in seconds (assume 1 hour slots)
    const int slotDurationSecs = 60 * 60;
    
    // Normalize suggested time to the nearest hour
    QTime timeComponent = currentTime.time();
    int minutes = timeComponent.minute();
    if (minutes > 0) {
        currentTime = currentTime.addSecs((60 - minutes) * 60);
    }
    
    while (currentTime < maxSearchTime) {
        // Check if this time slot works
        QDateTime endTime = currentTime.addSecs(slotDurationSecs);
        
        // Check if court is available at this time
        if (this->isCourtAvailable(courtId, currentTime, endTime)) {
            // Check if the court has this time slot available
            QVector<QTime> availableSlots = this->getAvailableTimeSlots(courtId, currentTime.date());
            bool slotFound = false;
            
            for (const QTime& slot : availableSlots) {
                QDateTime slotDateTime(currentTime.date(), slot);
                if (slotDateTime.time().hour() == currentTime.time().hour()) {
                    slotFound = true;
                    break;
                }
            }
            
            if (slotFound) {
                suggestedTime = currentTime;
                qDebug() << "Found available slot at:" << suggestedTime.toString();
    return true;
            }
        }
        currentTime = currentTime.addSecs(slotDurationSecs);
    }
    
    errorMessage = "No available slots found within the next 7 days";
    qDebug() << errorMessage;
    return false;
} 