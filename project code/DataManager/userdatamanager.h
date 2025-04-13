#ifndef USERDATAMANAGER_H
#define USERDATAMANAGER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonParseError>
#include "../Model/user.h"
#include <QList>
#include <QString>
#include <QVector>

class UserDataManager : public QObject
{
    Q_OBJECT

public:
    explicit UserDataManager(QObject* parent = nullptr);
    ~UserDataManager() override;

    bool initializeFromFile();  // Load data at startup
    bool saveToFile();         // Save data at exit

    // In-memory operations
    bool saveUserData(const User& user, QString& errorMessage);
    bool validateUser(const QString& email, const QString& password);
    bool emailExists(const QString& email) const;
    User getUserData(const QString& email);
    QList<User>& getUsers();
    bool deleteAccount(const QString& email, QString& errorMessage);

    // Validation methods
    static bool validateEmail(const QString& email, QString& errorMessage);
    static bool validatePassword(const QString& password, QString& errorMessage);
    static bool validateName(const QString& name, QString& errorMessage);
    bool validateNewUser(const User& user, QString& errorMessage);

    [[nodiscard]] QVector<User> getAllUsers() const;
    
    // Remember me functionality
    bool saveRememberedCredentials(const QString& email, const QString& password);
    bool clearRememberedCredentials();
    bool getRememberedCredentials(QString& email, QString& password) const;

private:
    QString usersFilePath;
    QString rememberedCredentialsPath;
    QList<User> users;
    bool hasRememberedCredentials = false;
    QString rememberedEmail;
    QString rememberedPassword;

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