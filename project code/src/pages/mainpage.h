#ifndef MAINPAGE_H
#define MAINPAGE_H

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
#include "Padel/BookingWindow.h"
#include <QLabel>
#include "../Language/LanguageSelector.h"
#include "Gym/availableclassesscreen.h"
//#include "../../UI/Widgets/Clock.h"
#include "../../UI/Widgets/Notifications/Notifications.h"
#include "Stylesheets/System/mainpageStyle.h"

class MainPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainPage(UserDataManager* userDataManager,
                     MemberDataManager* memberDataManager,
                     ClassDataManager* classDataManager,
                     PadelDataManager* padelDataManager,
                     QWidget *parent = nullptr);
    ~MainPage();
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
    void handleWorkoutPage() const;
    void handleNutritionPage() const;
    void handleProfilePage() const;
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
    QPushButton* workoutButton;
    QPushButton* nutritionButton;
    QPushButton* profileButton;
    QPushButton* settingsButton;

    // Pages
    HomePage* homePage;
    BookingWindow* workoutPage;
    AvailableClassesScreen* nutritionPage;
    QWidget* profilePage;
    SettingsPage* settingsPage;

    QLabel* titleLabel;
    QScrollArea* scrollArea;
    LanguageSelector* languageSelector;
    NotificationWidget* notificationWidget;

    ClockWidget* clockWidget;
};

#endif // MAINPAGE_H 