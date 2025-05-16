// AvailableClassesScreen.h

#ifndef AVAILABLECLASSESSCREEN_H
#define AVAILABLECLASSESSCREEN_H

#include <QStackedWidget>
#include "classdatamanager.h"
#include "../../../Model/Gym/staff.h"
#include "../../../Model/System/user.h"
#include "../../../Model/Gym/member.h"
#include "../UI/leftsidebar.h"
#include "../../../DataManager/userdatamanager.h"
#include "../../../DataManager/memberdatamanager.h"
#include "../../../DataManager/workoutdatamanager.h"

class QGridLayout;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QLabel;

class AvailableClassesScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent = nullptr);
    ~AvailableClassesScreen();
    void setCurrentUserEmail(const QString& email);
    void setUserDataManager(UserDataManager* manager);
    void setMemberDataManager(MemberDataManager* manager);
    void setWorkoutDataManager(WorkoutDataManager* manager);
    void updateTheme(bool isDark);

private slots:
    void handleEnrollment(int classId);
    void handleUnenroll(int classId);
    void handleWaitlist(int classId);
    void showAddClassDialog();
    void loadUserData();

private:
    void setupUI();
    void setupCoaches();
    void refreshClasses();
    void createClassCard(const Class &gymClass, QGridLayout *classesGrid, int row, int col);
    void handlePageChange(const QString &pageID);

    QWidget* ClassesContent();
    QWidget* WorkoutsContent();
    QWidget* ExtraContent();

    QStackedWidget*    contentStack;
    ClassDataManager*  classDataManager;
    UserDataManager*   userDataManager;
    MemberDataManager* memberDataManager;
    WorkoutDataManager* workoutManager;
    QScrollArea*       scrollArea;
    QWidget*           scrollWidget;
    QPushButton*       addClassButton;
    QWidget*           classesContent;
    QHBoxLayout*       mainHLayout;

    QLabel*            userNameLabel;
    QLabel*            enrolledClassesLabel;

    QVector<Staff>     coaches;
    User               currentUser;
    Member             currentMember;
    QSet<int>          enrolledClassIds;
    QString            currentUserEmail;

    LeftSidebar*       leftSidebar;
    bool               isDarkTheme;
};

#endif
