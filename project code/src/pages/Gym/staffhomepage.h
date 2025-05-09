#ifndef STAFFHOMEPAGE_H
#define STAFFHOMEPAGE_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include "../DataManager/userdatamanager.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/classdatamanager.h"
#include "../DataManager/padeldatamanager.h"
#include "homepage.h"
#include "settingspage.h"
#include "../staff/addmemberpage.h"
#include "../staff/retrievepage.h"
#include "../staff/searchmemberpage.h"
#include <QLabel>
#include "../Language/LanguageSelector.h"

class StaffHomePage : public QMainWindow
{
    Q_OBJECT

public:
    explicit StaffHomePage(UserDataManager* userDataManager, MemberDataManager* memberDataManager,
        ClassDataManager* classDataManager, QWidget* parent = nullptr);
    explicit StaffHomePage(UserDataManager* userDataManager, MemberDataManager* memberDataManager,
        ClassDataManager* classDataManager, PadelDataManager* padelDataManager,
        QWidget* parent = nullptr);
    ~StaffHomePage();
    void handleHomePage() const;
    void clearUserData();
    void handleLogin(const QString& email);
    QString getCurrentUserEmail() const { return currentUserEmail; }
signals:
    void logoutRequested();
    void userDataLoaded(const QString& email);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    bool event(QEvent* e) override {
        if (e->type() == QEvent::LanguageChange) {
            retranslateUI();
            return true;
        }
        return QMainWindow::event(e);
    }

private slots:
    void handleAddMemberPage() const;
    void handleRetrievePage() const;
    void handleSearchMemberPage() const;
    void handleSettingsPage() const;
    void toggleTheme();
    void onLanguageChanged(const QString& language);

private:
    void setupUI();
    void setupPages();
    void updateTheme(bool isDark);
    void updateButtonStates(QPushButton* activeButton) const;
    void updateAllTextColors();
    void updateLayout();
    void updateNavBarStyle();
    void retranslateUI();

    UserDataManager* userDataManager;
    MemberDataManager* memberDataManager;
    ClassDataManager* classDataManager;
    PadelDataManager* padelDataManager;
    QStackedWidget* stackedWidget;
    bool isDarkTheme;
    QString currentUserEmail;

    // Navigation buttons
    QPushButton* homeButton;
    QPushButton* addMemberButton;
    QPushButton* retrieveButton;
    QPushButton* searchMemberButton;
    QPushButton* settingsButton;

    // Pages
    HomePage* homePage;
    QWidget* addMemberPage;
    QWidget* retrievePage;
    QWidget* searchMemberPage;
    SettingsPage* settingsPage;

    QLabel* titleLabel;
    QScrollArea* scrollArea;
    LanguageSelector* languageSelector;
};

#endif // STAFFHOMEPAGE_H 