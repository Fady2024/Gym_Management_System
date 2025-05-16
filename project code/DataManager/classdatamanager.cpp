#include "classdatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>

ClassDataManager::ClassDataManager(QObject* parent)
    : QObject(parent) {
    QString projectDir;
    
#ifdef FORCE_SOURCE_DIR
    projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
#else
    projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
#endif
    
    dataDir = projectDir + "/project code/Data";

    QFile classesFile(dataDir + "/classes.json");
    if (!classesFile.exists()) {
        classesFile.open(QIODevice::WriteOnly);
        classesFile.write("[]");
        classesFile.close();
    }
    
    initializeFromFile();
}

ClassDataManager::~ClassDataManager() {
    handleApplicationClosing();
}

void ClassDataManager::handleApplicationClosing() {
    if (dataModified) {
        saveToFile();
    }
}

bool ClassDataManager::initializeFromFile() {
    QString errorMessage;
    QJsonArray classesArray = readClassesFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        return false;
    }

    classesById.clear();

    for (const QJsonValue& classValue : classesArray) {
        Class gymClass = jsonToClass(classValue.toObject());
        classesById[gymClass.getId()] = gymClass;
    }

    return true;
}

bool ClassDataManager::saveToFile() {
    QJsonArray classesArray;
    for (const auto& pair : classesById) {
        classesArray.append(classToJson(pair.second));
    }

    QString errorMessage;
    bool success = writeClassesToFile(classesArray, errorMessage);
    if (!success) {
        return false;
    }

    dataModified = false;
    return true;
}

bool ClassDataManager::addClass(const Class& gymClass, QString& errorMessage) {
    if (gymClass.getId() != 0) {
        errorMessage = "Class already has an ID (" + QString::number(gymClass.getId()) + ")";
        return false;
    }

    Class newClass = gymClass;
    int newId = generateClassId();
    newClass.setId(newId);
    classesById[newId] = newClass;
    dataModified = true;
    return true;
}
bool ClassDataManager::updateClass(const Class& gymClass, QString& errorMessage) {
    if (gymClass.getId() == 0) {
        errorMessage = "Invalid class ID";
        return false;
    }

    auto it = classesById.find(gymClass.getId());
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    classesById[gymClass.getId()] = gymClass;
    dataModified = true;
    return true;
}

bool ClassDataManager::deleteClass(int classId, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    classesById.erase(it);
    dataModified = true;
    return true;
}

Class ClassDataManager::getClassById(int classId) const {
    auto it = classesById.find(classId);
    if (it != classesById.end()) {
        return it->second;
    }
    return Class();
}

QVector<Class> ClassDataManager::getAllClasses() const {
    QVector<Class> result;
    result.reserve(classesById.size());
    for (const auto& pair : classesById) {
        result.append(pair.second);
    }
    return result;
}

QVector<Class> ClassDataManager::getClassesByCoach(const QString& coachName) const {
    QVector<Class> result;
    for (const auto& pair : classesById) {
        if (pair.second.getCoachName() == coachName) {
            result.append(pair.second);
        }
    }
    return result;
}

QVector<Class> ClassDataManager::getClassesByDate(const QDate& date) const {
    QVector<Class> result;
    for (const auto& pair : classesById) {
        if (pair.second.hasSessionOnDate(date)) {
            result.append(pair.second);
        }
    }
    return result;
}

bool ClassDataManager::addToWaitlist(int classId, int memberId, bool isVIP, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    Class& gymClass = it->second;
    if (gymClass.isInWaitlist(memberId)) {
        errorMessage = "Member already in waitlist";
        return false;
    }

    gymClass.addToWaitlist(memberId);
    dataModified = true;
    return true;
}

bool ClassDataManager::removeFromWaitlist(int classId, int memberId, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    Class& gymClass = it->second;
    gymClass.removeFromWaitlist(memberId);
    dataModified = true;
    return true;
}

int ClassDataManager::getNextWaitlistMember(int classId) const {
    auto it = classesById.find(classId);
    if (it != classesById.end()) {
        return it->second.getNextWaitlistMember();
    }
    return -1;
}

QVector<int> ClassDataManager::getWaitlist(int classId) const {
    auto it = classesById.find(classId);
    if (it != classesById.end()) {
        std::deque<int> waitlist = it->second.getWaitlist();
        QVector<int> result;
        result.reserve(static_cast<int>(waitlist.size()));
        for (int id : waitlist) {
            result.append(id);
        }
        return result;
    }
    return QVector<int>();
}

bool ClassDataManager::addSession(int classId, const QDate& date, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    Class& gymClass = it->second;
    gymClass.addSession(date);
    dataModified = true;
    return true;
}

