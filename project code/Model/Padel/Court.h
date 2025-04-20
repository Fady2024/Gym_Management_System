#ifndef COURT_H
#define COURT_H

#include <QString>

class Court {
public:
    Court();
    Court(int id, const QString& location, const QString& name);

    int getId() const;
    QString getLocation() const;
    QString getName() const;

    void setId(int id);
    void setLocation(const QString& location);
    void setName(const QString& name);

private:
    int m_id;
    QString m_location;
    QString m_name;
};

#endif // COURT_H
