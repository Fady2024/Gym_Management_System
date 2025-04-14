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

UserDataManager::UserDataManager(QObject* parent)
    : QObject(parent)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    
    usersFilePath = appDataPath + "/users.json";
    rememberedCredentialsPath = appDataPath + "/remembered.json";
    
    if (!initializeFromFile()) {
        qWarning() << "Failed to initialize user data from file";
    }
    loadRememberedCredentials();
}

UserDataManager::~UserDataManager() {
    saveToFile();
    saveRememberedCredentialsToFile();
}

bool UserDataManager::initializeFromFile() {
    QString errorMessage;
    QJsonArray usersArray;
    
    QFile localFile(usersFilePath);
    if (localFile.exists()) {
        if (!localFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open local users file:" << localFile.errorString();
            return false;
        }
        QJsonDocument doc = QJsonDocument::fromJson(localFile.readAll());
        localFile.close();
        if (doc.isArray()) {
            usersArray = doc.array();
        }
    } else {
        QFile resourceFile(":/project code/Data/users.json");
        if (resourceFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(resourceFile.readAll());
            resourceFile.close();
            if (doc.isArray()) {
                usersArray = doc.array();
            }
        }
    }
    
    users.clear();
    for (const QJsonValue& value : usersArray) {
        if (value.isObject()) {
            User user = jsonToUser(value.toObject());
            users.append(user);
        }
    }
    return true;
}

bool UserDataManager::saveToFile() {
    QJsonArray usersArray;
    for (const User& user : users) {
        usersArray.append(userToJson(user));
    }
    
    QFile file(usersFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open users file for writing:" << file.errorString();
        return false;
    }
    
    QJsonDocument doc(usersArray);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool UserDataManager::loadRememberedCredentials() {
    QFile localFile(rememberedCredentialsPath);
    if (localFile.exists()) {
        if (!localFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open local remembered credentials file:" << localFile.errorString();
            return false;
        }
        QJsonDocument doc = QJsonDocument::fromJson(localFile.readAll());
        localFile.close();
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            hasRememberedCredentials = obj["hasCredentials"].toBool();
            if (hasRememberedCredentials) {
                rememberedEmail = obj["email"].toString();
                rememberedPassword = obj["password"].toString();
            }
            return true;
        }
    } else {
        QFile resourceFile(":/project code/Data/remembered.json");
        if (resourceFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(resourceFile.readAll());
            resourceFile.close();
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                hasRememberedCredentials = obj["hasCredentials"].toBool();
                if (hasRememberedCredentials) {
                    rememberedEmail = obj["email"].toString();
                    rememberedPassword = obj["password"].toString();
                }
                return true;
            }
        }
    }
    return false;
}

bool UserDataManager::saveRememberedCredentialsToFile() const {
    QFile file(rememberedCredentialsPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open remembered credentials file for writing:" << file.errorString();
        return false;
    }
    
    QJsonObject obj;
    obj["hasCredentials"] = hasRememberedCredentials;
    if (hasRememberedCredentials) {
        obj["email"] = rememberedEmail;
        obj["password"] = rememberedPassword;
    }
    
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool UserDataManager::saveUserData(const User& user, QString& errorMessage) {
    if (!validateEmail(user.getEmail(), errorMessage)) return false;
    if (!validatePassword(user.getPassword(), errorMessage)) return false;
    if (!validateName(user.getName(), errorMessage)) return false;

    bool userExists = false;
    for (auto & i : users) {
        if (i.getEmail() == user.getEmail()) {
            i = user;
            userExists = true;
            break;
        }
    }

    if (!userExists) {
        if (!validateNewUser(user, errorMessage)) return false;
        users.append(user);
    }

    return true;
}

bool UserDataManager::validateUser(const QString& email, const QString& password) {
    if (!emailExists(email)) {
        return false;
    }

    for (const User& user : users) {
        if (user.getEmail().toLower() == email.toLower() && user.getPassword() == password) {
            return true;
        }
    }
    return false;
}

User UserDataManager::getUserData(const QString& email) {
    for (const User& user : users) {
        if (user.getEmail() == email) {
            return user;
        }
    }
    return User();
}

QList<User>& UserDataManager::getUsers() {
    return users;
}

bool UserDataManager::saveRememberedCredentials(const QString& email, const QString& password) {
    hasRememberedCredentials = true;
    rememberedEmail = email;
    rememberedPassword = password;
    return true;
}

bool UserDataManager::clearRememberedCredentials() {
    hasRememberedCredentials = false;
    rememberedEmail.clear();
    rememberedPassword.clear();
    return true;
}

bool UserDataManager::getRememberedCredentials(QString& email, QString& password) const
{
    if (hasRememberedCredentials) {
        email = rememberedEmail;
        password = rememberedPassword;
        return true;
    }
    return false;
}

QVector<User> UserDataManager::getAllUsers() const {
    return users;
}

QJsonArray UserDataManager::readUsersFromFile(QString& errorMessage) const
{
    // Create directories if they don't exist
    QDir dataDir("project code/Data");
    if (!dataDir.exists()) {
        if (!dataDir.mkpath(".")) {
            errorMessage = "Failed to create directory: " + dataDir.path();
            return {};
        }
    }

    QFile file(usersFilePath);
    if (!file.exists()) {
        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = "Failed to create file: " + file.errorString();
            return {};
        }
        file.write("[]");
        file.close();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Failed to open file: " + file.errorString();
        return {};
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = "Failed to parse JSON data: " + parseError.errorString();
        return {};
    }

    if (!doc.isArray()) {
        errorMessage = "JSON data is not an array";
        return {};
    }

    return doc.array();
}

