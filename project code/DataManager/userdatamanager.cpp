#include "userdatamanager.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QRandomGenerator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>

UserDataManager::UserDataManager(QObject* parent)
    : QObject(parent)
{
    // Get the project directory path
    QString projectDir;
    
#ifdef FORCE_SOURCE_DIR
    // Use the source directory path defined in CMake
    projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
    qDebug() << "Using source directory path:" << projectDir;
#else
    // Fallback to application directory
    projectDir = QCoreApplication::applicationDirPath();
    projectDir = QFileInfo(projectDir).dir().absolutePath();
    qDebug() << "Using application directory path:" << projectDir;
#endif
    
    // Set data directory paths
    dataDir = projectDir + "/project code/Data";
    usersPhotoDir = projectDir + "/project code/UsersPhoto";
    
    qDebug() << "Data directory path:" << dataDir;
    qDebug() << "Users photo directory path:" << usersPhotoDir;
    
    // Create directories if they don't exist
    QDir().mkpath(dataDir);
    QDir().mkpath(usersPhotoDir);
    
    // Initialize empty users.json if it doesn't exist
    QFile usersFile(dataDir + "/users.json");
    if (!usersFile.exists()) {
        usersFile.open(QIODevice::WriteOnly);
        usersFile.write("[]");
        usersFile.close();
    }
    
    // Initialize empty remembered.json if it doesn't exist
    QFile rememberedFile(dataDir + "/remembered.json");
    if (!rememberedFile.exists()) {
        rememberedFile.open(QIODevice::WriteOnly);
        rememberedFile.write("{}");
        rememberedFile.close();
    }
    
    // Initialize data from file
    if (!initializeFromFile()) {
        qDebug() << "Failed to initialize user data from file";
    }
}

UserDataManager::~UserDataManager()
{
    handleApplicationClosing();
}

void UserDataManager::handleApplicationClosing()
{
    if (dataModified) {
        qDebug() << "Saving data before application closing...";
        if (!saveToFile()) {
            qDebug() << "Failed to save data before application closing!";
        } else {
            qDebug() << "Data saved successfully before application closing.";
        }
    } else {
        qDebug() << "No changes to user data, skipping save on application exit";
    }
}

bool UserDataManager::initializeFromFile()
{
    QString errorMessage;
    QJsonArray usersArray = readUsersFromFile(errorMessage);
    if (!errorMessage.isEmpty()) {
        qDebug() << "Error reading users file:" << errorMessage;
        return false;
    }

    usersById.clear();
    emailToIdMap.clear();

    for (const QJsonValue& userValue : usersArray) {
        User user = jsonToUser(userValue.toObject());
        usersById[user.getId()] = user;
        emailToIdMap[user.getEmail()] = user.getId();
    }

    return loadRememberedCredentials();
}

bool UserDataManager::saveToFile()
{
    QJsonArray usersArray;
    for (const auto& pair : usersById) {
        usersArray.append(userToJson(pair.second));
    }

    QString errorMessage;
    bool success = writeUsersToFile(usersArray, errorMessage);
    if (!success) {
        qDebug() << "Error saving users file:" << errorMessage;
        return false;
    }

    success = saveRememberedCredentialsToFile();
    if (success) {
        dataModified = false;
    }
    return success;
}

bool UserDataManager::saveUserData(const User& user, QString& errorMessage)
{
    if (!validateNewUser(user, errorMessage)) {
        return false;
    }

    auto it = emailToIdMap.find(user.getEmail());
    if (it != emailToIdMap.end()) {
        int userId = it->second;
        usersById[userId] = user;
    } else {
        User newUser = user;
        int newId = generateUserId();
        newUser.setId(newId);
        usersById[newId] = newUser;
        emailToIdMap[newUser.getEmail()] = newId;
    }

    dataModified = true;
    qDebug() << "User data updated and marked for saving at application exit";
    return true;
}

bool UserDataManager::validateUser(const QString& email, const QString& password)
{
    auto it = emailToIdMap.find(email);
    if (it == emailToIdMap.end()) {
        return false;
    }

    const User& user = usersById[it->second];
    return user.getPassword() == password;
}

User UserDataManager::getUserData(const QString& email)
{
    auto it = emailToIdMap.find(email);
    if (it != emailToIdMap.end()) {
        return usersById[it->second];
    }
    return User();
}

User UserDataManager::getUserDataById(int id)
{
    auto it = usersById.find(id);
    if (it != usersById.end()) {
        return it->second;
    }
    return User();
}

QVector<User> UserDataManager::getAllUsers() const
{
    QVector<User> result;
    result.reserve(usersById.size());
    for (const auto& pair : usersById) {
        result.append(pair.second);
    }
    return result;
}

bool UserDataManager::deleteAccount(const QString& email, QString& errorMessage)
{
    auto it = emailToIdMap.find(email);
    if (it == emailToIdMap.end()) {
        errorMessage = "User not found";
        return false;
    }

    int userId = it->second;
    return deleteAccountById(userId, errorMessage);
}

bool UserDataManager::deleteAccountById(int id, QString& errorMessage)
{
    auto it = usersById.find(id);
    if (it == usersById.end()) {
        errorMessage = "User not found";
        return false;
    }

    // Remove from all data structures
    const User& user = it->second;
    emailToIdMap.erase(user.getEmail());
    usersById.erase(id);

    dataModified = true;
    qDebug() << "User deleted and data marked for saving at application exit";
    return true;
}

