#ifndef RETRIEVEPAGE_H
#define RETRIEVEPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "LeftSidebar.h"
#include "UserDataManager.h"
#include "MemberDataManager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QTableWidget>
#include <QVector>
#include <QHeaderView>
#include <QDebug>
#include <QLineEdit>
#include <QFormLayout>

#include "addmemberpage.h"

class RetrievePage : public QWidget
{
    Q_OBJECT

public:
    explicit RetrievePage(UserDataManager* userDataManager, MemberDataManager* memberManager, QWidget* parent = nullptr);
    void updateTheme(bool isDark);

private slots:
    void handlePageChange(const QString& pageId);
    void handleCellClick(int row, int column);
private:
    void setupUI();
    void showMessageDialog(const QString& message, bool isError = false);
    void populateTable(const QString& filter = QString());

    

    QLineEdit* searchEdit;
    QTableWidget* tableWidget;
    UserDataManager* userDataManager;
    MemberDataManager* memberManager;
    QHBoxLayout* mainLayout;
    LeftSidebar* leftSidebar;
    QStackedWidget* contentStack;
    QWidget* customContent;
    bool isDarkTheme;
    int currentUserId;

    AddMemberPage* addMemberPage;
};

#endif // RETRIEVEPAGE_H