#ifndef COURT_H
#define COURT_H

#include <QString>

class Court {
public:
    Court(); // default constructor
    Court(int id, const QString& name, const QString& location);

    int getId() const;
    QString getName() const;
    QString getLocation() const;

    void setId(int id);
    void setName(const QString& name);
    void setLocation(const QString& location);

private:
    int m_id;
    QString m_name;
    QString m_location;
};

#endif // COURT_H
