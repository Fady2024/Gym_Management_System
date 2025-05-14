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

    Staff() = default;

    Staff(int id, const QString& name, const QString& email, const QString& password,
          const QString& photoPath, const QDate& dateOfBirth, const Role role)
        : id(id), name(name), email(email), password(password),
          photoPath(photoPath), dateOfBirth(dateOfBirth), role(role) {}

    [[nodiscard]] int getId() const { return id; }
    [[nodiscard]] QString getName() const { return name; }
    [[nodiscard]] QString getEmail() const { return email; }
    [[nodiscard]] QString getPassword() const { return password; }
    [[nodiscard]] QString getUserPhotoPath() const { return photoPath; }
    [[nodiscard]] QDate getDateOfBirth() const { return dateOfBirth; }
    [[nodiscard]] Role getRole() const { return role; }

    void setId(int newId) { id = newId; }
    void setName(const QString& newName) { name = newName; }
    void setEmail(const QString& newEmail) { email = newEmail; }
    void setPassword(const QString& newPassword) { password = newPassword; }
    void setUserPhotoPath(const QString& newPhotoPath) { photoPath = newPhotoPath; }
    void setDateOfBirth(const QDate& newDateOfBirth) { dateOfBirth = newDateOfBirth; }
    void setRole(const Role newRole) { role = newRole; }

    bool operator==(const Staff& other) const {
        return id == other.id &&
            name == other.name &&
            email == other.email &&
            password == other.password &&
            photoPath == other.photoPath &&
            dateOfBirth == other.dateOfBirth &&
            role == other.role;
    }

private:
    int id = 0;
    QString name;
    QString email;
    QString password;
    QString photoPath;
    QDate dateOfBirth;
    Role role = Role::COACH;
};

#endif