#include "workoutdatamanager.h"

#include <iostream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

WorkoutDataManager::WorkoutDataManager(QObject* parent)
    : QObject(parent)
    , dataDir(QDir(QDir::currentPath()).absoluteFilePath("../project code/Data"))
{

    QDir dir(dataDir);
    if (!dir.exists()) {

        dataDir = QDir(QDir::currentPath()).absoluteFilePath("../../project code/Data");
        dir.setPath(dataDir);

    }

    if (!dir.exists()) {

        return;
    }

    QString workoutsPath = dir.absoluteFilePath("workouts.json");
    QString logsPath = dir.absoluteFilePath("workout_logs.json");

    bool initialized = initializeFromFile();

    if (!initialized) {

        std::cout << "FAILED WORKOUT";
    } else {

        std::cout << "WORKOUT loaded " << workouts.size();
    }

}

WorkoutDataManager::~WorkoutDataManager() {
    if (dataModified) {
        saveToFile();
    }
}

bool WorkoutDataManager::initializeFromFile() {

    bool workoutsLoaded = loadWorkouts();

    bool logsLoaded = loadWorkoutLogs();

    return workoutsLoaded && logsLoaded;
}

bool WorkoutDataManager::saveToFile() {
    return saveWorkouts() && saveWorkoutLogs();
}

