#include "mainpage.h"
#include <QWidget>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QSettings>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QResizeEvent>
#include "../UI/TopPanel.h"
#include "../Theme/ThemeManager.h"
#include "../Language/LanguageManager.h"
#include "../Language/LanguageSelector.h"
#include <QDebug>
#include <QTimer>

MainPage::MainPage(UserDataManager* userDataManager, MemberDataManager* memberDataManager,
                   ClassDataManager* classDataManager, PadelDataManager* padelDataManager, QWidget* parent)
    : QMainWindow(parent)
    , userDataManager(userDataManager)
    , memberDataManager(memberDataManager)
    , classDataManager(classDataManager)
    , padelDataManager(padelDataManager)
{

    try {
        qDebug() << "MainPage constructor started";

        if (!userDataManager) {
            qDebug() << "Warning: userDataManager is null in MainPage constructor";
        }

        if (!memberDataManager) {
            qDebug() << "Warning: memberDataManager is null in MainPage constructor";
        }

        if (!classDataManager) {
            qDebug() << "Warning: classDataManager is null in MainPage constructor";
        }

        if (!padelDataManager) {
            qDebug() << "Warning: padelDataManager is null in MainPage constructor";
        }

        isDarkTheme = ThemeManager::getInstance().isDarkTheme();
        setupUI();
        updateTheme(isDarkTheme);

        // Connect to ThemeManager
        connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
                this, [this](bool isDark) {
                    isDarkTheme = isDark;
                    updateTheme(isDark);
                });

        // Connect to LanguageManager
        connect(&LanguageManager::getInstance(), &LanguageManager::languageChanged,
                this, &MainPage::onLanguageChanged);

        qDebug() << "MainPage constructor completed successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in MainPage constructor: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in MainPage constructor";
    }
}

MainPage::~MainPage()
{
    // Clean up any resources if needed
}

