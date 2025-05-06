#ifndef AVAILABLECLASSESSCREEN_H
#define AVAILABLECLASSESSCREEN_H

#include <QWidget>
#include <QVector>
#include "classdatamanager.h"
#include "Gym/staff.h"

class QGridLayout;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QComboBox;

class AvailableClassesScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent = nullptr);
    ~AvailableClassesScreen();

private slots:
    void handleEnrollment(int classId);
    void handleWaitlist(int classId);
    void showAddClassDialog();

private:
    void setupUI();
    void refreshClasses();
    void createClassCard(const Class &gymClass, QGridLayout *classesGrid, int row, int col);
    void setupCoaches();

    ClassDataManager* classDataManager;
    QScrollArea* scrollArea;
    QWidget* scrollWidget;
    QPushButton* addClassButton;
    QVBoxLayout* mainVLayout;

    // dummy :))
    QVector<Staff> coaches;
};

#endif