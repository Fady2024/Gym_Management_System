#ifndef CLASSDATAMANAGER_H
#define CLASSDATAMANAGER_H


#include "../Model/Gym/class.h"
#include "../DataManager/memberdatamanager.h"
#include <QString>
#include <QVector>
#include <unordered_map>
#include <QDate>

// Structure to store attendance data
struct AttendanceRecord {
    int classId;
    int memberId;
    QDate date;
    bool attended;
    double amountPaid;
};

// Structure for monthly report
struct MonthlyReport {
    QDate month;
    int totalActiveMembers;
    int totalClassesHeld;
    int totalAttendance;
    double totalRevenue;
    QVector<QPair<QString, int>> classAttendance; // Class name and attendance count
    QVector<QPair<QString, double>> classRevenue; // Class name and revenue
};

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

    // Attendance tracking
    bool recordAttendance(int classId, int memberId, const QDate& date, bool attended, double amountPaid, QString& errorMessage);
    QVector<AttendanceRecord> getAttendanceRecords(int classId, const QDate& startDate, const QDate& endDate) const;
    int getAttendanceCount(int classId, const QDate& date) const;
    double getClassRevenue(int classId, const QDate& startDate, const QDate& endDate) const;

    // Monthly reports
    MonthlyReport generateMonthlyReport(const QDate& month) const;
    bool saveMonthlyReport(const MonthlyReport& report, QString& errorMessage) const;
    QVector<MonthlyReport> getMonthlyReports(const QDate& startDate, const QDate& endDate) const;

    // Dependency injection
    void setMemberDataManager(MemberDataManager* memberManager) { memberDataManager = memberManager; }

private:
    QString dataDir;
    std::unordered_map<int, Class> classesById;
    bool dataModified = false;
    std::vector<AttendanceRecord> attendanceRecords;
    std::vector<MonthlyReport> monthlyReports;
    MemberDataManager* memberDataManager = nullptr;

    QJsonArray readClassesFromFile(QString& errorMessage) const;
    bool writeClassesToFile(const QJsonArray& classes, QString& errorMessage) const;
    static QJsonObject classToJson(const Class& gymClass);
    static Class jsonToClass(const QJsonObject& json);
    [[nodiscard]] int generateClassId() const;
    QJsonObject attendanceRecordToJson(const AttendanceRecord& record) const;
    static AttendanceRecord jsonToAttendanceRecord(const QJsonObject& json);
    QJsonObject monthlyReportToJson(const MonthlyReport& report) const;
    static MonthlyReport jsonToMonthlyReport(const QJsonObject& json);
    bool loadAttendanceRecords();
    bool saveAttendanceRecords() const;
    bool loadMonthlyReports();
    bool saveMonthlyReports() const;
};

#endif 