void MainPage::setupUI()
{
    // Set minimum size to prevent UI elements from being cut off
    setMinimumSize(800, 600);

    const auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Set transparent background for main window and central widget
    setStyleSheet("QMainWindow { background: transparent; }");
    centralWidget->setStyleSheet("QWidget { background: transparent; }");

    const auto mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(0, 0, 0, 0);

    // Navigation bar setup with glass effect
    const auto navBar = new QWidget;
    navBar->setObjectName("navBar");
    navBar->setFixedHeight(64);
    navBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    updateNavBarStyle();

    const auto navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(24, 0, 24, 0);
    navLayout->setSpacing(16);

    // Logo and title setup
    const auto logoContainer = new QWidget;
    logoContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    logoContainer->setStyleSheet("background: transparent;");
    const auto logoLayout = new QHBoxLayout(logoContainer);
    logoLayout->setContentsMargins(0, 0, 0, 0);
    logoLayout->setSpacing(8);

    const auto logoLabel = new QLabel;
    const QPixmap logo(":/Images/dumbbell.png");
    logoLabel->setPixmap(logo.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    titleLabel = new QLabel("FitFlex<span style='color: #7E69AB;'>Pro</span>");
    titleLabel->setStyleSheet(titleLabelStyle.arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    titleLabel->setTextFormat(Qt::RichText);

    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(titleLabel);

    // Create scrollable navigation area
    scrollArea = new QScrollArea;
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFixedHeight(64);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   background: transparent;"
        "   border: none;"
        "}"
        "QWidget#scrollContainer {"
        "   background: transparent;"
        "}"
    );
    scrollArea->setWidgetResizable(true);
    scrollArea->setMouseTracking(true);

    // Navigation buttons container
    const auto navButtonsContainer = new QWidget;
    navButtonsContainer->setObjectName("scrollContainer");
    navButtonsContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    navButtonsContainer->setMouseTracking(true);
    const auto navButtonsLayout = new QHBoxLayout(navButtonsContainer);
    navButtonsLayout->setSpacing(16);
    navButtonsLayout->setContentsMargins(0, 0, 0, 0);

    // Create navigation buttons with transparent background
    auto createNavButton = [this](const QString& text, const QString& emoji) {
        const auto button = new QPushButton;
        button->setText(QString("%1 %2").arg(emoji, text));
        button->setFixedHeight(40);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(buttonStyle);
        return button;
    };
    clockWidget = new ClockWidget();

    homeButton = createNavButton(tr("Home"), "🏠");
    workoutButton = createNavButton(tr("Padel Courts"), "🏸");
    nutritionButton = createNavButton(tr("Gym Classes"), "🏋️‍♂️");
    profileButton = createNavButton(tr("Profile"), "👤");
    settingsButton = createNavButton(tr("Settings"), "⚙️");

    homeButton->setCheckable(true);
    workoutButton->setCheckable(true);
    nutritionButton->setCheckable(true);
    profileButton->setCheckable(true);
    settingsButton->setCheckable(true);

    navButtonsLayout->addWidget(homeButton);
    navButtonsLayout->addWidget(workoutButton);
    navButtonsLayout->addWidget(nutritionButton);
    navButtonsLayout->addWidget(profileButton);
    navButtonsLayout->addWidget(settingsButton);
    navButtonsLayout->addStretch();
    navButtonsLayout->addWidget(clockWidget);

    scrollArea->setWidget(navButtonsContainer);

    // Theme and language container setup
    const auto controlsContainer = new QWidget;
    controlsContainer->setFixedWidth(340);
    controlsContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    controlsContainer->setStyleSheet("background: transparent;");
    const auto controlsLayout = new QHBoxLayout(controlsContainer);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->setAlignment(Qt::AlignRight);
    controlsLayout->setSpacing(16);

    // Create language selector
    languageSelector = new LanguageSelector;
    languageSelector->setFixedSize(140, 40);
    languageSelector->updateTheme(isDarkTheme);

    // Create theme toggle
    const auto themeToggle = new TopPanel;
    themeToggle->setFixedSize(160, 40);
    themeToggle->setStyleSheet(
        "TopPanel {"
        "   background: transparent;"
        "   border-radius: 20px;"
        "   padding: 4px;"
        "}"
    );

    themeToggle->setInitialState(isDarkTheme);
    connect(themeToggle, &TopPanel::themeToggled, this, &MainPage::toggleTheme);

    // Add controls to the container
    controlsLayout->addWidget(languageSelector);
    controlsLayout->addWidget(themeToggle);

    // Add all components to nav layout
    navLayout->addWidget(logoContainer);
    navLayout->addWidget(scrollArea);
    navLayout->addWidget(controlsContainer);

    // Add nav bar and stacked widget to main layout
    mainVLayout->addWidget(navBar);

    stackedWidget = new QStackedWidget;
    stackedWidget->setStyleSheet(stackedWidgetStyle);
    setupPages();
    mainVLayout->addWidget(stackedWidget);

    // Install event filter for scroll area
    scrollArea->viewport()->installEventFilter(this);
    scrollArea->viewport()->setMouseTracking(true);

    connect(homeButton, &QPushButton::clicked, this, &MainPage::handleHomePage);
    connect(workoutButton, &QPushButton::clicked, this, &MainPage::handleWorkoutPage);
    connect(nutritionButton, &QPushButton::clicked, this, &MainPage::handleNutritionPage);
    connect(profileButton, &QPushButton::clicked, this, &MainPage::handleProfilePage);
    connect(settingsButton, &QPushButton::clicked, this, &MainPage::handleSettingsPage);

    // Initialize with home page
    handleHomePage();

    // Initialize with English text
    retranslateUI();
}

void MainPage::updateNavBarStyle()
{
    const QSize size = this->size();
    QString blurIntensity = size.width() < 800 ? "8px" : "12px";
    QString shadowOpacity = size.width() < 800 ? "0.03" : "0.05";

    QString navBarStyle = navBarStyler.arg(
        isDarkTheme ? "rgba(31, 41, 55, 0.7)" : "rgba(255, 255, 255, 0.7)",
        blurIntensity,
        shadowOpacity
    );

    if (const auto navBar = findChild<QWidget*>("navBar")) {
        navBar->setStyleSheet(navBarStyle);
    }
}

bool MainPage::eventFilter(QObject* obj, QEvent* event)
{
    static QPoint pressPos;
    static bool isDragging = false;

    if (scrollArea && obj == scrollArea->viewport()) {
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    pressPos = mouseEvent->pos();
                    isDragging = true;
                }
                break;
            }
            case QEvent::MouseMove: {
                if (isDragging) {
                    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                    int delta = pressPos.x() - mouseEvent->pos().x();
                    scrollArea->horizontalScrollBar()->setValue(
                        scrollArea->horizontalScrollBar()->value() + delta
                    );
                    pressPos = mouseEvent->pos();
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                isDragging = false;
                break;
            }
            default:
                break;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainPage::setupPages()
{
    try {
        qDebug() << "MainPage::setupPages called";

        if (!stackedWidget) {
            qDebug() << "Error: stackedWidget is null in setupPages";
            return;
        }

        homePage = new HomePage(this);
        if (!homePage) {
            qDebug() << "Error: Failed to create HomePage";
            return;
        }

        workoutPage = new BookingWindow(padelDataManager, this);
        if (!workoutPage) {
            qDebug() << "Error: Failed to create workoutPage";
            return;
        }

        nutritionPage = new AvailableClassesScreen(classDataManager);
        if (!nutritionPage) {
            qDebug() << "Error: Failed to create nutritionPage";
            return;
        }

        profilePage = new QWidget;
        if (!profilePage) {
            qDebug() << "Error: Failed to create profilePage";
            return;
        }

        settingsPage = new SettingsPage(userDataManager, memberDataManager, this);
        if (!settingsPage) {
            qDebug() << "Error: Failed to create settingsPage";
            return;
        }

        qDebug() << "Connecting settingsPage signals";
        connect(settingsPage, &SettingsPage::logoutRequested, this, &MainPage::logoutRequested);
        connect(this, &MainPage::userDataLoaded, settingsPage, &SettingsPage::onUserDataLoaded);

        qDebug() << "Adding widgets to stackedWidget";
        stackedWidget->addWidget(homePage);
        stackedWidget->addWidget(workoutPage);
        stackedWidget->addWidget(nutritionPage);
        stackedWidget->addWidget(profilePage);
        stackedWidget->addWidget(settingsPage);
        qDebug() << "Pages setup completed successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in MainPage::setupPages: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in MainPage::setupPages";
    }
}

void MainPage::handleHomePage() const
{
    try {
        qDebug() << "MainPage::handleHomePage called";

        if (!stackedWidget) {
            qDebug() << "Error: stackedWidget is null in handleHomePage";
            return;
        }

        if (!homePage) {
            qDebug() << "Error: homePage is null in handleHomePage";
            return;
        }

        qDebug() << "Setting current widget to homePage";
        stackedWidget->setCurrentWidget(homePage);

        if (!homeButton) {
            qDebug() << "Error: homeButton is null in handleHomePage";
            return;
        }

        qDebug() << "Updating button states";
        updateButtonStates(homeButton);
        qDebug() << "MainPage::handleHomePage completed successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in MainPage::handleHomePage: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in MainPage::handleHomePage";
    }
}

void MainPage::handleWorkoutPage() const
{
    stackedWidget->setCurrentWidget(workoutPage);
    updateButtonStates(workoutButton);

    // Pass current user information to the booking window
    if (workoutPage && !currentUserEmail.isEmpty()) {
        workoutPage->setUserDataManager(userDataManager);
        workoutPage->setMemberDataManager(memberDataManager);
        workoutPage->setCurrentUserEmail(currentUserEmail);
    }
}

void MainPage::handleNutritionPage() const
{
    stackedWidget->setCurrentWidget(nutritionPage);
    updateButtonStates(nutritionButton);
    if (nutritionPage && !currentUserEmail.isEmpty()) {
        nutritionPage->setUserDataManager(userDataManager);
        nutritionPage->setMemberDataManager(memberDataManager);
        nutritionPage->setCurrentUserEmail(currentUserEmail);
    }
}

void MainPage::handleProfilePage() const
{
    stackedWidget->setCurrentWidget(profilePage);
    updateButtonStates(profileButton);
}

void MainPage::handleSettingsPage() const
{
    stackedWidget->setCurrentWidget(settingsPage);
    updateButtonStates(settingsButton);
}

void MainPage::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    ThemeManager::getInstance().setDarkTheme(isDarkTheme);
}

void MainPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    updateNavBarStyle();
    updateAllTextColors();

    // Update language selector theme
    if (languageSelector) {
        languageSelector->updateTheme(isDark);
    }

    homePage->updateTheme(isDark);
    workoutPage->updateTheme(isDark);
    nutritionPage->updateTheme(isDark);
    settingsPage->updateTheme(isDark);
}

