#include "WorkoutProgressPage.h"
#include "../../DataManager/workoutdatamanager.h"
#include "../../DataManager/classdatamanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCalendarWidget>
#include <QListWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollArea>

WorkoutProgressPage::WorkoutProgressPage(QWidget *parent)
    : QWidget(parent)
    , workoutManager(nullptr)
    , classManager(nullptr)
    , userId(0)
    , selectedWorkoutId(0)
    , isDarkTheme(false)
    , caloriesProgressBar(nullptr)
    , totalWorkoutsLabel(nullptr)
    , averageCaloriesLabel(nullptr)
{
    setupUI();
}

void WorkoutProgressPage::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Top navigation buttons
    auto* navLayout = new QHBoxLayout;
    historyButton = new QPushButton("History", this);
    workoutsButton = new QPushButton("Available Workouts", this);
    progressButton = new QPushButton("Progress", this);

    navLayout->addWidget(historyButton);
    navLayout->addWidget(workoutsButton);
    navLayout->addWidget(progressButton);
    mainLayout->addLayout(navLayout);

    // Content area
    contentStack = new QStackedWidget(this);
    mainLayout->addWidget(contentStack);

    // Setup different views
    setupCalendar();
    setupWorkoutList();
    setupProgressView();

    // Connect signals
    connect(historyButton, &QPushButton::clicked, this, &WorkoutProgressPage::showWorkoutHistory);
    connect(workoutsButton, &QPushButton::clicked, this, &WorkoutProgressPage::showAvailableWorkouts);
    connect(progressButton, &QPushButton::clicked, this, &WorkoutProgressPage::showProgress);

    // Set initial view
    showAvailableWorkouts();
}

void WorkoutProgressPage::createProgressBar(QVBoxLayout* layout, const QString& label) {
    auto* containerWidget = new QWidget;
    auto* containerLayout = new QHBoxLayout(containerWidget);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    auto* nameLabel = new QLabel(label);
    nameLabel->setMinimumWidth(150);
    auto* progressBar = new QProgressBar;
    progressBar->setTextVisible(true);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);

    containerLayout->addWidget(nameLabel);
    containerLayout->addWidget(progressBar);

    layout->addWidget(containerWidget);
    exerciseProgressBars[label] = progressBar;
}

void WorkoutProgressPage::setupCalendar() {
    auto* calendarWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(calendarWidget);

    calendar = new QCalendarWidget(this);
    calendar->setMinimumWidth(400);
    layout->addWidget(calendar);

    workoutList = new QListWidget(this);
    layout->addWidget(workoutList);

    attendButton = new QPushButton("Attend Workout", this);
    attendButton->setEnabled(false);
    layout->addWidget(attendButton);

    contentStack->addWidget(calendarWidget);

    connect(calendar, &QCalendarWidget::clicked, this, &WorkoutProgressPage::onDateSelected);
    connect(workoutList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0) {
            selectedWorkoutId = workoutList->item(row)->data(Qt::UserRole).toInt();
            attendButton->setEnabled(true);
        } else {
            selectedWorkoutId = 0;
            attendButton->setEnabled(false);
        }
    });
    connect(attendButton, &QPushButton::clicked, this, &WorkoutProgressPage::onAttendWorkout);
}

void WorkoutProgressPage::setupWorkoutList() {
    auto* historyWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(historyWidget);

    auto* historyList = new QListWidget(this);
    layout->addWidget(historyList);

    contentStack->addWidget(historyWidget);
}

