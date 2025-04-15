#include "mainwindow.h"
#include <QCloseEvent>

MainWindow::MainWindow(UserDataManager* userDataManager, QWidget* parent)
    : QMainWindow(parent), userDataManager(userDataManager)
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (userDataManager) {
        userDataManager->handleApplicationClosing();
    }
    event->accept();
} 