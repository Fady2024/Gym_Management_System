#include <QApplication>
#include <QScreen>
#include "../Start app/authpage.h"
#include "../Start app/mainpage.h"
#include "../Start app/splashscreen.h"
#include "../Start app/onboardingpage.h"
#include "../Start app/languageselectionpage.h"
#include "../DataManager/userdatamanager.h"
#include "../Util/LanguageManager.h"
#include <QSettings>
#include <QStackedWidget>

int main(int argc, char* argv[])
{
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

    // Create main window
    QMainWindow mainWindow;
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

    // Create pages
    const auto userDataManager = new UserDataManager(&app);
    auto splashScreen = new SplashScreen(&mainWindow);
    auto languageSelectionPage = new LanguageSelectionPage(&mainWindow);
    auto onboardingPage = new OnboardingPage(&mainWindow);
    auto authPage = new AuthPage(userDataManager);
    auto mainPage = new MainPage(userDataManager);

    // Add pages to stacked widget
    stackedWidget->addWidget(splashScreen);
    stackedWidget->addWidget(languageSelectionPage);
    stackedWidget->addWidget(onboardingPage);
    stackedWidget->addWidget(authPage);
    stackedWidget->addWidget(mainPage);

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

    QObject::connect(authPage, &AuthPage::loginSuccessful, [stackedWidget, mainPage](const QString& email) {
        mainPage->handleLogin(email);
        stackedWidget->setCurrentWidget(mainPage);
    });

    QObject::connect(mainPage, &MainPage::logoutRequested, [stackedWidget, authPage]() {
        stackedWidget->setCurrentWidget(authPage);
    });

    // Start with splash screen
    stackedWidget->setCurrentWidget(splashScreen);
    splashScreen->startAnimation();

    const int result = QApplication::exec();
    delete userDataManager;

    return result;
}