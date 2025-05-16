#ifndef WORKOUTDATAMANAGER_H
#define WORKOUTDATAMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>

struct Exercise {
    QString name;
    int sets;
    QString reps;
    int caloriesPerSet;
};

struct Workout {
    int id;
    QString name;
    QVector<Exercise> exercises;
    int duration;
    int totalCalories;
    QString difficulty;
};

struct WorkoutLog {
    int userId;
    int classId;
    int workoutId;
    QDateTime timestamp;
    int totalCaloriesBurnt;
    QVector<QPair<QString, bool>> completedExercises;  // exercise name and completion status
};

class WorkoutDataManager : public QObject {
    Q_OBJECT

public:
    explicit WorkoutDataManager(QObject* parent = nullptr);
    ~WorkoutDataManager() override;

    bool initializeFromFile();
    bool saveToFile();

    // Workout management
    QVector<Workout> getAllWorkouts() const;
    Workout getWorkoutById(int workoutId) const;
    QVector<Workout> getWorkoutsByDifficulty(const QString& difficulty) const;

    // Logging management
    bool logWorkout(const WorkoutLog& log, QString& errorMessage);
    QVector<WorkoutLog> getUserWorkoutLogs(int userId) const;
    QVector<WorkoutLog> getUserWorkoutLogsByClass(int userId, int classId) const;
    QVector<WorkoutLog> getUserWorkoutLogsByDateRange(int userId, const QDate& startDate, const QDate& endDate) const;
    
    // Statistics
    int getTotalCaloriesBurnt(int userId) const;
    int getTotalWorkoutsCompleted(int userId) const;
    QMap<QString, int> getMostFrequentExercises(int userId) const;

private:
    QString dataDir;
    QVector<Workout> workouts;
    QVector<WorkoutLog> workoutLogs;
    bool dataModified = false;

    bool loadWorkouts();
    bool saveWorkouts() const;
    bool loadWorkoutLogs();
    bool saveWorkoutLogs() const;

    static QJsonObject workoutToJson(const Workout& workout);
    static Workout jsonToWorkout(const QJsonObject& json);
    static QJsonObject workoutLogToJson(const WorkoutLog& log);
    static WorkoutLog jsonToWorkoutLog(const QJsonObject& json);
};

#endif 