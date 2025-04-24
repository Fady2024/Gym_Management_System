#ifndef STAFF_H
#define STAFF_H

#include <QString>
#include <QDate>

class Staff
{
public:
    enum class Role {
        COACH,
        MANGER,
        RECEPTIONIST
    };
    Staff(int id,const QString& name, const QString& email, const QString& password, const QString& photoPath, const QDate& dateOfBirth, const Role role);

    [[nodiscard]] int getId() const;
    [[nodiscard]] QString getName() const;
    [[nodiscard]] QString getEmail() const;
    [[nodiscard]] QString getPassword() const;
    [[nodiscard]] QString getUserPhotoPath() const;
    [[nodiscard]] QDate getDateOfBirth() const;
    [[nodiscard]] Role getRole() const;

    void setId(int id);
    void setName(const QString& name);
    void setEmail(const QString& email);
    void setPassword(const QString& password);
    void setUserPhotoPath(const QString& photoPath);
    void setDateOfBirth(const QDate& dateOfBirth);
    void setRole(const Role role);

    bool operator==(const User& other) const {
        return id == other.id &&
            name == other.name &&
            email == other.email &&
            password == other.password &&
            photoPath == other.photoPath &&
            dateOfBirth == other.dateOfBirth &&
            role == other.role;
    }

private:
    int id;
    QString name;
    QString email;
    QString password;
    QString photoPath;
    QDate dateOfBirth;
    Role role;
};