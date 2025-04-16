#ifndef USERDATAMANAGER_H
#define USERDATAMANAGER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonParseError>
#include "../Model/user.h"
#include <QString>
#include <QVector>
#include <unordered_map>
#include <QObject>
#include <QDir>

class UserDataManager : public QObject
{
    Q_OBJECT

public:
    explicit UserDataManager(QObject* parent = nullptr);
    ~UserDataManager() override;

    bool initializeFromFile();  // Load data at startup
    bool saveToFile();         // Save data at exit
    void handleApplicationClosing();  // Handle application closing

    // In-memory operations
    bool saveUserData(const User& user, QString& errorMessage);
    bool validateUser(const QString& email, const QString& password);
    bool emailExists(const QString& email) const;
    User getUserData(const QString& email);
    User getUserDataById(int id);
    bool deleteAccount(const QString& email, QString& errorMessage);
    bool deleteAccountById(int id, QString& errorMessage);

    // Validation methods
    static bool validateEmail(const QString& email, QString& errorMessage);
    static bool validatePassword(const QString& password, QString& errorMessage);
    static bool validateName(const QString& name, QString& errorMessage);
    static bool validateDateOfBirth(const QDate& dateOfBirth, QString& errorMessage);
    bool validateNewUser(const User& user, QString& errorMessage);

    [[nodiscard]] QVector<User> getAllUsers() const;
    
    // Remember me functionality
    bool saveRememberedCredentials(const QString& email, const QString& password);
    bool clearRememberedCredentials();
    bool getRememberedCredentials(QString& email, QString& password) const;

    QString getUsersPhotoDir() const { return usersPhotoDir; }

private:
    QString dataDir;
    QString usersPhotoDir;
    std::unordered_map<int, User> usersById;
    std::unordered_map<QString, int> emailToIdMap;
    bool hasRememberedCredentials = false;
    QString rememberedEmail;
    QString rememberedPassword;
    bool dataModified = false;  // Track if data has been modified

    QJsonArray readUsersFromFile(QString& errorMessage) const;
    bool writeUsersToFile(const QJsonArray& users, QString& errorMessage) const;
    static QJsonObject userToJson(const User& user);
    static User jsonToUser(const QJsonObject& json);
    bool loadRememberedCredentials();

    [[nodiscard]] bool saveRememberedCredentialsToFile() const;

    [[nodiscard]] bool isEmailValid(const QString& email) const;
    [[nodiscard]] bool isPasswordValid(const QString& password) const;
    [[nodiscard]] bool isEmailUnique(const QString& email) const;
    [[nodiscard]] int generateUserId() const;
};

#endif