bool ClassDataManager::removeSession(int classId, const QDate& date, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    Class& gymClass = it->second;
    gymClass.removeSession(date);
    dataModified = true;
    return true;
}

QVector<QDate> ClassDataManager::getClassSessions(int classId) const {
    auto it = classesById.find(classId);
    if (it != classesById.end()) {
        std::queue<QDate> sessions = it->second.getSessions();
        QVector<QDate> result;
        
        std::queue<QDate> tempQueue = sessions;
        while (!tempQueue.empty()) {
            result.append(tempQueue.front());
            tempQueue.pop();
        }
        
        return result;
    }
    return QVector<QDate>();
}

bool ClassDataManager::enrollMember(int classId, int memberId, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    if (!memberDataManager || !memberDataManager->isSubscriptionActive(memberId)) {
        errorMessage = "Only active members can enroll in classes";
        return false;
    }

    // Check if member is already enrolled in any class
    for (const auto& pair : classesById) {
        if (pair.second.isMemberEnrolled(memberId)) {
            errorMessage = "You are already enrolled in another class. Please unenroll from it first.";
            return false;
        }
    }

    Class& gymClass = it->second;
    if (gymClass.isFull()) {
        if (memberDataManager->isVIPMember(memberId)) {
            bool foundNonVIP = false;
            for (const auto& enrolled : gymClass.getEnrolledMembers()) {
                if (!memberDataManager->isVIPMember(enrolled)) {
                    gymClass.removeMember(enrolled);
                    foundNonVIP = true;
                    break;
                }
            }
            if (!foundNonVIP) {
                errorMessage = "Class is full (even for VIP members)";
                return false;
            }
        } else {
            errorMessage = "Class is full";
            return false;
        }
    }

    // Update member's class ID in MemberDataManager
    Member member = memberDataManager->getMemberById(memberId);
    member.setClassId(classId);
    QString memberUpdateError;
    if (!memberDataManager->updateMember(member, memberUpdateError)) {
        errorMessage = "Failed to update member data: " + memberUpdateError;
        return false;
    }

    gymClass.addMember(memberId);
    gymClass.setNumOfEnrolled(gymClass.getNumOfEnrolled() + 1);
    dataModified = true;
    return true;
}