bool UserDataManager::emailExists(const QString& email) const
{
    return emailToIdMap.find(email) != emailToIdMap.end();
}

int UserDataManager::generateUserId() const {
    int maxId = 0;
    for (const auto& pair : usersById) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }
    return maxId + 1;  // Return the next available ID
}

bool UserDataManager::saveRememberedCredentials(const QString& email, const QString& password)
{
    rememberedEmail = email;
    rememberedPassword = password;
    hasRememberedCredentials = true;
    dataModified = true;  // Mark data as modified
    qDebug() << "Remembered credentials updated and marked for saving at application exit";
    return true;
}

bool UserDataManager::clearRememberedCredentials()
{
    rememberedEmail.clear();
    rememberedPassword.clear();
    hasRememberedCredentials = false;
    dataModified = true;  // Mark data as modified
    qDebug() << "Remembered credentials cleared and marked for saving at application exit";
    return true;
}

bool UserDataManager::getRememberedCredentials(QString& email, QString& password) const
{
    if (!hasRememberedCredentials) {
        return false;
    }
    email = rememberedEmail;
    password = rememberedPassword;
    return true;
}

QJsonArray UserDataManager::readUsersFromFile(QString& errorMessage) const
{
    QFile file(QDir(dataDir).filePath("users.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Could not open users file for reading";
        return QJsonArray();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Error parsing users file: " + parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}

bool UserDataManager::writeUsersToFile(const QJsonArray& users, QString& errorMessage) const
{
    QFile file(QDir(dataDir).filePath("users.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Could not open users file for writing";
        return false;
    }

    QJsonDocument doc(users);
    file.write(doc.toJson());
    file.close();
    return true;
}

QJsonObject UserDataManager::userToJson(const User& user)
{
    QJsonObject json;
    json["id"] = user.getId();
    json["email"] = user.getEmail();
    json["password"] = user.getPassword();
    json["name"] = user.getName();
    
    // Calculate age from date of birth
    QDate currentDate = QDate::currentDate();
    QDate birthDate = user.getDateOfBirth();
    int age = currentDate.year() - birthDate.year();
    if (currentDate.month() < birthDate.month() || 
        (currentDate.month() == birthDate.month() && currentDate.day() < birthDate.day())) {
        age--;
    }
    json["age"] = age;
    
    json["photoPath"] = user.getUserPhotoPath();
    return json;
}

User UserDataManager::jsonToUser(const QJsonObject& json)
{
    User user;
    user.setId(json["id"].toInt());
    user.setEmail(json["email"].toString());
    user.setPassword(json["password"].toString());
    user.setName(json["name"].toString());
    
    // Convert age back to date of birth (approximate)
    int age = json["age"].toInt();
    QDate currentDate = QDate::currentDate();
    QDate birthDate = currentDate.addYears(-age);
    user.setDateOfBirth(birthDate);
    
    user.setUserPhotoPath(json["photoPath"].toString());
    return user;
}

bool UserDataManager::loadRememberedCredentials()
{
    QFile file(QDir(dataDir).filePath("remembered.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject json = doc.object();
    rememberedEmail = json["email"].toString();
    rememberedPassword = json["password"].toString();
    hasRememberedCredentials = !rememberedEmail.isEmpty() && !rememberedPassword.isEmpty();
    return hasRememberedCredentials;
}

bool UserDataManager::saveRememberedCredentialsToFile() const
{
    if (!hasRememberedCredentials) {
        QFile::remove(QDir(dataDir).filePath("remembered.json"));
        return true;
    }

    QFile file(QDir(dataDir).filePath("remembered.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject json;
    json["email"] = rememberedEmail;
    json["password"] = rememberedPassword;

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool UserDataManager::validateEmail(const QString& email, QString& errorMessage)
{
    if (email.isEmpty()) {
        errorMessage = "Email cannot be empty";
        return false;
    }

    if (!email.contains('@') || !email.contains('.')) {
        errorMessage = "Invalid email format";
        return false;
    }

    return true;
}

bool UserDataManager::validatePassword(const QString& password, QString& errorMessage)
{
    if (password.length() < 8) {
        errorMessage = "Password must be at least 8 characters long";
        return false;
    }

    return true;
}

bool UserDataManager::validateName(const QString& name, QString& errorMessage)
{
    if (name.isEmpty()) {
        errorMessage = "Name cannot be empty";
        return false;
    }

    return true;
}

bool UserDataManager::validateDateOfBirth(const QDate& dateOfBirth, QString& errorMessage)
{
    if (!dateOfBirth.isValid()) {
        errorMessage = "Invalid date of birth";
        return false;
    }

    QDate currentDate = QDate::currentDate();
    if (dateOfBirth > currentDate) {
        errorMessage = "Date of birth cannot be in the future";
        return false;
    }

    return true;
}

bool UserDataManager::validateNewUser(const User& user, QString& errorMessage)
{
    if (!validateEmail(user.getEmail(), errorMessage)) {
        return false;
    }

    if (!validatePassword(user.getPassword(), errorMessage)) {
        return false;
    }

    if (!validateName(user.getName(), errorMessage)) {
        return false;
    }

    if (!validateDateOfBirth(user.getDateOfBirth(), errorMessage)) {
        return false;
    }

    return true;
}