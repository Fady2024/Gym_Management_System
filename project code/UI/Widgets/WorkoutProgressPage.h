#ifndef WORKOUTPROGRESSPAGE_H
#define WORKOUTPROGRESSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCalendarWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QProgressBar>
#include "../../DataManager/workoutdatamanager.h"
#include "../../DataManager/classdatamanager.h"

class WorkoutProgressPage : public QWidget
{
    Q_OBJECT

public:
    explicit WorkoutProgressPage(QWidget *parent = nullptr);
    void setWorkoutDataManager(WorkoutDataManager* manager) { workoutManager = manager; }
    void setClassDataManager(ClassDataManager* manager) { classManager = manager; }
    void setUserId(int id) { userId = id; updateUI(); }
    void updateTheme(bool isDark);

private slots:
    void onDateSelected(const QDate& date);
    void onWorkoutSelected(int workoutId);
    void onAttendWorkout();
    void updateUI();
    void showWorkoutHistory();
    void showAvailableWorkouts();
    void showProgress();

private:
    void setupUI();
    void setupCalendar();
    void setupWorkoutList();
    void setupProgressView();
    void loadWorkouts(const QDate& date);
    void loadUserProgress();
    void createProgressBar(QVBoxLayout* layout, const QString& label);

    QVBoxLayout* mainLayout;
    QHBoxLayout* contentLayout;
    QCalendarWidget* calendar;
    QListWidget* workoutList;
    QWidget* progressWidget;
    QStackedWidget* contentStack;
    QPushButton* historyButton;
    QPushButton* workoutsButton;
    QPushButton* progressButton;
    QPushButton* attendButton;

    WorkoutDataManager* workoutManager;
    ClassDataManager* classManager;
    int userId;
    int selectedWorkoutId;
    bool isDarkTheme;

    // Progress tracking widgets
    QMap<QString, QProgressBar*> exerciseProgressBars;
    QProgressBar* caloriesProgressBar;
    QLabel* totalWorkoutsLabel;
    QLabel* averageCaloriesLabel;
};

#endif 