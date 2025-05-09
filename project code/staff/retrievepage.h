#ifndef RETRIEVEPAGE_H
#define RETRIEVEPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "LeftSidebar.h"
#include "UserDataManager.h"
#include "MemberDataManager.h"

class RetrievePage : public QWidget
{
    Q_OBJECT

public:
    explicit RetrievePage(UserDataManager* userDataManager, MemberDataManager* memberManager, QWidget* parent = nullptr);
    void updateTheme(bool isDark);

private slots:
    void handlePageChange(const QString& pageId);

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

#endif // RETRIEVEPAGE_H