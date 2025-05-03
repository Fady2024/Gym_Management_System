#ifndef AVAILABLECLASSESSCREEN_H
#define AVAILABLECLASSESSCREEN_H

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QProgressBar>
#include "classdatamanager.h"

class AvailableClassesScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent = nullptr);
    ~AvailableClassesScreen();

private slots:
    void handleEnrollment(int classId);
    void handleWaitlist(int classId);

private:
    void setupUI();
    void refreshClasses();
    void showAddClassDialog();
    void createClassCard(const Class &gymClass);

    ClassDataManager* classDataManager;
    QGridLayout* classesGridLayout;
    QScrollArea* scrollArea;
    QWidget* scrollWidget;
    QPushButton* addClassButton;
};

#endif // AVAILABLECLASSESSCREEN_H