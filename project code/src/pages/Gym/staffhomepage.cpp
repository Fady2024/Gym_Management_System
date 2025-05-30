﻿#include "staffhomepage.h"
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
#include "Stylesheets/Gym/staffHomePageStyle.h"
#include "../staff/retrievepage.h"
#include "../staff/searchmemberpage.h"
#include <QDebug>

StaffHomePage::StaffHomePage(UserDataManager* userDataManager, MemberDataManager* memberDataManager,
    ClassDataManager* classDataManager, PadelDataManager* padelDataManager, QWidget* parent)
    : QMainWindow(parent)
    , userDataManager(userDataManager)
    , memberDataManager(memberDataManager)
    , classDataManager(classDataManager)
    , padelDataManager(padelDataManager)
{
    try {
        qDebug() << "StaffHomePage constructor started";

        if (!userDataManager) {
            qDebug() << "Warning: userDataManager is null in StaffHomePage constructor";
        }

        if (!memberDataManager) {
            qDebug() << "Warning: memberDataManager is null in StaffHomePage constructor";
        }

        if (!classDataManager) {
            qDebug() << "Warning: classDataManager is null in StaffHomePage constructor";
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
            this, &StaffHomePage::onLanguageChanged);

        qDebug() << "StaffHomePage constructor completed successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in StaffHomePage constructor: " << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in StaffHomePage constructor";
    }
}

StaffHomePage::~StaffHomePage()
{
    // Clean up any resources if needed
}

void StaffHomePage::setupUI()
{
    // Set minimum size to prevent UI elements from being cut off
    setMinimumSize(800, 600);

    const auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Set transparent background for main window and central widget
    setStyleSheet(mainWindowStyle);
    centralWidget->setStyleSheet(centralWidgetStyle);

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
    logoContainer->setStyleSheet(logoContainerStyle);
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
    scrollArea->setStyleSheet(scrollAreaStyle);
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
        button->setStyleSheet(normalButtonStyle);
        return button;
        };
    clockWidget = new ClockWidget();

    homeButton = createNavButton(tr("Home"), "🏠");
    analyticsButton = createNavButton(tr("Analytics"), "📈");
    retrieveButton = createNavButton(tr("Manage Members"), "👤");
    settingsButton = createNavButton(tr("Settings"), "⚙️");

    homeButton->setCheckable(true);
    analyticsButton->setCheckable(true);

    retrieveButton->setCheckable(true);
    settingsButton->setCheckable(true);

    navButtonsLayout->addWidget(homeButton);
    navButtonsLayout->addWidget(analyticsButton);
    navButtonsLayout->addWidget(retrieveButton);
    navButtonsLayout->addWidget(settingsButton);
    navButtonsLayout->addSpacing(100);
    navButtonsLayout->addStretch(1);
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
    themeToggle->setStyleSheet(themeToggleStyle);

    themeToggle->setInitialState(isDarkTheme);
    connect(themeToggle, &TopPanel::themeToggled, this, &StaffHomePage::toggleTheme);

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

    connect(homeButton, &QPushButton::clicked, this, &StaffHomePage::handleHomePage);
    connect(analyticsButton, &QPushButton::clicked, this, &StaffHomePage::handleAnalyticsPage);
    connect(retrieveButton, &QPushButton::clicked, this, &StaffHomePage::handleRetrievePage);
    connect(settingsButton, &QPushButton::clicked, this, &StaffHomePage::handleSettingsPage);

    // Initialize with home page
    handleHomePage();

    // Initialize with English text
    retranslateUI();
}

void StaffHomePage::updateNavBarStyle()
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

bool StaffHomePage::eventFilter(QObject* obj, QEvent* event)
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

void StaffHomePage::setupPages()
{
    try {
        qDebug() << "StaffHomePage::setupPages called";

        if (!stackedWidget) {
            qDebug() << "Error: stackedWidget is null in setupPages";
            return;
        }

        homePage = new HomePage(this);
        if (!homePage) {
            qDebug() << "Error: Failed to create HomePage";
            return;
        }

        analyticsPage = new Revenue(userDataManager, memberDataManager,classDataManager, this);
        if (!analyticsPage) {
            qDebug() << "Error: Failed to create analyticsPage";
            return;
        }

        retrievePage = new RetrievePage(userDataManager, memberDataManager, this);
        if (!retrievePage) {
            qDebug() << "Error: Failed to create RetrivePage";
            return;
        }

        settingsPage = new SettingsPage(userDataManager, memberDataManager, this);
        if (!settingsPage) {
            qDebug() << "Error: Failed to create settingsPage";
            return;
        }

        qDebug() << "Connecting settingsPage signals";
        connect(settingsPage, &SettingsPage::logoutRequested, this, &StaffHomePage::logoutRequested);
        connect(this, &StaffHomePage::userDataLoaded, settingsPage, &SettingsPage::onUserDataLoaded);

        qDebug() << "Adding widgets to stackedWidget";
        stackedWidget->addWidget(homePage);
        stackedWidget->addWidget(analyticsPage);
        stackedWidget->addWidget(retrievePage);
        stackedWidget->addWidget(settingsPage);
        qDebug() << "Pages setup completed successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in StaffHomePage::setupPages: " << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in StaffHomePage::setupPages";
    }
}

void StaffHomePage::handleHomePage() const
{
    try {
        qDebug() << "StaffHomePage::handleHomePage called";

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
        qDebug() << "StaffHomePage::handleHomePage completed successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in StaffHomePage::handleHomePage: " << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in StaffHomePage::handleHomePage";
    }
}