bool UserDataManager::writeUsersToFile(const QJsonArray& users, QString& errorMessage) const
{
    // Create directories if they don't exist
    QDir dataDir("project code/Data");
    if (!dataDir.exists()) {
        if (!dataDir.mkpath(".")) {
            errorMessage = "Failed to create directory: " + dataDir.path();
            return false;
        }
    }

    QFile file(usersFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Failed to open file: " + file.errorString();
        return false;
    }

    const QJsonDocument doc(users);
    file.write(doc.toJson());
    file.close();

    return true;
}

QJsonObject UserDataManager::userToJson(const User& user) {
    QJsonObject json;
    json["name"] = user.getName();
    json["email"] = user.getEmail();
    json["password"] = user.getPassword();
    json["photoPath"] = user.getUserPhotoPath();
    return json;
}

User UserDataManager::jsonToUser(const QJsonObject& json) {
    User user;
    user.setName(json["name"].toString());
    user.setEmail(json["email"].toString());
    user.setPassword(json["password"].toString());
    user.setUserPhotoPath(json["photoPath"].toString());
    return user;
}

bool UserDataManager::validateEmail(const QString& email, QString& errorMessage) {
    if (email.isEmpty()) {
        errorMessage = "Email cannot be empty";
        return false;
    }

    const QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        errorMessage = "Invalid email format";
        return false;
    }

    return true;
}

bool UserDataManager::validatePassword(const QString& password, QString& errorMessage) {
    if (password.isEmpty()) {
        errorMessage = "Password cannot be empty";
        return false;
    }

    if (password.length() < 8) {
        errorMessage = "Password must be at least 8 characters long";
        return false;
    }

    bool hasUpperCase = false;
    bool hasLowerCase = false;
    bool hasDigit = false;
    bool hasSpecialChar = false;

    for (const QChar& c : password) {
        if (c.isUpper()) hasUpperCase = true;
        else if (c.isLower()) hasLowerCase = true;
        else if (c.isDigit()) hasDigit = true;
        else if (!c.isLetterOrNumber()) hasSpecialChar = true;
    }

    if (!hasUpperCase || !hasLowerCase || !hasDigit || !hasSpecialChar) {
        errorMessage = "Password must contain uppercase, lowercase, numbers, and special characters";
        return false;
    }

    return true;
}

bool UserDataManager::validateName(const QString& name, QString& errorMessage) {
    if (name.isEmpty()) {
        errorMessage = "Name cannot be empty";
        return false;
    }

    if (name.length() < 2) {
        errorMessage = "Name must be at least 2 characters long";
        return false;
    }

    const QRegularExpression nameRegex("^[a-zA-Z\\s]+$");
    if (!nameRegex.match(name).hasMatch()) {
        errorMessage = "Name can only contain letters and spaces";
        return false;
    }

    return true;
}

bool UserDataManager::validateNewUser(const User& user, QString& errorMessage) {
    if (emailExists(user.getEmail())) {
        errorMessage = "Email already registered";
        return false;
    }

    if (!validateEmail(user.getEmail(), errorMessage)) return false;
    if (!validatePassword(user.getPassword(), errorMessage)) return false;
    if (!validateName(user.getName(), errorMessage)) return false;

    return true;
}

bool UserDataManager::deleteAccount(const QString& email, QString& errorMessage) {
    for (int i = 0; i < users.size(); ++i) {
        if (users[i].getEmail() == email) {
            users.removeAt(i);
            
            if (rememberedEmail == email) {
                clearRememberedCredentials();
            }
            
            return true;
        }
    }
    
    errorMessage = "Account not found";
    return false;
}

bool UserDataManager::emailExists(const QString& email) const {
    for (const User& user : users) {
        if (user.getEmail().toLower() == email.toLower()) {
            return true;
        }
    }
    return false;
}