bool ClassDataManager::unenrollMember(int classId, int memberId, QString& errorMessage) {
    auto it = classesById.find(classId);
    if (it == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    Class& gymClass = it->second;

    // Update member's class ID in MemberDataManager
    Member member = memberDataManager->getMemberById(memberId);
    member.setClassId(-1); // -1 indicates no class
    QString memberUpdateError;
    if (!memberDataManager->updateMember(member, memberUpdateError)) {
        errorMessage = "Failed to update member data: " + memberUpdateError;
        return false;
    }

    gymClass.removeMember(memberId);
    int newCount = gymClass.getNumOfEnrolled() - 1;
    gymClass.setNumOfEnrolled(qMax(0, newCount));

    dataModified = true;
    return true;
}

bool ClassDataManager::isClassFull(int classId) const {
    auto it = classesById.find(classId);
    if (it != classesById.end()) {
        return it->second.isFull();
    }
    return false;
}

int ClassDataManager::getEnrolledCount(int classId) const {
    auto it = classesById.find(classId);
    if (it != classesById.end()) {
        return it->second.getNumOfEnrolled();
    }
    return 0;
}

QJsonArray ClassDataManager::readClassesFromFile(QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("classes.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open classes file for reading";
        return QJsonArray();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing classes file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}

bool ClassDataManager::writeClassesToFile(const QJsonArray& classes, QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("classes.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open classes file for writing";
        return false;
    }

    QJsonDocument doc(classes);
    file.write(doc.toJson());
    file.close();
    return true;
}

QJsonObject ClassDataManager::classToJson(const Class& gymClass) {
    QJsonObject json;
    json["id"] = gymClass.getId();
    json["className"] = gymClass.getClassName();
    json["coachName"] = gymClass.getCoachName();
    json["from"] = gymClass.getFromDate().toString();
    json["to"] = gymClass.getToDate().toString();
    json["capacity"] = gymClass.getCapacity();
    json["numOfEnrolled"] = gymClass.getNumOfEnrolled();

    QJsonArray sessionsArray;
    std::queue<QDate> sessions = gymClass.getSessions();
    std::queue<QDate> tempQueue = sessions;
    
    while (!tempQueue.empty()) {
        sessionsArray.append(tempQueue.front().toString(Qt::ISODate));
        tempQueue.pop();
    }
    
    json["sessions"] = sessionsArray;

    return json;
}

Class ClassDataManager::jsonToClass(const QJsonObject& json) {
    Class gymClass;
    gymClass.setId(json["id"].toInt());
    gymClass.setClassName(json["className"].toString());
    gymClass.setCoachName(json["coachName"].toString());
    gymClass.setFromDate(QDate::fromString(json["from"].toString()));
    gymClass.setToDate(QDate::fromString(json["to"].toString()));
    gymClass.setCapacity(json["capacity"].toInt());
    gymClass.setNumOfEnrolled(json["numOfEnrolled"].toInt());

    QJsonArray sessionsArray = json["sessions"].toArray();
    for (const QJsonValue& dateValue : sessionsArray) {
        gymClass.addSession(QDate::fromString(dateValue.toString(), Qt::ISODate));
    }

    return gymClass;
}

int ClassDataManager::generateClassId() const {
    int maxId = 0;
    for (const auto& pair : classesById) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }
    return maxId + 1;
}

bool ClassDataManager::recordAttendance(int classId, int memberId, const QDate& date, bool attended, double amountPaid, QString& errorMessage) {
    if (classesById.find(classId) == classesById.end()) {
        errorMessage = "Class not found";
        return false;
    }

    AttendanceRecord record;
    record.classId = classId;
    record.memberId = memberId;
    record.date = date;
    record.attended = attended;
    record.amountPaid = amountPaid;

    attendanceRecords.push_back(record);
    dataModified = true;

    return saveAttendanceRecords();
}

QVector<AttendanceRecord> ClassDataManager::getAttendanceRecords(int classId, const QDate& startDate, const QDate& endDate) const {
    QVector<AttendanceRecord> result;
    for (const auto& record : attendanceRecords) {
        if (record.classId == classId && 
            record.date >= startDate && 
            record.date <= endDate) {
            result.append(record);
        }
    }
    return result;
}

int ClassDataManager::getAttendanceCount(int classId, const QDate& date) const {
    int count = 0;
    for (const auto& record : attendanceRecords) {
        if (record.classId == classId && 
            record.date == date && 
            record.attended) {
            count++;
        }
    }
    return count;
}

double ClassDataManager::getClassRevenue(int classId, const QDate& startDate, const QDate& endDate) const {
    double revenue = 0.0;
    for (const auto& record : attendanceRecords) {
        if (record.classId == classId && 
            record.date >= startDate && 
            record.date <= endDate) {
            revenue += record.amountPaid;
        }
    }
    return revenue;
}

MonthlyReport ClassDataManager::generateMonthlyReport(const QDate& month) const {
    MonthlyReport report;
    report.month = month;
    report.totalActiveMembers = 0;
    report.totalClassesHeld = 0;
    report.totalAttendance = 0;
    report.totalRevenue = 0.0;

    QDate startDate(month.year(), month.month(), 1);
    QDate endDate = startDate.addMonths(1).addDays(-1);

    std::set<int> activeMembers;

    for (const auto& classPair : classesById) {
        const Class& gymClass = classPair.second;
        QString className = gymClass.getClassName();
        
        QVector<AttendanceRecord> classRecords = getAttendanceRecords(gymClass.getId(), startDate, endDate);
        
        if (!classRecords.isEmpty()) {
            report.totalClassesHeld++;
            
            int classAttendance = 0;
            double classRevenue = 0.0;
            
            for (const auto& record : classRecords) {
                if (record.attended) {
                    classAttendance++;
                    activeMembers.insert(record.memberId);
                    classRevenue += record.amountPaid;
                }
            }
            
            report.totalAttendance += classAttendance;
            report.totalRevenue += classRevenue;
            
            report.classAttendance.append(qMakePair(className, classAttendance));
            report.classRevenue.append(qMakePair(className, classRevenue));
        }
    }

    report.totalActiveMembers = static_cast<int>(activeMembers.size());
    return report;
}

bool ClassDataManager::saveMonthlyReport(const MonthlyReport& report, QString& errorMessage) const {
    QFile file(QDir(dataDir).filePath("monthly_reports.json"));
    if (!file.open(QIODevice::ReadWrite)) {
        errorMessage = "Could not open monthly reports file";
        return false;
    }

    QJsonArray reportsArray;
    if (file.size() > 0) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            reportsArray = doc.array();
        }
    }

    reportsArray.append(monthlyReportToJson(report));
    
    file.resize(0);
    file.write(QJsonDocument(reportsArray).toJson());
    file.close();
    
    return true;
}

