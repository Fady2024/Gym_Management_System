#include <QApplication>
#include <QScreen>
#include "../src/auth/authpage.h"
#include "../src/pages/mainpage.h"
#include "../src/onboarding/splashscreen.h"
#include "../src/onboarding/onboardingpage.h"
#include "../src/onboarding/languageselectionpage.h"
#include "../DataManager/userdatamanager.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/classdatamanager.h"
#include "../DataManager/padeldatamanager.h"
#include "../Language/LanguageManager.h"
#include "../Model/System/timeLogic.h"
#include <QSettings>
#include <QStackedWidget>
#include "../Core/AppInitializer.h"
#include <QCloseEvent>
#include "mainwindow.h"
#include "../src/pages/Gym/staffhomepage.h"
#include <QDebug>
#include <QTimer>

int main(int argc, char* argv[])
{
    AppInitializer::initializeApplication();
    QApplication app(argc, argv);
    QApplication::setApplicationName("FitFlex Pro");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;

    auto& languageManager = LanguageManager::getInstance();
    languageManager.setLanguage("en");
    QString savedLanguage = settings.value("language").toString();
    if (!savedLanguage.isEmpty() && savedLanguage != "en") {
        languageManager.setLanguage(savedLanguage);
    } else {
        settings.setValue("language", "en");
    }

    const QIcon appIcon(":/Images/dumbbell.png");
    QApplication::setWindowIcon(appIcon);

    const auto userDataManager = new UserDataManager(&app);
    const auto memberDataManager = new MemberDataManager(&app);
    const auto classDataManager = new ClassDataManager(&app);
    const auto padelDataManager = new PadelDataManager(&app);

    memberDataManager->setUserDataManager(userDataManager);
    classDataManager->setMemberDataManager(memberDataManager);
    padelDataManager->setMemberDataManager(memberDataManager);
    timeLogicInstance.setMemberDataManager(memberDataManager);

    MainWindow mainWindow(userDataManager, memberDataManager, classDataManager, padelDataManager);
    mainWindow.setWindowIcon(appIcon);
    mainWindow.setWindowTitle(QObject::tr("FitFlex Pro"));

    const auto screen = QApplication::primaryScreen();
    const QRect screenGeometry = screen->geometry();
    mainWindow.setMinimumSize(800, 600);
    const int width = static_cast<int>(screenGeometry.width() * 0.85);
    const int height = static_cast<int>(screenGeometry.height() * 0.85);
    mainWindow.resize(width, height);
    const int x = (screenGeometry.width() - width) / 2;
    const int y = (screenGeometry.height() - height) / 2;
    mainWindow.move(x, y);
    mainWindow.showMaximized();

    auto stackedWidget = new QStackedWidget(&mainWindow);
    mainWindow.setCentralWidget(stackedWidget);

    auto splashScreen = new SplashScreen(&mainWindow);
    auto languageSelectionPage = new LanguageSelectionPage(&mainWindow);
    auto onboardingPage = new OnboardingPage(&mainWindow);
    auto authPage = new AuthPage(userDataManager);
    auto mainPage = new MainPage(userDataManager, memberDataManager, classDataManager, padelDataManager);
    auto staffHomePage = new StaffHomePage(userDataManager, memberDataManager, classDataManager, padelDataManager);

    stackedWidget->addWidget(splashScreen);
    stackedWidget->addWidget(languageSelectionPage);
    stackedWidget->addWidget(onboardingPage);
    stackedWidget->addWidget(authPage);
    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(staffHomePage);

    QObject::connect(splashScreen, &SplashScreen::animationFinished, [stackedWidget, languageSelectionPage]() {
        stackedWidget->setCurrentWidget(languageSelectionPage);
        languageSelectionPage->startAnimation();
    });

    QObject::connect(languageSelectionPage, &LanguageSelectionPage::languageSelected, [stackedWidget, onboardingPage]() {
        stackedWidget->setCurrentWidget(onboardingPage);
        onboardingPage->startAnimation();
    });

    QObject::connect(onboardingPage, &OnboardingPage::onboardingCompleted, [stackedWidget, authPage]() {
        stackedWidget->setCurrentWidget(authPage);
    });

    QObject::connect(authPage, &AuthPage::loginSuccessful, [stackedWidget, mainPage, staffHomePage, userDataManager, memberDataManager](const QString& email) {

        if (!userDataManager || !memberDataManager || !mainPage || !staffHomePage || !stackedWidget) {

            return;
        }

        try {

            static QString previousEmail;
            bool isAccountSwitch = !previousEmail.isEmpty() && previousEmail != email;

            if (isAccountSwitch) {

                try {
                    mainPage->clearUserData();

                } catch (const std::exception& e) {

                }

                QTimer::singleShot(500, [mainPage, email, stackedWidget, staffHomePage, userDataManager, memberDataManager, isStaff=email.toLower().endsWith("@admin.com") || email.toLower().endsWith("@staff.com")]() {
                    try {

                        User user;
                        try {
                            user = userDataManager->getUserData(email);
                            if (user.getId() <= 0) {

                            } else {

                            }
                        } catch (const std::exception& e) {

                        }

                        if (!isStaff && user.getId() > 0 && memberDataManager) {
                            try {
                                bool isMember = memberDataManager->userIsMember(user.getId());

                                if (isMember) {
                                    int memberId = memberDataManager->getMemberIdByUserId(user.getId());

                                    timeLogicInstance.setCurrentMemberId(memberId);
                                }
                            } catch (const std::exception& e) {

                            }
                        }

                        if (isStaff) {

                            staffHomePage->handleLogin(email);
                            stackedWidget->setCurrentWidget(staffHomePage);
                        } else {

                            mainPage->handleLogin(email);
                            stackedWidget->setCurrentWidget(mainPage);
                        }
                    } catch (const std::exception& e) {

                    } catch (...) {

                    }
                });

                previousEmail = email;
                return;
            }
            previousEmail = email;

            User user = userDataManager->getUserData(email);

            bool isStaff = email.toLower().endsWith("@admin.com") || email.toLower().endsWith("@staff.com");

            if (isStaff) {
                try {

                    staffHomePage->handleLogin(email);

                    stackedWidget->setCurrentWidget(staffHomePage);

                } catch (const std::exception& e) {

                } catch (...) {

                }
            } else {

                if (user.getId() > 0) {
                    try {
                        QString errorMsg;

                        bool isMember = false;
                        try {
                            isMember = memberDataManager->userIsMember(user.getId());

                            if (isMember) {
                                int memberId = memberDataManager->getMemberIdByUserId(user.getId());
                                timeLogicInstance.setCurrentMemberId(memberId);
                            }
                        } catch (const std::exception& e) {

                        } catch (...) {

                        }

                        try {
                            mainPage->handleLogin(email);
                        } catch (const std::exception& e) {

                            throw;
                        }

                        if (stackedWidget->indexOf(mainPage) < 0) {

                            stackedWidget->addWidget(mainPage);
                        }
                        stackedWidget->setCurrentWidget(mainPage);

                    } catch (const std::exception& e) {

                    } catch (...) {

                    }
                } else {

                    try {

                        mainPage->handleLogin(email);

                        if (stackedWidget->indexOf(mainPage) < 0) {

                            stackedWidget->addWidget(mainPage);
                        }
                        stackedWidget->setCurrentWidget(mainPage);
                    } catch (const std::exception& e) {

                    } catch (...) {

                    }
                }
            }
        } catch (const std::exception& e) {

        } catch (...) {

        }
    });

    QObject::connect(mainPage, &MainPage::logoutRequested, [stackedWidget, authPage]() {
        stackedWidget->setCurrentWidget(authPage);
    });

    QObject::connect(staffHomePage, &StaffHomePage::logoutRequested, [stackedWidget, authPage]() {
        stackedWidget->setCurrentWidget(authPage);
    });

    stackedWidget->setCurrentWidget(splashScreen);
    splashScreen->startAnimation();

    const int result = QApplication::exec();

    delete userDataManager;
    delete memberDataManager;
    delete classDataManager;
    delete padelDataManager;

    return result;
}