void WorkoutProgressPage::setupProgressView() {
    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    progressWidget = new QWidget;
    auto* layout = new QVBoxLayout(progressWidget);
    layout->setSpacing(15);

    // Summary section
    auto* summaryGroup = new QWidget;
    auto* summaryLayout = new QVBoxLayout(summaryGroup);
    
    totalWorkoutsLabel = new QLabel("Total Workouts: 0");
    totalWorkoutsLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    summaryLayout->addWidget(totalWorkoutsLabel);

    averageCaloriesLabel = new QLabel("Average Calories per Workout: 0");
    averageCaloriesLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    summaryLayout->addWidget(averageCaloriesLabel);

    // Calories progress
    auto* caloriesWidget = new QWidget;
    auto* caloriesLayout = new QHBoxLayout(caloriesWidget);
    auto* caloriesLabel = new QLabel("Total Calories Burned");
    caloriesLabel->setMinimumWidth(150);
    caloriesProgressBar = new QProgressBar;
    caloriesProgressBar->setTextVisible(true);
    caloriesProgressBar->setMinimum(0);
    caloriesProgressBar->setMaximum(10000); // Assuming 10,000 calories as max
    caloriesLayout->addWidget(caloriesLabel);
    caloriesLayout->addWidget(caloriesProgressBar);

    layout->addWidget(summaryGroup);
    layout->addWidget(caloriesWidget);

    // Section for exercise frequency
    auto* exercisesLabel = new QLabel("Most Frequent Exercises");
    exercisesLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-top: 20px;");
    layout->addWidget(exercisesLabel);

    // We'll add exercise progress bars dynamically in loadUserProgress()

    layout->addStretch();
    scrollArea->setWidget(progressWidget);
    contentStack->addWidget(scrollArea);
}

void WorkoutProgressPage::onDateSelected(const QDate& date) {
    loadWorkouts(date);
}

void WorkoutProgressPage::loadWorkouts(const QDate& date) {
    if (!workoutManager) return;

    workoutList->clear();
    QVector<Workout> workouts = workoutManager->getAllWorkouts();

    for (const Workout& workout : workouts) {
        auto* item = new QListWidgetItem(workout.name);
        item->setData(Qt::UserRole, workout.id);
        workoutList->addItem(item);
    }
}

void WorkoutProgressPage::onAttendWorkout() {
    if (!workoutManager || !classManager || selectedWorkoutId == 0) return;

    QDateTime now = QDateTime::currentDateTime();
    WorkoutLog log;
    log.userId = userId;
    log.workoutId = selectedWorkoutId;
    log.timestamp = now;
    log.classId = 0; // Set to actual class ID if in a class

    Workout workout = workoutManager->getWorkoutById(selectedWorkoutId);
    log.totalCaloriesBurnt = workout.totalCalories;

    for (const Exercise& exercise : workout.exercises) {
        log.completedExercises.append(qMakePair(exercise.name, true));
    }

    QString errorMessage;
    if (workoutManager->logWorkout(log, errorMessage)) {
        QMessageBox::information(this, "Success", "Workout logged successfully!");
        updateUI();
    } else {
        QMessageBox::warning(this, "Error", "Failed to log workout: " + errorMessage);
    }
}

void WorkoutProgressPage::showWorkoutHistory() {
    contentStack->setCurrentIndex(1);
    updateUI();
}

void WorkoutProgressPage::showAvailableWorkouts() {
    contentStack->setCurrentIndex(0);
    loadWorkouts(calendar->selectedDate());
}

void WorkoutProgressPage::showProgress() {
    contentStack->setCurrentIndex(2);
    loadUserProgress();
}

void WorkoutProgressPage::loadUserProgress() {
    if (!workoutManager || userId == 0) return;

    // Get user's workout logs
    QVector<WorkoutLog> logs = workoutManager->getUserWorkoutLogs(userId);
    
    // Update summary statistics
    int totalWorkouts = logs.size();
    int totalCalories = 0;
    
    for (const WorkoutLog& log : logs) {
        totalCalories += log.totalCaloriesBurnt;
    }

    float avgCalories = totalWorkouts > 0 ? static_cast<float>(totalCalories) / totalWorkouts : 0;

    totalWorkoutsLabel->setText(QString("Total Workouts: %1").arg(totalWorkouts));
    averageCaloriesLabel->setText(QString("Average Calories per Workout: %1").arg(static_cast<int>(avgCalories)));
    
    // Update calories progress
    caloriesProgressBar->setValue(totalCalories);
    caloriesProgressBar->setFormat(QString("%1 calories").arg(totalCalories));

    // Update exercise frequency
    QMap<QString, int> exerciseCounts = workoutManager->getMostFrequentExercises(userId);
    
    // Find the maximum count for scaling
    int maxCount = 0;
    for (auto it = exerciseCounts.begin(); it != exerciseCounts.end(); ++it) {
        maxCount = qMax(maxCount, it.value());
    }

    // Create or update progress bars for each exercise
    for (auto it = exerciseCounts.begin(); it != exerciseCounts.end(); ++it) {
        QProgressBar* bar = exerciseProgressBars.value(it.key());
        if (!bar) {
            createProgressBar(qobject_cast<QVBoxLayout*>(progressWidget->layout()), it.key());
            bar = exerciseProgressBars[it.key()];
        }
        
        int percentage = maxCount > 0 ? (it.value() * 100) / maxCount : 0;
        bar->setValue(percentage);
        bar->setFormat(QString("%1 times (%2%)").arg(it.value()).arg(percentage));
    }
}

