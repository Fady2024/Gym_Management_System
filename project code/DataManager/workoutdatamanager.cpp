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
    qDebug() << "========== WorkoutDataManager constructor called ==========";
    qDebug() << "Current directory:" << QDir::currentPath();
    qDebug() << "Data directory:" << dataDir;
    
    // Ensure data directory exists
    QDir dir(dataDir);
    if (!dir.exists()) {
        qDebug() << "ERROR: Data directory does not exist:" << dataDir;
        qDebug() << "Trying alternative path...";
        // Try alternative path
        dataDir = QDir(QDir::currentPath()).absoluteFilePath("../../project code/Data");
        dir.setPath(dataDir);
        qDebug() << "Alternative data directory:" << dataDir;
        qDebug() << "Alternative directory exists:" << dir.exists();
    }
    
    if (!dir.exists()) {
        qDebug() << "ERROR: Could not find Data directory in either location";
        return;
    }
    
    // Check if key files exist
    QString workoutsPath = dir.absoluteFilePath("workouts.json");
    QString logsPath = dir.absoluteFilePath("workout_logs.json");
    
    qDebug() << "Workouts file path:" << workoutsPath;
    qDebug() << "Workouts file exists:" << QFile::exists(workoutsPath);
    qDebug() << "Logs file path:" << logsPath;
    qDebug() << "Logs file exists:" << QFile::exists(logsPath);
    
    bool initialized = initializeFromFile();
    qDebug() << "Initialization result:" << (initialized ? "SUCCESS" : "FAILED");
    
    if (!initialized) {
        qDebug() << "Failed to initialize workout data from files";
        std::cout << "FAILED WORKOUT";
    } else {
        qDebug() << "Successfully loaded" << workouts.size() << "workouts";
        std::cout << "WORKOUT loaded " << workouts.size();
    }
    qDebug() << "========== WorkoutDataManager constructor finished ==========";
}

WorkoutDataManager::~WorkoutDataManager() {
    if (dataModified) {
        saveToFile();
    }
}

bool WorkoutDataManager::initializeFromFile() {
    qDebug() << "========== Initializing workout data from files... ==========";
    bool workoutsLoaded = loadWorkouts();
    qDebug() << "Workouts loaded:" << workoutsLoaded;
    
    bool logsLoaded = loadWorkoutLogs();
    qDebug() << "Logs loaded:" << logsLoaded;
    
    qDebug() << "========== Workout data initialization " << (workoutsLoaded && logsLoaded ? "SUCCEEDED" : "FAILED") << " ==========";
    return workoutsLoaded && logsLoaded;
}

bool WorkoutDataManager::saveToFile() {
    return saveWorkouts() && saveWorkoutLogs();
}

bool WorkoutDataManager::loadWorkouts() {
    QDir dir(dataDir);
    QString filePath = dir.absoluteFilePath("workouts.json");
    qDebug() << "========== Loading workouts from:" << filePath << "==========";
    
    if (!QFile::exists(filePath)) {
        qDebug() << "ERROR: Workouts file doesn't exist at path:" << filePath;
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Failed to open workouts file:" << file.errorString();
        qDebug() << "File path:" << filePath;
        qDebug() << "Current directory:" << QDir::currentPath();
        qDebug() << "File permissions:" << file.permissions();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        qDebug() << "ERROR: Workouts file is empty";
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {
        qDebug() << "ERROR: Failed to parse workouts JSON data";
        qDebug() << "Parse error:" << parseError.errorString();
        qDebug() << "Error offset:" << parseError.offset;
        qDebug() << "File content:" << data;
        return false;
    }

    if (!doc.isObject()) {
        qDebug() << "ERROR: JSON document is not an object";
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("workouts")) {
        qDebug() << "ERROR: JSON does not contain 'workouts' key";
        return false;
    }

    QJsonArray workoutsArray = root["workouts"].toArray();
    workouts.clear();

    for (const QJsonValue& value : workoutsArray) {
        if (!value.isObject()) {
            qDebug() << "WARNING: Skipping invalid workout entry (not an object)";
            continue;
        }
        workouts.append(jsonToWorkout(value.toObject()));
    }

    qDebug() << "Successfully loaded" << workouts.size() << "workouts";
    if (!workouts.isEmpty()) {
        qDebug() << "First workout name:" << workouts.first().name;
        qDebug() << "First workout exercises:" << workouts.first().exercises.size();
    }
    qDebug() << "========== Workouts loading completed ==========";
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
    qDebug() << "========== Loading workout logs from:" << filePath << "==========";
    
    // Check if file exists
    bool fileExists = QFile::exists(filePath);
    qDebug() << "Workout logs file exists:" << fileExists;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open workout logs file:" << file.errorString();
        qDebug() << "File path:" << filePath;
        qDebug() << "Current directory:" << QDir::currentPath();
        qDebug() << "File permissions:" << file.permissions();
        
        // Create a default empty workout_logs.json file
        QJsonObject root;
        root["logs"] = QJsonArray();
        
        QFile defaultFile(filePath);
        if (defaultFile.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(root);
            defaultFile.write(doc.toJson());
            defaultFile.close();
            
            workoutLogs.clear();
            qDebug() << "Created default empty workout logs file at:" << filePath;
            qDebug() << "========== Workout logs loading completed with default file ==========";
            return true;
        }
        
        qDebug() << "ERROR: Failed to create default workout logs file";
        qDebug() << "========== Workout logs loading FAILED ==========";
        return false;
    }

    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        qDebug() << "ERROR: Workout logs file is empty";
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {
        qDebug() << "ERROR: Failed to parse workout logs JSON data";
        qDebug() << "Parse error:" << parseError.errorString();
        qDebug() << "Error offset:" << parseError.offset;
        qDebug() << "File content:" << data;
        return false;
    }

    if (!doc.isObject()) {
        qDebug() << "ERROR: JSON document is not an object";
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("logs")) {
        qDebug() << "ERROR: JSON does not contain 'logs' key";
        return false;
    }

    QJsonArray logsArray = root["logs"].toArray();
    workoutLogs.clear();

    for (const QJsonValue& value : logsArray) {
        if (!value.isObject()) {
            qDebug() << "WARNING: Skipping invalid workout log entry (not an object)";
            continue;
        }
        workoutLogs.append(jsonToWorkoutLog(value.toObject()));
    }

    qDebug() << "Successfully loaded" << workoutLogs.size() << "workout logs";
    qDebug() << "========== Workout logs loading completed ==========";
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
                if (exercise.second) { // Only count completed exercises
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