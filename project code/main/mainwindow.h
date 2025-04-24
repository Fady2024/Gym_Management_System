#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include "../DataManager/userdatamanager.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/classdatamanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(UserDataManager* userDataManager, MemberDataManager* memberDataManager, 
              ClassDataManager* classDataManager, QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    UserDataManager* userDataManager;
    MemberDataManager* memberDataManager;
    ClassDataManager* classDataManager;
};

#endif // MAINWINDOW_H 