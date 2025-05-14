#ifndef SEARCHMEMBERPAGE_H
#define SEARCHMEMBERPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "LeftSidebar.h"
#include "UserDataManager.h"
#include "MemberDataManager.h"
#include <QLabel>
#include <QLineEdit>

class SearchMember : public QWidget
{
    Q_OBJECT

public:
    explicit SearchMember(UserDataManager* userDataManager, MemberDataManager* memberManager, QWidget* parent = nullptr);
    void updateTheme(bool isDark);

private slots:
    void handlePageChange(const QString& pageId);
    void handleSearchClicked(QLineEdit* input, QLabel* resultLabel);
   
private:
    void setupUI();
    void showMessageDialog(const QString& message, bool isError = false);

    UserDataManager* userDataManager;
    MemberDataManager* memberManager;
    QHBoxLayout* mainLayout;
    LeftSidebar* leftSidebar;
    QStackedWidget* contentStack;
    QWidget* customContent;
    bool isDarkTheme;
    int currentUserId;
};

#endif // SEARCHMEMBERPAGE_H