void MainPage::updateAllTextColors()
{
    titleLabel->setStyleSheet(titleLabelStyle.arg(isDarkTheme ? "#FFFFFF" : "#111827"));
}

void MainPage::updateButtonStates(QPushButton* activeButton) const
{
    try {
        qDebug() << "MainPage::updateButtonStates called";

        if (!homeButton || !workoutButton || !nutritionButton || !profileButton || !settingsButton) {
            qDebug() << "Error: One or more navigation buttons are null";
            return;
        }

        if (!activeButton) {
            qDebug() << "Error: activeButton is null";
            return;
        }

        homeButton->setChecked(false);
        workoutButton->setChecked(false);
        nutritionButton->setChecked(false);
        profileButton->setChecked(false);
        settingsButton->setChecked(false);

        activeButton->setChecked(true);
        qDebug() << "Button states updated successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in MainPage::updateButtonStates: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in MainPage::updateButtonStates";
    }
}

void MainPage::handleLogin(const QString& email)
{
    try {
        qDebug() << "MainPage::handleLogin called with email: " << email;

        // First, ensure we have cleared any previous data
        if (!currentUserEmail.isEmpty() && currentUserEmail != email) {
            qDebug() << "Switching user from " << currentUserEmail << " to " << email;
            // Clear previous user data
            clearUserData();
        }

        // Set new current user email
        currentUserEmail = email;

        qDebug() << "About to emit userDataLoaded signal";
        emit userDataLoaded(email);
        if (settingsPage) {
            settingsPage->loadUserData(email);
            if (auto subscriptionStatusPage = settingsPage->findChild<SubscriptionStatusPage*>()) {
                qDebug() << "Forcing refresh of SubscriptionStatusPage data";
                subscriptionStatusPage->loadMemberData();
            }

            if (auto subscriptionPage = settingsPage->findChild<SubscriptionPage*>()) {
                qDebug() << "Forcing refresh of SubscriptionPage data";
                // If there's a method to force reload, call it here
            }
        }

        qDebug() << "Checking if homePage is initialized";
        if (!homePage) {
            qDebug() << "Error: homePage is null in MainPage::handleLogin";
            return;
        }

        qDebug() << "Calling handleHomePage()";
        handleHomePage();
        qDebug() << "MainPage::handleLogin completed successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in MainPage::handleLogin: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in MainPage::handleLogin";
    }
}