QVector<MonthlyReport> ClassDataManager::getMonthlyReports(const QDate& startDate, const QDate& endDate) const {
    QVector<MonthlyReport> result;
    QFile file(QDir(dataDir).filePath("monthly_reports.json"));
    
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return result;
    }

    QJsonArray reportsArray = doc.array();
    for (const QJsonValue& value : reportsArray) {
        MonthlyReport report = jsonToMonthlyReport(value.toObject());
        if (report.month >= startDate && report.month <= endDate) {
            result.append(report);
        }
    }

    return result;
}

QJsonObject ClassDataManager::attendanceRecordToJson(const AttendanceRecord& record) const {
    QJsonObject json;
    json["classId"] = record.classId;
    json["memberId"] = record.memberId;
    json["date"] = record.date.toString(Qt::ISODate);
    json["attended"] = record.attended;
    json["amountPaid"] = record.amountPaid;
    return json;
}

AttendanceRecord ClassDataManager::jsonToAttendanceRecord(const QJsonObject& json) {
    AttendanceRecord record;
    record.classId = json["classId"].toInt();
    record.memberId = json["memberId"].toInt();
    record.date = QDate::fromString(json["date"].toString(), Qt::ISODate);
    record.attended = json["attended"].toBool();
    record.amountPaid = json["amountPaid"].toDouble();
    return record;
}

QJsonObject ClassDataManager::monthlyReportToJson(const MonthlyReport& report) const {
    QJsonObject json;
    json["month"] = report.month.toString(Qt::ISODate);
    json["totalActiveMembers"] = report.totalActiveMembers;
    json["totalClassesHeld"] = report.totalClassesHeld;
    json["totalAttendance"] = report.totalAttendance;
    json["totalRevenue"] = report.totalRevenue;

    QJsonArray attendanceArray;
    for (const auto& pair : report.classAttendance) {
        QJsonObject attendanceObj;
        attendanceObj["className"] = pair.first;
        attendanceObj["count"] = pair.second;
        attendanceArray.append(attendanceObj);
    }
    json["classAttendance"] = attendanceArray;

    QJsonArray revenueArray;
    for (const auto& pair : report.classRevenue) {
        QJsonObject revenueObj;
        revenueObj["className"] = pair.first;
        revenueObj["amount"] = pair.second;
        revenueArray.append(revenueObj);
    }
    json["classRevenue"] = revenueArray;

    return json;
}

MonthlyReport ClassDataManager::jsonToMonthlyReport(const QJsonObject& json) {
    MonthlyReport report;
    report.month = QDate::fromString(json["month"].toString(), Qt::ISODate);
    report.totalActiveMembers = json["totalActiveMembers"].toInt();
    report.totalClassesHeld = json["totalClassesHeld"].toInt();
    report.totalAttendance = json["totalAttendance"].toInt();
    report.totalRevenue = json["totalRevenue"].toDouble();

    QJsonArray attendanceArray = json["classAttendance"].toArray();
    for (const QJsonValue& value : attendanceArray) {
        QJsonObject obj = value.toObject();
        report.classAttendance.append(qMakePair(obj["className"].toString(), obj["count"].toInt()));
    }

    QJsonArray revenueArray = json["classRevenue"].toArray();
    for (const QJsonValue& value : revenueArray) {
        QJsonObject obj = value.toObject();
        report.classRevenue.append(qMakePair(obj["className"].toString(), obj["amount"].toDouble()));
    }

    return report;
}

bool ClassDataManager::loadAttendanceRecords() {
    QFile file(QDir(dataDir).filePath("attendance.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return false;
    }

    attendanceRecords.clear();
    QJsonArray recordsArray = doc.array();
    for (const QJsonValue& value : recordsArray) {
        attendanceRecords.push_back(jsonToAttendanceRecord(value.toObject()));
    }

    return true;
}

bool ClassDataManager::saveAttendanceRecords() const {
    QFile file(QDir(dataDir).filePath("attendance.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray recordsArray;
    for (const auto& record : attendanceRecords) {
        recordsArray.append(attendanceRecordToJson(record));
    }

    file.write(QJsonDocument(recordsArray).toJson());
    file.close();
    return true;
}

bool ClassDataManager::loadMonthlyReports() {
    QFile file(QDir(dataDir).filePath("monthly_reports.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return false;
    }

    monthlyReports.clear();
    QJsonArray reportsArray = doc.array();
    for (const QJsonValue& value : reportsArray) {
        monthlyReports.push_back(jsonToMonthlyReport(value.toObject()));
    }

    return true;
}

bool ClassDataManager::saveMonthlyReports() const {
    QFile file(QDir(dataDir).filePath("monthly_reports.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray reportsArray;
    for (const auto& report : monthlyReports) {
        reportsArray.append(monthlyReportToJson(report));
    }

    file.write(QJsonDocument(reportsArray).toJson());
    file.close();
    return true;
} 