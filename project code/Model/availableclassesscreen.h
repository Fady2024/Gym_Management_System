// AvailableClassesScreen.h

#ifndef AVAILABLECLASSESSCREEN_H
#define AVAILABLECLASSESSCREEN_H

#include <QWidget>
#include <QVector>
#include <QSet>
#include "classdatamanager.h"
#include "Gym/staff.h"
#include "System//user.h"
#include "Gym/member.h"

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

    ClassDataManager* classDataManager;
    QScrollArea*       scrollArea;
    QWidget*           scrollWidget;
    QPushButton*       addClassButton;
    QVBoxLayout*       mainVLayout;

    QLabel*            userNameLabel;
    QLabel*            enrolledClassesLabel;

    QVector<Staff>     coaches;
    User               currentUser;
    Member             currentMember;
    QSet<int>          enrolledClassIds;
};

#endif
