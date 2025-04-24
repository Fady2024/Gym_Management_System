#ifndef CLASSDATAMANAGER_H
#define CLASSDATAMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonParseError>
#include "../Model/class.h"
#include <QString>
#include <QVector>
#include <unordered_map>
#include <QDir>

class ClassDataManager : public QObject {
    Q_OBJECT

public:
    explicit ClassDataManager(QObject* parent = nullptr);
    ~ClassDataManager() override;

    bool initializeFromFile();
    bool saveToFile();
    void handleApplicationClosing();

    // Class management methods
    bool addClass(const Class& gymClass, QString& errorMessage);
    bool updateClass(const Class& gymClass, QString& errorMessage);
    bool deleteClass(int classId, QString& errorMessage);
    Class getClassById(int classId) const;
    QVector<Class> getAllClasses() const;
    QVector<Class> getClassesByCoach(const QString& coachName) const;
    QVector<Class> getClassesByDate(const QDate& date) const;

    // Waitlist management
    bool addToWaitlist(int classId, int memberId, bool isVIP, QString& errorMessage);
    bool removeFromWaitlist(int classId, int memberId, QString& errorMessage);
    int getNextWaitlistMember(int classId) const;
    QVector<int> getWaitlist(int classId) const;

    // Session management
    bool addSession(int classId, const QDate& date, QString& errorMessage);
    bool removeSession(int classId, const QDate& date, QString& errorMessage);
    QVector<QDate> getClassSessions(int classId) const;

    // Enrollment management
    bool enrollMember(int classId, int memberId, QString& errorMessage);
    bool unenrollMember(int classId, int memberId, QString& errorMessage);
    bool isClassFull(int classId) const;
    int getEnrolledCount(int classId) const;

private:
    QString dataDir;
    std::unordered_map<int, Class> classesById;
    bool dataModified = false;

    QJsonArray readClassesFromFile(QString& errorMessage) const;
    bool writeClassesToFile(const QJsonArray& classes, QString& errorMessage) const;
    static QJsonObject classToJson(const Class& gymClass);
    static Class jsonToClass(const QJsonObject& json);
    [[nodiscard]] int generateClassId() const;
};

#endif 