bool WorkoutDataManager::loadWorkouts() {
    QDir dir(dataDir);
    QString filePath = dir.absoluteFilePath("workouts.json");

    if (!QFile::exists(filePath)) {

        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {

        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {

        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {

        return false;
    }

    if (!doc.isObject()) {

        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("workouts")) {

        return false;
    }

    QJsonArray workoutsArray = root["workouts"].toArray();
    workouts.clear();

    for (const QJsonValue& value : workoutsArray) {
        if (!value.isObject()) {

            continue;
        }
        workouts.append(jsonToWorkout(value.toObject()));
    }

    if (!workouts.isEmpty()) {

    }

    return true;
}

bool WorkoutDataManager::saveWorkouts() const {
    QFile file(QDir(dataDir).absoluteFilePath("workouts.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray workoutsArray;
    for (const Workout& workout : workouts) {
        workoutsArray.append(workoutToJson(workout));
    }

    QJsonObject root;
    root["workouts"] = workoutsArray;

    QJsonDocument doc(root);
    file.write(doc.toJson());
    return true;
}

bool WorkoutDataManager::loadWorkoutLogs() {
    QDir dir(dataDir);
    QString filePath = dir.absoluteFilePath("workout_logs.json");

    bool fileExists = QFile::exists(filePath);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {

        QJsonObject root;
        root["logs"] = QJsonArray();

        QFile defaultFile(filePath);
        if (defaultFile.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(root);
            defaultFile.write(doc.toJson());
            defaultFile.close();

            workoutLogs.clear();

            return true;
        }

        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {

        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {

        return false;
    }

    if (!doc.isObject()) {

        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("logs")) {

        return false;
    }

    QJsonArray logsArray = root["logs"].toArray();
    workoutLogs.clear();

    for (const QJsonValue& value : logsArray) {
        if (!value.isObject()) {

            continue;
        }
        workoutLogs.append(jsonToWorkoutLog(value.toObject()));
    }

    return true;
}

bool WorkoutDataManager::saveWorkoutLogs() const {
    QFile file(QDir(dataDir).absoluteFilePath("workout_logs.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray logsArray;
    for (const WorkoutLog& log : workoutLogs) {
        logsArray.append(workoutLogToJson(log));
    }

    QJsonObject root;
    root["logs"] = logsArray;

    QJsonDocument doc(root);
    file.write(doc.toJson());
    return true;
}

QVector<Workout> WorkoutDataManager::getAllWorkouts() const {
    return workouts;
}

Workout WorkoutDataManager::getWorkoutById(int workoutId) const {
    for (const Workout& workout : workouts) {
        if (workout.id == workoutId) {
            return workout;
        }
    }
    return Workout();
}

QVector<Workout> WorkoutDataManager::getWorkoutsByDifficulty(const QString& difficulty) const {
    QVector<Workout> result;
    for (const Workout& workout : workouts) {
        if (workout.difficulty == difficulty) {
            result.append(workout);
        }
    }
    return result;
}

bool WorkoutDataManager::logWorkout(const WorkoutLog& log, QString& errorMessage) {
    workoutLogs.append(log);
    dataModified = true;
    return saveWorkoutLogs();
}

QVector<WorkoutLog> WorkoutDataManager::getUserWorkoutLogs(int userId) const {
    QVector<WorkoutLog> result;
    for (const WorkoutLog& log : workoutLogs) {
        if (log.userId == userId) {
            result.append(log);
        }
    }
    return result;
}

QVector<WorkoutLog> WorkoutDataManager::getUserWorkoutLogsByClass(int userId, int classId) const {
    QVector<WorkoutLog> result;
    for (const WorkoutLog& log : workoutLogs) {
        if (log.userId == userId && log.classId == classId) {
            result.append(log);
        }
    }
    return result;
}

QVector<WorkoutLog> WorkoutDataManager::getUserWorkoutLogsByDateRange(int userId, const QDate& startDate, const QDate& endDate) const {
    QVector<WorkoutLog> result;
    for (const WorkoutLog& log : workoutLogs) {
        if (log.userId == userId &&
            log.timestamp.date() >= startDate &&
            log.timestamp.date() <= endDate) {
            result.append(log);
        }
    }
    return result;
}

int WorkoutDataManager::getTotalCaloriesBurnt(int userId) const {
    int total = 0;
    for (const WorkoutLog& log : workoutLogs) {
        if (log.userId == userId) {
            total += log.totalCaloriesBurnt;
        }
    }
    return total;
}

int WorkoutDataManager::getTotalWorkoutsCompleted(int userId) const {
    int total = 0;
    for (const WorkoutLog& log : workoutLogs) {
        if (log.userId == userId) {
            total++;
        }
    }
    return total;
}

QMap<QString, int> WorkoutDataManager::getMostFrequentExercises(int userId) const {
    QMap<QString, int> exerciseCounts;
    for (const WorkoutLog& log : workoutLogs) {
        if (log.userId == userId) {
            for (const auto& exercise : log.completedExercises) {
                if (exercise.second) {
                    exerciseCounts[exercise.first]++;
                }
            }
        }
    }
    return exerciseCounts;
}

QJsonObject WorkoutDataManager::workoutToJson(const Workout& workout) {
    QJsonObject obj;
    obj["id"] = workout.id;
    obj["name"] = workout.name;
    obj["duration"] = workout.duration;
    obj["totalCalories"] = workout.totalCalories;
    obj["difficulty"] = workout.difficulty;

    QJsonArray exercisesArray;
    for (const Exercise& exercise : workout.exercises) {
        QJsonObject exerciseObj;
        exerciseObj["name"] = exercise.name;
        exerciseObj["sets"] = exercise.sets;
        exerciseObj["reps"] = exercise.reps;
        exerciseObj["caloriesPerSet"] = exercise.caloriesPerSet;
        exercisesArray.append(exerciseObj);
    }
    obj["exercises"] = exercisesArray;

    return obj;
}

Workout WorkoutDataManager::jsonToWorkout(const QJsonObject& json) {
    Workout workout;
    workout.id = json["id"].toInt();
    workout.name = json["name"].toString();
    workout.duration = json["duration"].toInt();
    workout.totalCalories = json["totalCalories"].toInt();
    workout.difficulty = json["difficulty"].toString();

    QJsonArray exercisesArray = json["exercises"].toArray();
    for (const QJsonValue& value : exercisesArray) {
        QJsonObject exerciseObj = value.toObject();
        Exercise exercise;
        exercise.name = exerciseObj["name"].toString();
        exercise.sets = exerciseObj["sets"].toInt();
        exercise.reps = exerciseObj["reps"].toString();
        exercise.caloriesPerSet = exerciseObj["caloriesPerSet"].toInt();
        workout.exercises.append(exercise);
    }

    return workout;
}

QJsonObject WorkoutDataManager::workoutLogToJson(const WorkoutLog& log) {
    QJsonObject obj;
    obj["userId"] = log.userId;
    obj["classId"] = log.classId;
    obj["workoutId"] = log.workoutId;
    obj["timestamp"] = log.timestamp.toString(Qt::ISODate);
    obj["totalCaloriesBurnt"] = log.totalCaloriesBurnt;

    QJsonArray exercisesArray;
    for (const auto& exercise : log.completedExercises) {
        QJsonObject exerciseObj;
        exerciseObj["name"] = exercise.first;
        exerciseObj["completed"] = exercise.second;
        exercisesArray.append(exerciseObj);
    }
    obj["completedExercises"] = exercisesArray;

    return obj;
}

WorkoutLog WorkoutDataManager::jsonToWorkoutLog(const QJsonObject& json) {
    WorkoutLog log;
    log.userId = json["userId"].toInt();
    log.classId = json["classId"].toInt();
    log.workoutId = json["workoutId"].toInt();
    log.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    log.totalCaloriesBurnt = json["totalCaloriesBurnt"].toInt();

    QJsonArray exercisesArray = json["completedExercises"].toArray();
    for (const QJsonValue& value : exercisesArray) {
        QJsonObject exerciseObj = value.toObject();
        QString name = exerciseObj["name"].toString();
        bool completed = exerciseObj["completed"].toBool();
        log.completedExercises.append(qMakePair(name, completed));
    }

    return log;
}