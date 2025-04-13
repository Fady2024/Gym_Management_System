#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
    User();
    User(const QString& name, const QString& email, const QString& password, const QString& photoPath);

    [[nodiscard]] int getId() const;
    [[nodiscard]] QString getName() const;
    [[nodiscard]] QString getEmail() const;
    [[nodiscard]] QString getPassword() const;
    [[nodiscard]] QString getUserPhotoPath() const;

    void setId(int id);
    void setName(const QString& name);
    void setEmail(const QString& email);
    void setPassword(const QString& password);
    void setUserPhotoPath(const QString& photoPath);

private:
    int id;
    QString name;
    QString email;
    QString password;
    QString photoPath;
};

#endif 