void WorkoutProgressPage::updateUI() {
    if (contentStack->currentIndex() == 0) {
        loadWorkouts(calendar->selectedDate());
    } else if (contentStack->currentIndex() == 1) {
        // Update history view
        auto* historyList = qobject_cast<QListWidget*>(contentStack->widget(1)->layout()->itemAt(0)->widget());
        historyList->clear();

        if (workoutManager) {
            QVector<WorkoutLog> logs = workoutManager->getUserWorkoutLogs(userId);
            for (const WorkoutLog& log : logs) {
                Workout workout = workoutManager->getWorkoutById(log.workoutId);
                QString text = QString("%1 - %2 - %3 calories")
                    .arg(log.timestamp.toString("yyyy-MM-dd hh:mm"))
                    .arg(workout.name)
                    .arg(log.totalCaloriesBurnt);
                historyList->addItem(text);
            }
        }
    } else if (contentStack->currentIndex() == 2) {
        loadUserProgress();
    }
}

void WorkoutProgressPage::updateTheme(bool isDark) {
    isDarkTheme = isDark;
    QString buttonStyle = isDark ? 
        "QPushButton { background-color: #2d3748; color: white; border: none; padding: 8px; border-radius: 4px; } "
        "QPushButton:hover { background-color: #4a5568; }" :
        "QPushButton { background-color: #e2e8f0; color: black; border: none; padding: 8px; border-radius: 4px; } "
        "QPushButton:hover { background-color: #cbd5e0; }";

    QString progressBarStyle = isDark ?
        "QProgressBar { border: 2px solid #4a5568; border-radius: 5px; text-align: center; background-color: #2d3748; color: white; } "
        "QProgressBar::chunk { background-color: #6366f1; border-radius: 3px; }" :
        "QProgressBar { border: 2px solid #e2e8f0; border-radius: 5px; text-align: center; background-color: #f8fafc; color: black; } "
        "QProgressBar::chunk { background-color: #818cf8; border-radius: 3px; }";

    QString labelStyle = isDark ?
        "QLabel { color: white; }" :
        "QLabel { color: black; }";

    historyButton->setStyleSheet(buttonStyle);
    workoutsButton->setStyleSheet(buttonStyle);
    progressButton->setStyleSheet(buttonStyle);
    attendButton->setStyleSheet(buttonStyle);

    if (caloriesProgressBar) {
        caloriesProgressBar->setStyleSheet(progressBarStyle);
    }

    for (QProgressBar* bar : exerciseProgressBars) {
        if (bar) {
            bar->setStyleSheet(progressBarStyle);
        }
    }

    if (totalWorkoutsLabel) {
        totalWorkoutsLabel->setStyleSheet(labelStyle + "font-size: 16px; font-weight: bold;");
    }
    if (averageCaloriesLabel) {
        averageCaloriesLabel->setStyleSheet(labelStyle + "font-size: 16px; font-weight: bold;");
    }
}

void WorkoutProgressPage::onWorkoutSelected(int workoutId) {
    selectedWorkoutId = workoutId;
    attendButton->setEnabled(workoutId > 0);
} 