void MainPage::clearUserData()
{
    qDebug() << "Clearing all user data from application state...";

    // Clear current user email
    currentUserEmail.clear();

    // Reset Settings Page completely
    if (settingsPage) {
        qDebug() << "Clearing SettingsPage data";

        // Reset user data in settings page
        settingsPage->loadUserData("");

        // Reset subscription data in all subscription-related components
        if (auto subscriptionStatusPage = settingsPage->findChild<SubscriptionStatusPage*>()) {
            subscriptionStatusPage->setCurrentMemberId(0);
            qDebug() << "Reset SubscriptionStatusPage member ID";
        }

        if (auto subscriptionPage = settingsPage->findChild<SubscriptionPage*>()) {
            subscriptionPage->setCurrentMemberId(0);
            qDebug() << "Reset SubscriptionPage member ID";
        }

        if (auto paymentPage = settingsPage->findChild<PaymentPage*>()) {
            qDebug() << "Reset PaymentPage data if present";
        }
    }

    // Clear HomePage data
    if (homePage) {
        qDebug() << "Cleared HomePage data";
    }

    if (workoutPage) {
        qDebug() << "Cleared WorkoutPage data";
    }

    if (nutritionPage) {
        qDebug() << "Cleared NutritionPage data";
    }

    if (profilePage) {
        qDebug() << "Cleared ProfilePage data";
    }

    // Clear Padel-related data
    if (padelDataManager) {
        qDebug() << "Clearing Padel-related data";
        // Any specific padel data clearing can be added here
    }

    if (stackedWidget && homePage) {
        stackedWidget->setCurrentWidget(homePage);
        qDebug() << "Reset to HomePage view";

        if (homeButton) {
            updateButtonStates(homeButton);
            qDebug() << "Reset navigation to Home button";
        }
    }

    // Add a small delay to ensure everything is reset before loading new data
    QTimer::singleShot(200, [this]() {
        qDebug() << "Delayed reset completed";
    });

    qDebug() << "All user data cleared successfully";
}

