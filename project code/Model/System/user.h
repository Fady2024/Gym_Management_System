#ifndef USER_H
#define USER_H

#include <QString>
#include <QDate>

class User
{
public:
    User();
    User(const QString& name, const QString& email, const QString& password, const QString& photoPath, const QDate& dateOfBirth);

    [[nodiscard]] int getId() const;
    [[nodiscard]] QString getName() const;
    [[nodiscard]] QString getEmail() const;
    [[nodiscard]] QString getPassword() const;
    [[nodiscard]] QString getUserPhotoPath() const;
    [[nodiscard]] QDate getDateOfBirth() const;

    void setId(int id);
    void setName(const QString& name);
    void setEmail(const QString& email);
    void setPassword(const QString& password);
    void setUserPhotoPath(const QString& photoPath);
    void setDateOfBirth(const QDate& dateOfBirth);

    bool operator==(const User& other) const {
        return id == other.id &&
               name == other.name &&
               email == other.email &&
               password == other.password &&
               photoPath == other.photoPath &&
               dateOfBirth == other.dateOfBirth;
    }

private:
    int id;
    QString name;
    QString email;
    QString password;
    QString photoPath;
    QDate dateOfBirth;
};

#endif 