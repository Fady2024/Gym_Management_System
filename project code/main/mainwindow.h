#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include "../DataManager/userdatamanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(UserDataManager* userDataManager, QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    UserDataManager* userDataManager;
};

#endif // MAINWINDOW_H 