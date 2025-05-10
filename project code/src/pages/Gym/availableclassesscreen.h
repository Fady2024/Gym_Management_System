// AvailableClassesScreen.h

#ifndef AVAILABLECLASSESSCREEN_H
#define AVAILABLECLASSESSCREEN_H

#include <QWidget>
#include <QVector>
#include <QSet>
#include <QStackedWidget>
#include "classdatamanager.h"
#include "../../../Model/Gym/staff.h"
#include "../../../Model/System/user.h"
#include "../../../Model/Gym/member.h"
#include "../UI/leftsidebar.h"

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
private slots:
    void handleEnrollment(int classId);
    void handleUnenroll(int classId);
    void handleWaitlist(int classId);
    void showAddClassDialog();

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

    LeftSidebar*       leftSidebar;
};

#endif
