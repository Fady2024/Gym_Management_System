#include "user.h"

User::User() : id(0) {}

User::User(const QString& name, const QString& email, const QString& password, const QString& photoPath, const QDate& dateOfBirth)
    : id(0), name(name), email(email), password(password), photoPath(photoPath), dateOfBirth(dateOfBirth) {
}

int User::getId() const { return id; }
QString User::getName() const { return name; }
QString User::getEmail() const { return email; }
QString User::getPassword() const { return password; }
QString User::getUserPhotoPath() const { return photoPath; }
QDate User::getDateOfBirth() const { return dateOfBirth; }

void User::setId(int id) { this->id = id; }
void User::setName(const QString& name) { this->name = name; }
void User::setEmail(const QString& email) { this->email = email; }
void User::setPassword(const QString& password) { this->password = password; }
void User::setUserPhotoPath(const QString& photoPath) { this->photoPath = photoPath; }
void User::setDateOfBirth(const QDate& dateOfBirth) { this->dateOfBirth = dateOfBirth; }