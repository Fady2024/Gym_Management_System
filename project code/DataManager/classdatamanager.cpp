#include "classdatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>

ClassDataManager::ClassDataManager(QObject* parent)
    : QObject(parent) {
    // Get the project directory path
    QString projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
    
    // Set data directory path
    dataDir = projectDir + "/project code/Data";
    
    // Create directory if it doesn't exist
    QDir().mkpath(dataDir);
    
    // Initialize empty classes.json if it doesn't exist
    QFile classesFile(dataDir + "/classes.json");
    if (!classesFile.exists()) {
        classesFile.open(QIODevice::WriteOnly);
        classesFile.write("[]");
        classesFile.close();
    }
    
    // Initialize data from file
    if (!initializeFromFile()) {
        qDebug() << "Failed to initialize class data from file";
    }
}

ClassDataManager::~ClassDataManager() {
    handleApplicationClosing();
}

void ClassDataManager::handleApplicationClosing() {
    if (dataModified) {
        qDebug() << "Saving class data before application closing...";
        if (!saveToFile()) {
            qDebug() << "Failed to save class data before application closing!";
        } else {
            qDebug() << "Class data saved successfully before application closing.";
        }
    } else {
        qDebug() << "No changes to class data, skipping save on application exit";
    }
}

bool ClassDataManager::initializeFromFile() {
    QString errorMessage;
    QJsonArray classesArray = readClassesFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading classes file:" << errorMessage;
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
        qDebug() << "Error saving classes file:" << errorMessage;
        return false;
    }

    dataModified = false;
    return true;
}

bool ClassDataManager::addClass(const Class& gymClass, QString& errorMessage) {
    if (gymClass.getId() != 0) {
        errorMessage = "Class already has an ID";
        return false;
    }

    Class newClass = gymClass;
    int newId = generateClassId();
    newClass.setId(newId);
    classesById[newId] = newClass;
    dataModified = true;
    qDebug() << "Class added and marked for saving at application exit";
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
    qDebug() << "Class updated and marked for saving at application exit";
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
    qDebug() << "Class deleted and marked for saving at application exit";
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

    Class& gymClass = it->second;
    if (gymClass.isFull()) {
        errorMessage = "Class is full";
        return false;
    }

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
    if (gymClass.getNumOfEnrolled() > 0) {
        gymClass.setNumOfEnrolled(gymClass.getNumOfEnrolled() - 1);
        dataModified = true;
        return true;
    }

    errorMessage = "No members enrolled in this class";
    return false;
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
    json["from"] = gymClass.getFrom().toString();
    json["to"] = gymClass.getTo().toString();
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
    gymClass.setFrom(QTime::fromString(json["from"].toString()));
    gymClass.setTo(QTime::fromString(json["to"].toString()));
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