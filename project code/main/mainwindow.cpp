#include "mainwindow.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(UserDataManager* userDataManager, 
                      MemberDataManager* memberDataManager, 
                      ClassDataManager* classDataManager,
                      PadelDataManager* padelDataManager,
                      QWidget* parent)
    : QMainWindow(parent), 
      userDataManager(userDataManager),
      memberDataManager(memberDataManager),
      classDataManager(classDataManager),
      padelDataManager(padelDataManager)
{
    qDebug() << "Main window created - data will be saved on application exit";
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    qDebug() << "Application closing - saving all data...";
    
    bool saveSuccess = true;

    if (userDataManager) {
        qDebug() << "Saving user data...";
        userDataManager->handleApplicationClosing();
    } else {
        qDebug() << "Warning: UserDataManager is null, unable to save user data";
        saveSuccess = false;
    }
    
    if (memberDataManager) {
        qDebug() << "Saving member data...";
        memberDataManager->handleApplicationClosing();
    } else {
        qDebug() << "Warning: MemberDataManager is null, unable to save member data";
        saveSuccess = false;
    }
    
    if (classDataManager) {
        qDebug() << "Saving class data...";
        classDataManager->handleApplicationClosing();
    } else {
        qDebug() << "Warning: ClassDataManager is null, unable to save class data";
        saveSuccess = false;
    }

    if (padelDataManager) {
        qDebug() << "Saving padel data...";
        padelDataManager->handleApplicationClosing();
    } else {
        qDebug() << "Warning: PadelDataManager is null, unable to save padel data";
        saveSuccess = false;
    }
    
    if (!saveSuccess) {
        QMessageBox::warning(this, tr("Save Warning"), 
                            tr("Some data may not have been saved correctly."));
    } else {
        qDebug() << "All data saved successfully!";
    }
    
    event->accept();
} 