void StaffHomePage::handleAnalyticsPage() const
{
    stackedWidget->setCurrentWidget(analyticsPage);
    updateButtonStates(analyticsButton);
}

void StaffHomePage::handleRetrievePage() const
{
    stackedWidget->setCurrentWidget(retrievePage);
    updateButtonStates(retrieveButton);
}

void StaffHomePage::handleSettingsPage() const
{
    stackedWidget->setCurrentWidget(settingsPage);
    updateButtonStates(settingsButton);
}

void StaffHomePage::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    ThemeManager::getInstance().setDarkTheme(isDarkTheme);
}

void StaffHomePage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    updateNavBarStyle();
    updateAllTextColors();

    // Update language selector theme
    if (languageSelector) {
        languageSelector->updateTheme(isDark);
    }

    homePage->updateTheme(isDark);
    settingsPage->updateTheme(isDark);
}

void StaffHomePage::updateAllTextColors()
{
    titleLabel->setStyleSheet(titleLabelStyle.arg(isDarkTheme ? "#FFFFFF" : "#111827"));
}

void StaffHomePage::updateButtonStates(QPushButton* activeButton) const
{
    try {
        qDebug() << "StaffHomePage::updateButtonStates called";

        if (!homeButton || !analyticsButton || !retrieveButton || !settingsButton) {
            qDebug() << "Error: One or more navigation buttons are null";
            return;
        }

        if (!activeButton) {
            qDebug() << "Error: activeButton is null";
            return;
        }

        homeButton->setChecked(false);
        analyticsButton->setChecked(false);
        retrieveButton->setChecked(false);
        settingsButton->setChecked(false);

        activeButton->setChecked(true);
        qDebug() << "Button states updated successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in StaffHomePage::updateButtonStates: " << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in StaffHomePage::updateButtonStates";
    }
}

void StaffHomePage::handleLogin(const QString& email)
{
    try {
        qDebug() << "StaffHomePage::handleLogin called with email: " << email;
        currentUserEmail = email;

        qDebug() << "About to emit userDataLoaded signal";
        emit userDataLoaded(email);

        qDebug() << "Checking if homePage is initialized";
        if (!homePage) {
            qDebug() << "Error: homePage is null in StaffHomePage::handleLogin";
            return;
        }

        qDebug() << "Calling handleHomePage()";
        handleHomePage();
        qDebug() << "StaffHomePage::handleLogin completed successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in StaffHomePage::handleLogin: " << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in StaffHomePage::handleLogin";
    }
}

void StaffHomePage::clearUserData()
{
    currentUserEmail.clear();

    if (settingsPage) {
        settingsPage->loadUserData("");
    }

    if (homePage) {
        // Clear home page data
    }
    if (analyticsPage) {
        // Clear addMember page data
    }
    if (retrievePage) {
        // Clear retrieve page data
    }
    if (stackedWidget && homePage) {
        stackedWidget->setCurrentWidget(homePage);
        if (homeButton) {
            updateButtonStates(homeButton);
        }
    }
}
void StaffHomePage::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateLayout();
    updateNavBarStyle();
}

void StaffHomePage::updateLayout()
{
    const QSize size = this->size();

    // Adjust navigation buttons layout based on window width
    if (size.width() < 800) {
        // For small screens, reduce padding and font size
        const QString buttonStyle = smallButtonStyle;

        homeButton->setStyleSheet(buttonStyle);
        analyticsButton->setStyleSheet(buttonStyle);
        retrieveButton->setStyleSheet(buttonStyle);
        settingsButton->setStyleSheet(buttonStyle);

        // Hide button text if very small, keep emoji
        if (size.width() < 600) {
            homeButton->setText("🏠");
            analyticsButton->setText("📈");
            retrieveButton->setText("👤");
            settingsButton->setText("⚙️");
        }
        else {
            homeButton->setText(QString("🏠 %1").arg(tr("Home")));
            analyticsButton->setText(QString("📈 %1").arg(tr("Analytics")));
            retrieveButton->setText(QString("👤 %1").arg(tr("Manage Members")));
            settingsButton->setText(QString("⚙️ %1").arg(tr("Settings")));
        }
    }
    else {
        // For larger screens, use normal styling
        const QString buttonStyle = normalButtonStyle;

        homeButton->setStyleSheet(buttonStyle);
        analyticsButton->setStyleSheet(buttonStyle);
        retrieveButton->setStyleSheet(buttonStyle);
        settingsButton->setStyleSheet(buttonStyle);

        homeButton->setText(QString("🏠 %1").arg(tr("Home")));
        analyticsButton->setText(QString("📈 %1").arg(tr("Analytics")));
        retrieveButton->setText(QString("👤 %1").arg(tr("Manage Members")));
        settingsButton->setText(QString("⚙️ %1").arg(tr("Settings")));
    }

    // Update pages layout
    if (homePage) homePage->updateLayout();
    if (settingsPage) settingsPage->updateLayout();
}

void StaffHomePage::onLanguageChanged(const QString& language)
{
    retranslateUI();

    // Update all pages
    if (homePage) homePage->retranslateUI();
    if (settingsPage) settingsPage->retranslateUI();

    // Update window title
    window()->setWindowTitle(tr("FitFlex Pro"));
}

void StaffHomePage::retranslateUI()
{
    // Update window title
    window()->setWindowTitle(tr("FitFlex Pro"));

    // Update navigation buttons
    if (homeButton) homeButton->setText(tr("Home"));
    if (analyticsButton) analyticsButton->setText(tr("Analytics"));
    if (retrieveButton) retrieveButton->setText(tr("Manage Members"));
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