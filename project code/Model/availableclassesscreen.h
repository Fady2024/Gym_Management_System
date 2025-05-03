#ifndef AVAILABLECLASSESSCREEN_H
#define AVAILABLECLASSESSCREEN_H

#include <QWidget>
#include <QVector>
#include "class.h"

class QPushButton;
class QGridLayout;
class QScrollArea;
class QLabel;

class AvailableClassesScreen : public QWidget
{
public:
    explicit AvailableClassesScreen(QWidget *parent = nullptr);
    void addClass(const Class &gymClass);
    const QVector<Class>& getClasses() const { return classes; }
    void showAddClassDialog(); //edeny el 7alabesa

private:
    void setupUI();
    void createClassCard(const Class &gymClass);
    void updateClassesDisplay();
    void handleEnrollment(int classId) { /*  b3den ba2a */ }
    void handleWaitlist(int classId) { /* b3den ba2a */ }
    QVector<Class> classes;
    QGridLayout *classesGridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollWidget;
    QPushButton *addClassButton;
};

#endif