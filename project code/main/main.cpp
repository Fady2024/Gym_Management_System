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

    // Set up application settings
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;

    // Initialize language manager and set default language if not set
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

    // Create data managers
    const auto userDataManager = new UserDataManager(&app);
    const auto memberDataManager = new MemberDataManager(&app);
    const auto classDataManager = new ClassDataManager(&app);
    const auto padelDataManager = new PadelDataManager(&app);
    
    // Connect the data managers
    memberDataManager->setUserDataManager(userDataManager);
    classDataManager->setMemberDataManager(memberDataManager);
    padelDataManager->setMemberDataManager(memberDataManager);

    // Create main window with all data managers
    MainWindow mainWindow(userDataManager, memberDataManager, classDataManager, padelDataManager);
    mainWindow.setWindowIcon(appIcon);
    mainWindow.setWindowTitle(QObject::tr("FitFlex Pro"));

    // Set up main window size and position
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

    // Create stacked widget for all pages
    auto stackedWidget = new QStackedWidget(&mainWindow);
    mainWindow.setCentralWidget(stackedWidget);

    // Create pages with necessary data managers
    auto splashScreen = new SplashScreen(&mainWindow);
    auto languageSelectionPage = new LanguageSelectionPage(&mainWindow);
    auto onboardingPage = new OnboardingPage(&mainWindow);
    auto authPage = new AuthPage(userDataManager);
    auto mainPage = new MainPage(userDataManager, memberDataManager, classDataManager, padelDataManager);
    auto staffHomePage = new StaffHomePage(userDataManager, memberDataManager, classDataManager, padelDataManager);

    // Add pages to stacked widget
    /*stackedWidget->addWidget(splashScreen);
    stackedWidget->addWidget(languageSelectionPage);
    stackedWidget->addWidget(onboardingPage);
    stackedWidget->addWidget(authPage);*/
    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(staffHomePage);

    // Connect signals and slots
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
        // More defensive checks to prevent crashes
        if (!userDataManager || !memberDataManager || !mainPage || !staffHomePage || !stackedWidget) {
            qDebug() << "Critical error: One or more required components is null";
            return;
        }

        try {
            qDebug() << "Starting login flow for: " << email;

            // Store the email in a static variable to track account changes
            static QString previousEmail;
            bool isAccountSwitch = !previousEmail.isEmpty() && previousEmail != email;
            
            if (isAccountSwitch) {
                qDebug() << "Account switch detected! Previous: " << previousEmail << ", New: " << email;
                
                // Force clearing of all cached data
                try {
                    mainPage->clearUserData();
                    qDebug() << "Successfully cleared user data during account switch";
                } catch (const std::exception& e) {
                    qDebug() << "Error clearing user data: " << e.what();
                }
                
                // Longer delay to ensure all data is cleared
                QTimer::singleShot(500, [mainPage, email, stackedWidget, staffHomePage, userDataManager, memberDataManager, isStaff=email.toLower().endsWith("@admin.com") || email.toLower().endsWith("@staff.com")]() {
                    try {
                        // Verify the user exists before proceeding
                        User user;
                        try {
                            user = userDataManager->getUserData(email);
                            if (user.getId() <= 0) {
                                qDebug() << "Warning: Invalid user ID during delayed login for: " << email;
                            } else {
                                qDebug() << "Found valid user ID: " << user.getId() << " for email: " << email;
                            }
                        } catch (const std::exception& e) {
                            qDebug() << "Exception getting user data: " << e.what();
                        }
                        
                        // For regular users, explicitly check their member status
                        if (!isStaff && user.getId() > 0 && memberDataManager) {
                            try {
                                bool isMember = memberDataManager->userIsMember(user.getId());
                                qDebug() << "User membership check during account switch: " << (isMember ? "Is member" : "Not member");
                                
                                if (isMember) {
                                    int memberId = memberDataManager->getMemberIdByUserId(user.getId());
                                    qDebug() << "Found member ID during account switch: " << memberId;
                                }
                            } catch (const std::exception& e) {
                                qDebug() << "Exception checking member status during account switch: " << e.what();
                            }
                        }
                        
                        // Route to appropriate page based on user type
                        if (isStaff) {
                            qDebug() << "Redirecting to staff page after account switch";
                            staffHomePage->handleLogin(email);
                            stackedWidget->setCurrentWidget(staffHomePage);
                        } else {
                            qDebug() << "Redirecting to main page after account switch";
                            mainPage->handleLogin(email);
                            stackedWidget->setCurrentWidget(mainPage);
                        }
                    } catch (const std::exception& e) {
                        qDebug() << "Exception during delayed login: " << e.what();
                    } catch (...) {
                        qDebug() << "Unknown exception during delayed login";
                    }
                });
                
                // Update previous email
                previousEmail = email;
                return;
            }
            previousEmail = email;

            // Get the user data
            User user = userDataManager->getUserData(email);
            qDebug() << "User logged in: " << email << " (ID: " << user.getId() << ")";
            
            // Determine if user is staff or regular user (simple check based on email domain)
            bool isStaff = email.toLower().endsWith("@admin.com") || email.toLower().endsWith("@staff.com");
            qDebug() << "User is staff: " << isStaff;
            
            if (isStaff) {
                try {
                    qDebug() << "Attempting to handle staff login";
                    staffHomePage->handleLogin(email);
                    qDebug() << "Setting current widget to staff home page";
                    stackedWidget->setCurrentWidget(staffHomePage);
                    qDebug() << "Redirecting to staff page";
                } catch (const std::exception& e) {
                    qDebug() << "Exception in staff redirection: " << e.what();
                } catch (...) {
                    qDebug() << "Unknown exception in staff redirection";
                }
            } else {
                qDebug() << "Handling regular user login";
                // For regular users, create member record if needed and go to main page
                if (user.getId() > 0) {
                    try {
                        QString errorMsg;
                        qDebug() << "Checking if user is already a member";
                        bool isMember = false;
                        try {
                            isMember = memberDataManager->userIsMember(user.getId());
                            qDebug() << "User is member: " << isMember;
                        } catch (const std::exception& e) {
                            qDebug() << "Exception checking member status: " << e.what();
                        } catch (...) {
                            qDebug() << "Unknown exception checking member status";
                        }
                        qDebug() << "Skipping automatic member creation - users become members after subscription";
                        
                        qDebug() << "Calling mainPage->handleLogin";
                        try {
                            mainPage->handleLogin(email);
                        } catch (const std::exception& e) {
                            qDebug() << "Exception in mainPage->handleLogin: " << e.what();
                            throw;
                        }
                        
                        qDebug() << "Setting current widget to main page";
                        if (stackedWidget->indexOf(mainPage) < 0) {
                            qDebug() << "ERROR: mainPage not found in stackedWidget! Re-adding it.";
                            stackedWidget->addWidget(mainPage);
                        }
                        stackedWidget->setCurrentWidget(mainPage);
                        qDebug() << "Redirecting to main page";
                    } catch (const std::exception& e) {
                        qDebug() << "Exception in member creation/redirection: " << e.what();
                    } catch (...) {
                        qDebug() << "Unknown exception in member creation/redirection";
                    }
                } else {
                    qDebug() << "Warning: User has invalid ID: " << user.getId();
                    try {
                        qDebug() << "Attempting to handle login for user with invalid ID";
                        mainPage->handleLogin(email);
                        qDebug() << "Setting current widget to main page";
                        if (stackedWidget->indexOf(mainPage) < 0) {
                            qDebug() << "ERROR: mainPage not found in stackedWidget for invalid user! Re-adding it.";
                            stackedWidget->addWidget(mainPage);
                        }
                        stackedWidget->setCurrentWidget(mainPage);
                    } catch (const std::exception& e) {
                        qDebug() << "Exception handling login for invalid user: " << e.what();
                    } catch (...) {
                        qDebug() << "Unknown exception handling login for invalid user";
                    }
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception during login flow: " << e.what();
        } catch (...) {
            qDebug() << "Unknown exception during login flow";
        }
    });

    QObject::connect(mainPage, &MainPage::logoutRequested, [stackedWidget, authPage]() {
        stackedWidget->setCurrentWidget(authPage);
    });

    QObject::connect(staffHomePage, &StaffHomePage::logoutRequested, [stackedWidget, authPage]() {
        stackedWidget->setCurrentWidget(authPage);
    });

    // Start with splash screen
    stackedWidget->setCurrentWidget(splashScreen);
    splashScreen->startAnimation();

    const int result = QApplication::exec();

    // Clean up data managers
    delete userDataManager;
    delete memberDataManager;
    delete classDataManager;
    delete padelDataManager;

    return result;
}