void MainPage::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateLayout();
    updateNavBarStyle();
}

void MainPage::updateLayout()
{
    const QSize size = this->size();

    // Adjust navigation buttons layout based on window width
    if (size.width() < 800) {
        homeButton->setStyleSheet(smallButtonStyle);
        workoutButton->setStyleSheet(smallButtonStyle);
        nutritionButton->setStyleSheet(smallButtonStyle);
        profileButton->setStyleSheet(smallButtonStyle);
        settingsButton->setStyleSheet(smallButtonStyle);

        // Hide button text if very small, keep emoji
        if (size.width() < 600) {
            homeButton->setText("🏠");
            workoutButton->setText("🏸");
            nutritionButton->setText("🏋️‍♂️");
            profileButton->setText("👤");
            settingsButton->setText("⚙️");
        } else {
            homeButton->setText(QString("🏠 %1").arg(tr("Home")));
            workoutButton->setText(QString("🏸 %1").arg(tr("Padel Courts")));
            nutritionButton->setText(QString("️‍️‍🏋️‍♂️ %1").arg(tr("Gym Classes")));
            profileButton->setText(QString("👤 %1").arg(tr("Profile")));
            settingsButton->setText(QString("⚙️ %1").arg(tr("Settings")));
        }
    } else {
        // For larger screens, use normal styling
        homeButton->setStyleSheet(buttonStyle);
        workoutButton->setStyleSheet(buttonStyle);
        nutritionButton->setStyleSheet(buttonStyle);
        profileButton->setStyleSheet(buttonStyle);
        settingsButton->setStyleSheet(buttonStyle);

        homeButton->setText(QString("🏠 %1").arg(tr("Home")));
        workoutButton->setText(QString("🏸 %1").arg(tr("Padel Courts")));
        nutritionButton->setText(QString("️‍️‍🏋️‍♂️ %1").arg(tr("Gym Classes")));
        profileButton->setText(QString("👤 %1").arg(tr("Profile")));
        settingsButton->setText(QString("⚙️ %1").arg(tr("Settings")));
    }

    // Update pages layout
    if (homePage) homePage->updateLayout();
    if (settingsPage) settingsPage->updateLayout();
}

void MainPage::onLanguageChanged(const QString& language)
{
    retranslateUI();

    // Update all pages
    if (homePage) homePage->retranslateUI();
    if (settingsPage) settingsPage->retranslateUI();

    // Update window title
    window()->setWindowTitle(tr("FitFlex Pro"));
}

void MainPage::retranslateUI()
{
    // Update window title
    window()->setWindowTitle(tr("FitFlex Pro"));

    // Update navigation buttons
    if (homeButton) homeButton->setText(tr("Home"));
    if (workoutButton) workoutButton->setText(tr("Padel Courts"));
    if (nutritionButton) nutritionButton->setText(tr("Gym Classes"));
    if (profileButton) profileButton->setText(tr("Profile"));
    if (settingsButton) settingsButton->setText(tr("Settings"));

    // Update title
    if (titleLabel) {
        titleLabel->setText(QString("FitFlex<span style='color: #7E69AB;'>Pro</span>"));
    }

    // Update pages
    if (homePage) homePage->retranslateUI();
    if (settingsPage) settingsPage->retranslateUI();

    // Force layout update
    updateLayout();
}