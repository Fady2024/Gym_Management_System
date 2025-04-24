#include "staff.h"

Staff::Staff(int id,const QString& name, const QString& email, const QString& password, const QString& photoPath, const QDate& dateOfBirth,const Role role)
    : id(id), name(name), email(email), password(password), photoPath(photoPath), dateOfBirth(dateOfBirth), role(role) {
}

int Staff::getId() const { return id; }
QString Staff::getName() const { return name; }
QString Staff::getEmail() const { return email; }
QString Staff::getPassword() const { return password; }
QString Staff::getUserPhotoPath() const { return photoPath; }
QDate Staff::getDateOfBirth() const { return dateOfBirth; }
Role Staff::getRole() const { return role; }

void Staff::setId(int id) { this->id = id; }
void Staff::setName(const QString& name) { this->name = name; }
void Staff::setEmail(const QString& email) { this->email = email; }
void Staff::setPassword(const QString& password) { this->password = password; }
void Staff::setUserPhotoPath(const QString& photoPath) { this->photoPath = photoPath; }
void Staff::setDateOfBirth(const QDate& dateOfBirth) { this->dateOfBirth = dateOfBirth; }
void Staff::setRole(const Role role) { this->role = role; }