#include "authpage.h"
#include "mainpage.h"
#include "../Theme/ThemeManager.h"
#include "../Language/LanguageManager.h"

#include <QScrollArea>
#include <QTextEdit>
#include <qcoreevent.h>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSettings>
#include "UIUtils.h"
#include <QSizePolicy>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QParallelAnimationGroup>
#include <QDebug>
#include <QSequentialAnimationGroup>
#include <QImageReader>
#include <QImage>

#include "TopPanel.h"

AuthPage::AuthPage(UserDataManager* userDataManager, QWidget* parent)
    : QMainWindow(parent)
    , userDataManager(userDataManager)
    , isDarkTheme(ThemeManager::getInstance().isDarkTheme())
    , opacityEffect(new QGraphicsOpacityEffect(this))
{
    setMinimumSize(800, 600);
    
    setupUI();
    setupGlassEffect();
    setupMessageWidget();
    
    updateTheme(isDarkTheme);
    
    // Connect to ThemeManager
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
            this, [this](bool isDark) {
                isDarkTheme = isDark;
                updateTheme(isDark);
            });

    // Connect to LanguageManager
    connect(&LanguageManager::getInstance(), &LanguageManager::languageChanged,
            this, &AuthPage::retranslateUI);

    QString rememberedEmail, rememberedPassword;
    if (userDataManager->getRememberedCredentials(rememberedEmail, rememberedPassword)) {
        loginEmailInput->setText(rememberedEmail);
        loginPasswordInput->setText(rememberedPassword);
        rememberMeCheckbox->setChecked(true);
    }
}

void AuthPage::setupUI()
{
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    const auto mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(0, 0, 0, 0);

    const auto navBar = new QWidget;
    navBar->setObjectName("navBar");
    navBar->setFixedHeight(56);
    navBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    navBar->setStyleSheet(
        "QWidget#navBar {"
        "   background: rgba(255, 255, 255, 0.1);"
        "   backdrop-filter: blur(12px);"
        "   border-bottom: 1px solid rgba(255, 255, 255, 0.1);"
        "   box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);"
        "}"
    );

    const auto navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(24, 0, 24, 0);
    navLayout->setSpacing(16);

    const auto logoLabel = new QLabel;
    const QPixmap logo("Images/dumbbell.png");
    logoLabel->setPixmap(logo.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    const auto titleLabel = new QLabel;
    titleLabel->setObjectName("titleLabel");
    titleLabel->setText(QString("FitFlex<span style='color: #7E69AB;'>Pro</span>"));
    titleLabel->setStyleSheet(QString("QLabel { font-size: 20px; font-weight: 600; color: %1; }")
        .arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    titleLabel->setTextFormat(Qt::RichText);

    const auto logoLayout = new QHBoxLayout;
    logoLayout->setSpacing(8);
    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(titleLabel);

    const auto themeToggle = new TopPanel;
    themeToggle->setFixedSize(200, 40);
    themeToggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    themeToggle->setStyleSheet(
        "TopPanel {"
        "   background: transparent;"
        "   border-radius: 20px;"
        "   padding: 4px;"
        "}"
    );

    themeToggle->setInitialState(isDarkTheme);
    connect(themeToggle, &TopPanel::themeToggled, this, &AuthPage::toggleTheme);

    const auto themeContainer = new QWidget;
    const auto themeLayout = new QVBoxLayout(themeContainer);
    themeLayout->setContentsMargins(0, 0, 0, 0);
    themeLayout->setSpacing(0);
    themeLayout->addStretch(1);
    themeLayout->addWidget(themeToggle);
    themeLayout->addStretch(1);
    themeContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    navLayout->addLayout(logoLayout);
    navLayout->addStretch();
    navLayout->addWidget(themeContainer);

    const auto contentWidget = new QWidget;
    contentWidget->setObjectName("contentWidget");

    const auto contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // Left side - Image slider
    const auto sliderWidget = new QWidget;
    sliderWidget->setObjectName("sliderWidget");
    sliderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sliderWidget->setMinimumWidth(400);
    contentLayout->addWidget(sliderWidget, 3);

    // Right side - Auth forms
    const auto authWidget = new QWidget;
    authWidget->setObjectName("authWidget");
    authWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    authWidget->setMinimumWidth(350);

    const auto authWidgetLayout = new QVBoxLayout(authWidget);
    authWidgetLayout->setContentsMargins(24, 24, 24, 24);

    // Auth container with glass effect
    const auto authContainer = new QWidget;
    authContainer->setObjectName("authContainer");
    authContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    authContainer->setStyleSheet(UIUtils::getAuthContainerStyle(isDarkTheme));

    const auto containerLayout = new QVBoxLayout(authContainer);
    containerLayout->setSpacing(12);
    containerLayout->setContentsMargins(24, 24, 24, 24);
    containerLayout->setAlignment(Qt::AlignCenter);

    // Title
    const auto welcomeLabel = new QLabel(tr("Welcome to FitFlex<span style='color: #8B5CF6;'>Pro</span>"));
    welcomeLabel->setStyleSheet(UIUtils::getWelcomeLabelStyle(isDarkTheme));
    welcomeLabel->setTextFormat(Qt::RichText);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setObjectName("welcomeLabel");

    // Tab buttons
    const auto tabLayout = new QHBoxLayout;
    tabLayout->setAlignment(Qt::AlignCenter);

    auto loginTab = new QPushButton(tr("Login"));
    auto signupTab = new QPushButton(tr("Sign Up"));

    loginTab->setProperty("active", true);
    signupTab->setProperty("active", false);
    loginTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, true));
    signupTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, false));
    loginTab->setObjectName("loginTab");
    signupTab->setObjectName("signupTab");

    tabLayout->addWidget(loginTab);
    tabLayout->addWidget(signupTab);

    // Stacked widget for login/signup forms
    stackedWidget = new QStackedWidget;

    // Add widgets to container layout
    containerLayout->addLayout(tabLayout);
    containerLayout->addWidget(welcomeLabel);
    containerLayout->addWidget(stackedWidget);

    // Add container to auth widget layout
    authWidgetLayout->addWidget(authContainer);

    // Add auth widget to content layout
    contentLayout->addWidget(authWidget, 2);

    // Add main widgets to main layout
    mainVLayout->addWidget(navBar);
    mainVLayout->addWidget(contentWidget);

    // Set minimum window size
    setMinimumSize(800, 600);

    // Setup forms AFTER creating stackedWidget
    setupLoginForm();
    setupSignupForm();

    // Connect tab buttons AFTER setting up forms
    connect(loginTab, &QPushButton::clicked, [this, loginTab, signupTab]() {
        animateTabSwitch(0, loginTab, signupTab);
    });
    connect(signupTab, &QPushButton::clicked, [this, loginTab, signupTab]() {
        animateTabSwitch(1, loginTab, signupTab);
    });

    // Setup image slider
    setupImageSlider();

    // Install event filters AFTER creating the input fields
    if (loginPasswordInput) loginPasswordInput->installEventFilter(this);
    if (signupPasswordInput) signupPasswordInput->installEventFilter(this);
}

void AuthPage::setupGlassEffect() const
{
    if (const auto authContainer = findChild<QWidget*>("authContainer")) {
        authContainer->setGraphicsEffect(opacityEffect);
        opacityEffect->setOpacity(0.95);
    }
}

void AuthPage::animateTabSwitch(int index, QPushButton* loginTab, QPushButton* signupTab)
{
    if (!stackedWidget || !loginTab || !signupTab) {
        return; // Safety check for null pointers
    }

    // Skip animation if we're already on the selected tab
    if (stackedWidget->currentIndex() == index) {
        loginTab->setProperty("active", index == 0);
        signupTab->setProperty("active", index == 1);
        loginTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, index == 0));
        signupTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, index == 1));
        return;
    }

    // Disable tab buttons during animation to prevent multiple clicks
    loginTab->setEnabled(false);
    signupTab->setEnabled(false);

    // Create animation group for parallel animations
    auto animationGroup = new QParallelAnimationGroup(this);

    // 1. Slide Animation for content
    auto slideAnimation = new QPropertyAnimation(stackedWidget, "pos");
    slideAnimation->setDuration(400);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Calculate slide distance and direction
    const int slideDistance = stackedWidget->width();
    const QPoint startPos = stackedWidget->pos();
    QPoint endPos = startPos;
    endPos.setX(startPos.x() + (index == 1 ? -slideDistance : slideDistance));

    slideAnimation->setStartValue(startPos);
    slideAnimation->setEndValue(endPos);

    // 2. Fade Animation for content
    auto fadeOut = new QPropertyAnimation(stackedWidget, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutQuad);

    auto fadeIn = new QPropertyAnimation(stackedWidget, "windowOpacity");
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InQuad);

    // 3. Scale Animation for active tab
    auto scaleAnimation = new QPropertyAnimation(index == 0 ? loginTab : signupTab, "geometry");
    scaleAnimation->setDuration(300);
    scaleAnimation->setEasingCurve(QEasingCurve::OutBack);

    QRect startRect = index == 0 ? loginTab->geometry() : signupTab->geometry();
    QRect endRect = startRect;
    endRect.setHeight(endRect.height() + 2); // Slight height increase
    endRect.moveTop(endRect.top() - 1); // Move up slightly

    scaleAnimation->setStartValue(startRect);
    scaleAnimation->setEndValue(endRect);

    // 4. Scale Animation for inactive tab
    auto inactiveScaleAnimation = new QPropertyAnimation(index == 0 ? signupTab : loginTab, "geometry");
    inactiveScaleAnimation->setDuration(300);
    inactiveScaleAnimation->setEasingCurve(QEasingCurve::OutBack);

    QRect inactiveStartRect = index == 0 ? signupTab->geometry() : loginTab->geometry();
    QRect inactiveEndRect = inactiveStartRect;
    inactiveEndRect.setHeight(inactiveEndRect.height() - 2); // Slight height decrease
    inactiveEndRect.moveTop(inactiveEndRect.top() + 1); // Move down slightly

    inactiveScaleAnimation->setStartValue(inactiveStartRect);
    inactiveScaleAnimation->setEndValue(inactiveEndRect);

    // Add animations to the group
    animationGroup->addAnimation(slideAnimation);
    animationGroup->addAnimation(fadeOut);
    animationGroup->addAnimation(scaleAnimation);
    animationGroup->addAnimation(inactiveScaleAnimation);

    // Connect animations for proper sequencing
    connect(animationGroup, &QParallelAnimationGroup::finished, [this, index, fadeIn, loginTab, signupTab]() {
        // Update the stacked widget index
        stackedWidget->setCurrentIndex(index);

        // Reset position
        stackedWidget->move(stackedWidget->pos().x(), 0);

        // Start fade in animation
        fadeIn->start();

        // Update tab styles with new theme
        loginTab->setProperty("active", index == 0);
        signupTab->setProperty("active", index == 1);
        loginTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, index == 0));
        signupTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, index == 1));

        // Re-enable tab buttons
        loginTab->setEnabled(true);
        signupTab->setEnabled(true);
    });

    // Connect fade in animation to clean up
    connect(fadeIn, &QPropertyAnimation::finished, [animationGroup, fadeIn, fadeOut]() {
        animationGroup->deleteLater();
        fadeIn->deleteLater();
        fadeOut->deleteLater();
    });

    // Start the animation sequence
    animationGroup->start();
}

void AuthPage::setupLoginForm()
{
    loginWidget = new QWidget;
    loginWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    const auto mainLayout = new QVBoxLayout(loginWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create a container widget for the form content with glass effect
    const auto formContainer = new QWidget;
    formContainer->setObjectName("loginFormContainer");
    formContainer->setStyleSheet(
        "QWidget#loginFormContainer {"
        "   background: rgba(255, 255, 255, 0.05);"
        "   border-radius: 16px;"
        "   backdrop-filter: blur(10px);"
        "}"
    );
    formContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    const auto layout = new QVBoxLayout(formContainer);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 24, 24, 24);

    // Create subtitle text
    const auto subtitleLabel = new QLabel;
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setText(tr("Welcome back! Please enter your details."));
    subtitleLabel->setStyleSheet(
        "QLabel {"
        "   color: #64748b;"
        "   font-size: 14px;"
        "   margin-bottom: 8px;"
        "}"
    );
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // Email input with icon
    const auto emailContainer = new QWidget;
    emailContainer->setFixedWidth(300);
    emailContainer->setStyleSheet(
        "QWidget {"
        "   background: transparent;"
        "   border-radius: 12px;"
        "   position: relative;"
        "}"
    );

    const auto emailLayout = new QHBoxLayout(emailContainer);
    emailLayout->setContentsMargins(0, 0, 0, 0);
    emailLayout->setSpacing(0);

    loginEmailInput = new CustomLineEdit;
    loginEmailInput->setPlaceholderText(tr("Email address"));
    loginEmailInput->setStyleSheet(
        "QLineEdit {"
        "   background-color: " + QString(isDarkTheme ? "#1a1b26" : "#ffffff") + ";"
        "   border: 2px solid " + QString(isDarkTheme ? "#2e2f3d" : "#e2e8f0") + ";"
        "   border-radius: 12px;"
        "   padding-left: 45px;"
        "   padding-right: 16px;"
        "   padding-top: 14px;"
        "   padding-bottom: 14px;"
        "   color: " + QString(isDarkTheme ? "#e2e8f0" : "#1e293b") + ";"
        "   font-size: 14px;"
        "   min-width: 200px;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #8B5CF6;"
        "}"
        "QLineEdit:hover {"
        "   border: 2px solid #7c3aed;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #64748b;"
        "}"
    );
    loginEmailInput->setFixedHeight(45);

    const auto emailIcon = new QLabel(loginEmailInput);
    emailIcon->setPixmap(UIUtils::getIconWithColor("mail.png", QColor(0x8B5CF6), 18));
    emailIcon->setStyleSheet(
        "QLabel {"
        "   background: transparent;"
        "   padding: 0;"
        "   margin: 0;"
        "}"
    );
    emailIcon->setGeometry(16, 14, 18, 18);

    emailLayout->addWidget(loginEmailInput);

    // Password input with icon
    loginPasswordInput = new CustomLineEdit;
    loginPasswordInput->setPlaceholderText(tr("Password"));
    loginPasswordInput->setEchoMode(QLineEdit::Password);
    loginPasswordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));

    // Lock icon (left side)
    const auto lockIcon = new QLabel(loginPasswordInput);
    lockIcon->setPixmap(UIUtils::getIconWithColor("lock.png", QColor(0x8B5CF6), 18));
    lockIcon->setStyleSheet(
        "QLabel {"
        "   background: transparent;"
        "   padding: 0;"
        "   margin: 0;"
        "}"
    );
    lockIcon->setGeometry(16, 14, 18, 18);

    // Toggle password visibility icon (right side)
    const auto togglePasswordIcon = new QLabel(loginPasswordInput);
    togglePasswordIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
    togglePasswordIcon->setStyleSheet(
        "QLabel {"
        "   background: transparent;"
        "   padding: 0;"
        "   margin: 0;"
        "   cursor: pointer;"
        "}"
    );
    togglePasswordIcon->setGeometry(loginPasswordInput->width() - 34, 14, 18, 18);
    togglePasswordIcon->setCursor(Qt::PointingHandCursor);
    togglePasswordIcon->installEventFilter(this);

    connect(togglePasswordIcon, &QLabel::linkActivated, [this, togglePasswordIcon](const QString&) {
        if (loginPasswordInput->echoMode() == QLineEdit::Password) {
            loginPasswordInput->setEchoMode(QLineEdit::Normal);
            togglePasswordIcon->setPixmap(UIUtils::getIconWithColor("open eyes.png", QColor(0x8B5CF6), 18));
        } else {
            loginPasswordInput->setEchoMode(QLineEdit::Password);
            togglePasswordIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
        }
    });

    // Options container with divider
    const auto optionsContainer = new QWidget;
    optionsContainer->setFixedWidth(300);
    const auto optionsLayout = new QHBoxLayout(optionsContainer);
    optionsLayout->setContentsMargins(0, 0, 0, 0);

    // Custom checkbox style
    rememberMeCheckbox = new QCheckBox(tr("Remember me"));
    rememberMeCheckbox->setStyleSheet(
        "QCheckBox {"
        "   color: #64748b;"
        "   font-size: 13px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 18px;"
        "   height: 18px;"
        "   border: 2px solid #8B5CF6;"
        "   border-radius: 4px;"
        "   background: transparent;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background: #8B5CF6;"
        "   image: url(Images/check.png);"
        "}"
    );

    forgotPasswordButton = new QPushButton(tr("Forgot Password?"));
    forgotPasswordButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   color: #8B5CF6;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   padding: 0;"
        "}"
        "QPushButton:hover {"
        "   color: #7C3AED;"
        "   text-decoration: underline;"
        "}"
    );
    forgotPasswordButton->setCursor(Qt::PointingHandCursor);

    optionsLayout->addWidget(rememberMeCheckbox);
    optionsLayout->addStretch();
    optionsLayout->addWidget(forgotPasswordButton);

    // Sign in button with hover effect
    loginButton = new QPushButton(tr("Sign In"));
    loginButton->setFixedWidth(300);
    loginButton->setFixedHeight(45);
    loginButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED);"
        "   border-radius: 12px;"
        "   color: white;"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "   padding: 12px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7C3AED, stop:1 #6D28D9);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6D28D9, stop:1 #5B21B6);"
        "}"
    );
    loginButton->setCursor(Qt::PointingHandCursor);
    connect(loginButton, &QPushButton::clicked, this, &AuthPage::handleLogin);

    // Or continue with divider
    const auto dividerContainer = new QWidget;
    const auto dividerLayout = new QHBoxLayout(dividerContainer);
    dividerLayout->setContentsMargins(0, 8, 0, 8);

    const auto leftLine = new QFrame;
    leftLine->setFrameShape(QFrame::HLine);
    leftLine->setStyleSheet("background-color: #374151;");

    const auto orLabel = new QLabel;
    orLabel->setObjectName("orLabel");
    orLabel->setText(tr("Or continue with"));
    orLabel->setStyleSheet("color: #64748b; font-size: 13px; margin: 0 12px;");

    const auto rightLine = new QFrame;
    rightLine->setFrameShape(QFrame::HLine);
    rightLine->setStyleSheet("background-color: #374151;");

    dividerLayout->addWidget(leftLine);
    dividerLayout->addWidget(orLabel);
    dividerLayout->addWidget(rightLine);

    // Social login buttons
    const auto socialLayout = new QHBoxLayout;
    socialLayout->setSpacing(12);
    socialLayout->setAlignment(Qt::AlignCenter);

    auto createSocialButton = [this](const QString& iconPath) {
        const auto button = new QPushButton;
        button->setFixedSize(45, 45);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(24, 24));
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(QString(
            "QPushButton {"
            "   background: %1;"
            "   border: 1px solid %2;"
            "   border-radius: 12px;"
            "   padding: 10px;"
            "   transition: all 0.3s ease;"
            "}"
            "QPushButton:hover {"
            "   background: %3;"
            "   border-color: %4;"
            "   transform: translateY(-2px);"
            "   box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);"
            "}"
            "QPushButton:pressed {"
            "   background: %5;"
            "   transform: translateY(1px);"
            "   box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);"
            "}"
        ).arg(
            isDarkTheme ? "#1e293b" : "#ffffff",
            isDarkTheme ? "#374151" : "#e2e8f0",
            isDarkTheme ? "#2d3748" : "#f8fafc",
            isDarkTheme ? "#4b5563" : "#cbd5e1",
            isDarkTheme ? "#1a202c" : "#f1f5f9"
        ));
        return button;
    };

    const auto googleButton = createSocialButton(":/Images/google.png");
    const auto appleButton = createSocialButton(":/Images/apple.png");
    const auto facebookButton = createSocialButton(":/Images/facebook.png");

    socialLayout->addWidget(googleButton);
    socialLayout->addWidget(appleButton);
    socialLayout->addWidget(facebookButton);

    // Add all widgets to form layout
    layout->addWidget(subtitleLabel);
    layout->addWidget(emailContainer);
    layout->addWidget(loginPasswordInput);
    layout->addWidget(optionsContainer);
    layout->addWidget(loginButton);
    layout->addWidget(dividerContainer);
    layout->addLayout(socialLayout);
    layout->setAlignment(Qt::AlignHCenter);

    // Center the form container in the main layout
    mainLayout->addStretch();
    mainLayout->addWidget(formContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    stackedWidget->addWidget(loginWidget);
}

void AuthPage::setupSignupForm()
{
    signupWidget = new QWidget;
    const auto layout = new QVBoxLayout(signupWidget);
    layout->setSpacing(12);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    const auto formContainer = new QWidget;
    formContainer->setFixedWidth(400);
    const auto formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(24, 16, 24, 16);
    formLayout->setAlignment(Qt::AlignHCenter);

    const auto profileContainer = new QWidget;
    const auto profileLayout = new QVBoxLayout(profileContainer);
    profileLayout->setSpacing(4);
    profileLayout->setAlignment(Qt::AlignCenter);
    profileLayout->setContentsMargins(0, 0, 0, 8);

    profileImageButton = new QPushButton;
    profileImageButton->setFixedSize(100, 100);
    profileImageButton->setIcon(UIUtils::getIcon("person.png", 50));
    profileImageButton->setIconSize(QSize(50, 50));
    profileImageButton->setStyleSheet(UIUtils::getProfileUploadStyle(isDarkTheme));
    profileImageButton->setCursor(Qt::PointingHandCursor);
    connect(profileImageButton, &QPushButton::clicked, this, &AuthPage::selectProfileImage);

    const auto uploadLabel = new QLabel(tr("Upload Photo"));
    uploadLabel->setStyleSheet(UIUtils::getProfileUploadLabelStyle(isDarkTheme));
    uploadLabel->setAlignment(Qt::AlignCenter);

    profileLayout->addWidget(profileImageButton, 0, Qt::AlignCenter);
    profileLayout->addWidget(uploadLabel, 0, Qt::AlignCenter);

    const auto inputsContainer = new QWidget;
    const auto inputsLayout = new QVBoxLayout(inputsContainer);
    inputsLayout->setSpacing(8);
    inputsLayout->setContentsMargins(0, 0, 0, 0);
    inputsLayout->setAlignment(Qt::AlignCenter);

    signupNameInput = new CustomLineEdit;
    signupNameInput->setPlaceholderText(tr("Your Name"));
    signupNameInput->setFixedHeight(40);
    signupNameInput->setFixedWidth(352);
    signupNameInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));

    const auto nameIcon = new QLabel(signupNameInput);
    nameIcon->setPixmap(UIUtils::getIconWithColor("person_bw.png", QColor(0x8B5CF6), 18));
    nameIcon->setStyleSheet("QLabel { background: transparent; padding: 0; margin: 0; }");
    nameIcon->setGeometry(16, 11, 18, 18);

    signupEmailInput = new CustomLineEdit;
    signupEmailInput->setPlaceholderText(tr("Email address"));
    signupEmailInput->setFixedHeight(40);
    signupEmailInput->setFixedWidth(352);
    signupEmailInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));

    const auto emailIcon = new QLabel(signupEmailInput);
    emailIcon->setPixmap(UIUtils::getIconWithColor("mail.png", QColor(0x8B5CF6), 18));
    emailIcon->setStyleSheet("QLabel { background: transparent; padding: 0; margin: 0; }");
    emailIcon->setGeometry(16, 11, 18, 18);

    signupAgeInput = new CustomLineEdit;
    signupAgeInput->setPlaceholderText(tr("Age"));
    signupAgeInput->setFixedHeight(40);
    signupAgeInput->setFixedWidth(352);
    signupAgeInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    signupAgeInput->setValidator(new QIntValidator(13, 120, signupAgeInput));

    const auto ageIcon = new QLabel(signupAgeInput);
    ageIcon->setPixmap(UIUtils::getIconWithColor("Age.png", QColor(0x8B5CF6), 18));
    ageIcon->setStyleSheet("QLabel { background: transparent; padding: 0; margin: 0; }");
    ageIcon->setGeometry(16, 11, 18, 18);

    signupPasswordInput = new CustomLineEdit;
    signupPasswordInput->setPlaceholderText(tr("Password"));
    signupPasswordInput->setEchoMode(QLineEdit::Password);
    signupPasswordInput->setFixedHeight(40);
    signupPasswordInput->setFixedWidth(352);
    signupPasswordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));

    const auto lockIcon = new QLabel(signupPasswordInput);
    lockIcon->setPixmap(UIUtils::getIconWithColor("lock.png", QColor(0x8B5CF6), 18));
    lockIcon->setStyleSheet("QLabel { background: transparent; padding: 0; margin: 0; }");
    lockIcon->setGeometry(16, 11, 18, 18);

    const auto togglePasswordIcon = new QLabel(signupPasswordInput);
    togglePasswordIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
    togglePasswordIcon->setStyleSheet("QLabel { background: transparent; padding: 0; margin: 0; cursor: pointer; }");
    togglePasswordIcon->setGeometry(signupPasswordInput->width() - 34, 11, 18, 18);
    togglePasswordIcon->setCursor(Qt::PointingHandCursor);
    togglePasswordIcon->installEventFilter(this);

    const auto termsContainer = new QWidget;
    termsContainer->setFixedWidth(352);
    const auto termsLayout = new QHBoxLayout(termsContainer);
    termsLayout->setContentsMargins(0, 0, 0, 0);
    termsLayout->setAlignment(Qt::AlignLeft);

    termsCheckbox = new QCheckBox;
    termsCheckbox->setText(tr("I agree to the"));
    termsCheckbox->setStyleSheet(UIUtils::getCheckboxStyle(isDarkTheme));

    const auto termsLink = new QLabel;
    termsLink->setText(QString("<a href='#' style='color: #8B5CF6; text-decoration: none; font-weight: bold;'>%1</a>").arg(tr("Terms of Service")));
    termsLink->setTextFormat(Qt::RichText);
    termsLink->setOpenExternalLinks(false);
    termsLink->setCursor(Qt::PointingHandCursor);
    connect(termsLink, &QLabel::linkActivated, this, &AuthPage::showTermsOfService);

    termsLayout->addWidget(termsCheckbox);
    termsLayout->addWidget(termsLink);
    termsLayout->addStretch();

    signupButton = new QPushButton(tr("Sign Up"));
    signupButton->setFixedWidth(352);
    signupButton->setFixedHeight(45);
    signupButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #8B5CF6, stop:0.5 #7C3AED, stop:1 #6D28D9);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 12px;"
        "   padding: 8px 16px;"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "   letter-spacing: 0.3px;"
        "   box-shadow: 0 4px 6px rgba(139, 92, 246, 0.2);"
        "   transition: all 0.3s ease;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #7C3AED, stop:0.5 #6D28D9, stop:1 #5B21B6);"
        "   box-shadow: 0 6px 10px rgba(139, 92, 246, 0.3);"
        "   transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #6D28D9, stop:0.5 #5B21B6, stop:1 #4C1D95);"
        "   box-shadow: 0 2px 4px rgba(139, 92, 246, 0.2);"
        "   transform: translateY(1px);"
        "}"
    );
    signupButton->setCursor(Qt::PointingHandCursor);
    connect(signupButton, &QPushButton::clicked, this, &AuthPage::handleSignup);

    inputsLayout->addWidget(signupNameInput, 0, Qt::AlignCenter);
    inputsLayout->addWidget(signupEmailInput, 0, Qt::AlignCenter);
    inputsLayout->addWidget(signupAgeInput, 0, Qt::AlignCenter);
    inputsLayout->addWidget(signupPasswordInput, 0, Qt::AlignCenter);
    inputsLayout->addWidget(termsContainer, 0, Qt::AlignCenter);
    inputsLayout->addWidget(signupButton, 0, Qt::AlignCenter);

    formLayout->addWidget(profileContainer);
    formLayout->addWidget(inputsContainer);

    layout->addStretch(1);
    layout->addWidget(formContainer, 0, Qt::AlignCenter);
    layout->addStretch(1);

    stackedWidget->addWidget(signupWidget);
}

void AuthPage::setupImageSlider()
{
    const auto slider = new ImageSlider(this);
    slider->updateTheme(isDarkTheme);

    // Find the content widget and its layout
    const auto contentWidget = findChild<QWidget*>("contentWidget");
    if (contentWidget && contentWidget->layout()) {
        // Find the slider widget container (first widget in the content layout)
        if (QWidget* sliderContainer = contentWidget->layout()->itemAt(0)->widget()) {
            const auto containerLayout = new QVBoxLayout(sliderContainer);
            containerLayout->setContentsMargins(0, 0, 0, 0);
            containerLayout->addWidget(slider);

            // Set minimum height for the slider container
            sliderContainer->setMinimumHeight(600);
        }
    }
}

void AuthPage::toggleForm()
{
    stackedWidget->setCurrentIndex(stackedWidget->currentIndex() == 0 ? 1 : 0);
    clearFields();
}

void AuthPage::togglePasswordVisibility() const
{
    QLineEdit* currentPasswordInput = stackedWidget->currentIndex() == 0 ? loginPasswordInput : signupPasswordInput;
    const auto toggleButton = currentPasswordInput->parent()->findChild<QPushButton*>();

    if (currentPasswordInput->echoMode() == QLineEdit::Password) {
        currentPasswordInput->setEchoMode(QLineEdit::Normal);
        if (toggleButton) {
            toggleButton->setIcon(UIUtils::getIcon("open eyes.png", 20));
        }
    } else {
        currentPasswordInput->setEchoMode(QLineEdit::Password);
        if (toggleButton) {
            toggleButton->setIcon(UIUtils::getIcon("close eyes.png", 20));
        }
    }
}

void AuthPage::handleLogin()
{
    if (!userDataManager) {
        qDebug() << "Fatal error: userDataManager is null in handleLogin";
        showError(tr("System error. Please try again later."));
        return;
    }

    try {
        const QString email = loginEmailInput ? loginEmailInput->text() : QString();
        const QString password = loginPasswordInput ? loginPasswordInput->text() : QString();
        
        if (email.isEmpty() || password.isEmpty()) {
            showError(tr("Please enter both email and password"));
            return;
        }

        // First check if email exists
        if (!userDataManager->emailExists(email)) {
            showError(tr("Email not found. Please check your email or sign up."));
            return;
        }

        // Then validate password
        if (!userDataManager->validateUser(email, password)) {
            showError(tr("Incorrect password. Please try again."));
            return;
        }

        // Handle successful login
        if (rememberMeCheckbox && rememberMeCheckbox->isChecked()) {
            userDataManager->saveRememberedCredentials(email, password);
        } else if (rememberMeCheckbox && !rememberMeCheckbox->isChecked()) {
            // Show confirmation dialog before clearing remembered credentials
            showConfirmationDialog(
                tr("Clear Remembered Login"),
                tr("Do you want to clear your remembered login information?"),
                tr("Yes"),
                tr("No"),
                [this]() {
                    userDataManager->clearRememberedCredentials();
                }
            );
        }
        
        animateMessageWidget(false); // false means success
        if (messageText) {
            messageText->setText(tr("Login successful! Welcome back!"));
        }
        
        // Create a timer to delay the navigation
        const auto delayTimer = new QTimer(this);
        delayTimer->setSingleShot(true);
        connect(delayTimer, &QTimer::timeout, this, [this, email]() {
            try {
                qDebug() << "Emitting loginSuccessful signal for: " << email;
                emit loginSuccessful(email);
            } catch (const std::exception& e) {
                qDebug() << "Exception when emitting login signal: " << e.what();
            } catch (...) {
                qDebug() << "Unknown exception when emitting login signal";
            }
        });
        delayTimer->start(1500);
    } catch (const std::exception& e) {
        qDebug() << "Exception in handleLogin: " << e.what();
        showError(tr("An error occurred during login. Please try again."));
    } catch (...) {
        qDebug() << "Unknown exception in handleLogin";
        showError(tr("An unexpected error occurred. Please try again."));
    }
}

void AuthPage::handleSignup()
{
    if (!userDataManager) {
        qDebug() << "Fatal error: userDataManager is null in handleSignup";
        showError(tr("System error. Please try again later."));
        return;
    }
    
    try {
        // Validate name first (top field)
        const QString name = signupNameInput ? signupNameInput->text() : QString();
        if (name.isEmpty()) {
            showError(tr("Please enter your name"));
            if (signupNameInput) signupNameInput->setFocus();
            return;
        }
        
        QString errorMessage;
        if (!userDataManager->validateName(name, errorMessage)) {
            showError(tr("Name: %1").arg(errorMessage));
            if (signupNameInput) signupNameInput->setFocus();
            return;
        }

        // Validate email (second field)
        const QString email = signupEmailInput ? signupEmailInput->text() : QString();
        if (email.isEmpty()) {
            showError(tr("Please enter your email"));
            if (signupEmailInput) signupEmailInput->setFocus();
            return;
        }
        
        if (!userDataManager->validateEmail(email, errorMessage)) {
            showError(tr("Email: %1").arg(errorMessage));
            if (signupEmailInput) signupEmailInput->setFocus();
            return;
        }
        
        // Check if email already exists
        if (userDataManager->emailExists(email)) {
            showError(tr("Email is already registered. Please use a different email or login."));
            if (signupEmailInput) signupEmailInput->setFocus();
            return;
        }

        // Validate age (third field)
        const QString ageStr = signupAgeInput ? signupAgeInput->text() : QString();
        if (ageStr.isEmpty()) {
            showError(tr("Please enter your age"));
            if (signupAgeInput) signupAgeInput->setFocus();
            return;
        }
        
        bool ok;
        int age = ageStr.toInt(&ok);
        if (!ok || age < 13 || age > 120) {
            showError(tr("Please enter a valid age between 13 and 120"));
            if (signupAgeInput) signupAgeInput->setFocus();
            return;
        }

        // Calculate date of birth from age
        QDate currentDate = QDate::currentDate();
        QDate dateOfBirth = currentDate.addYears(-age);

        // Validate password (fourth field)
        const QString password = signupPasswordInput ? signupPasswordInput->text() : QString();
        if (password.isEmpty()) {
            showError(tr("Please enter a password"));
            if (signupPasswordInput) signupPasswordInput->setFocus();
            return;
        }
        
        if (!userDataManager->validatePassword(password, errorMessage)) {
            showError(tr("Password: %1").arg(errorMessage));
            if (signupPasswordInput) signupPasswordInput->setFocus();
            return;
        }

        // Validate terms checkbox (bottom)
        if (!termsCheckbox || !termsCheckbox->isChecked()) {
            showError(tr("Please agree to the Terms of Service"));
            return;
        }

        // Create and validate new user
        const User newUser(name, email, password, selectedImagePath, dateOfBirth);
        if (!userDataManager->saveUserData(newUser, errorMessage)) {
            showError(tr("Failed to create account: %1").arg(errorMessage));
            return;
        }

        // Handle successful signup
        animateMessageWidget(false);
        if (messageText) {
            messageText->setText(tr("Account created successfully! Welcome to FitFlexPro!"));
        }
        
        // Create a timer to delay the navigation
        const auto delayTimer = new QTimer(this);
        delayTimer->setSingleShot(true);
        connect(delayTimer, &QTimer::timeout, this, [this, email]() {
            try {
                qDebug() << "Emitting loginSuccessful signal for new signup: " << email;
                emit loginSuccessful(email);
            } catch (const std::exception& e) {
                qDebug() << "Exception when emitting login signal after signup: " << e.what();
            } catch (...) {
                qDebug() << "Unknown exception when emitting login signal after signup";
            }
        });
        delayTimer->start(1500);
    } catch (const std::exception& e) {
        qDebug() << "Exception in handleSignup: " << e.what();
        showError(tr("An error occurred during signup. Please try again."));
    } catch (...) {
        qDebug() << "Unknown exception in handleSignup";
        showError(tr("An unexpected error occurred. Please try again."));
    }
}

void AuthPage::selectProfileImage()
{
    // Debug: Print supported formats
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Profile Image",
        "",
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff *.webp *.svg *.ico)"
    );

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.exists()) {
            showError(tr("File does not exist: %1").arg(filePath));
            return;
        }

        // Get file info and extension
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();
        QString baseName = fileInfo.baseName();

        // Try multiple loading methods with special handling for JPEG/JPG
        QImage image;
        bool loaded = false;

        // Method 1: Use QImageReader with explicit format for JPG/JPEG
        if (extension == "jpg" || extension == "jpeg") {
            QImageReader reader(filePath);
            reader.setFormat("jpeg");  // Explicitly set JPEG format
            reader.setAutoTransform(true);
            reader.setDecideFormatFromContent(true);
            image = reader.read();
            loaded = !image.isNull();
            
            if (!loaded) {
                qDebug() << "JPEG loading error:" << reader.errorString();
            }
        }

        // Method 2: Direct QImage loading if previous method failed
        if (!loaded) {
            image = QImage(filePath);
            loaded = !image.isNull();
        }

        // Method 3: QImageReader with format detection
        if (!loaded) {
            QImageReader reader(filePath);
            QByteArray format = QImageReader::imageFormat(filePath);
            if (!format.isEmpty()) {
                reader.setFormat(format);
            }
            reader.setAutoTransform(true);
            reader.setDecideFormatFromContent(true);
            image = reader.read();
            loaded = !image.isNull();
            
            if (!loaded) {
                qDebug() << "QImageReader error:" << reader.errorString();
            }
        }

        // Method 4: Try loading through QPixmap
        if (!loaded) {
            QPixmap pixmap(filePath);
            if (!pixmap.isNull()) {
                image = pixmap.toImage();
                loaded = !image.isNull();
            }
        }

        if (!loaded) {
            // Special error message for JPEG files
            if (extension == "jpg" || extension == "jpeg") {
                showError(tr("Failed to load JPEG image: %1\nPlease ensure the file is not corrupted and try again.").arg(filePath));
            } else {
                showError(tr("Failed to load image: %1\nSupported formats are PNG, JPEG, BMP, and GIF.").arg(filePath));
            }
            return;
        }

        // Convert QImage to QPixmap
        QPixmap uploadedPhoto = QPixmap::fromImage(image);
        
        if (uploadedPhoto.isNull()) {
            showError(tr("Failed to convert image: %1").arg(filePath));
            return;
        }

        // Create a square pixmap as the base
        constexpr int size = 140;
        QPixmap circularPhoto(size, size);
        circularPhoto.fill(Qt::transparent);

        // Create a painter for the circular mask
        QPainter painter(&circularPhoto);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // Create circular mask
        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        painter.setClipPath(path);

        // Scale and center the uploaded photo
        const QPixmap scaledPhoto = uploadedPhoto.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // Calculate centering position for the scaled photo
        const int x = (scaledPhoto.width() - size) / 2;
        const int y = (scaledPhoto.height() - size) / 2;

        // Draw the scaled and centered photo
        painter.drawPixmap(-x, -y, scaledPhoto);

        // Create final circular border
        painter.setClipping(false);
        painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
        painter.drawEllipse(1, 1, size-2, size-2);

        // Set the circular photo as the button icon
        profileImageButton->setIcon(QIcon(circularPhoto));
        profileImageButton->setIconSize(QSize(size, size));
        
        // Store the circular photo for reuse
        lastCircularPhoto = circularPhoto;

        // Get the project directory path
        QString projectDir;
        
#ifdef FORCE_SOURCE_DIR
        // Use the source directory path defined in CMake
        projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
        qDebug() << "Auth - Using source directory path:" << projectDir;
#else
        // Fallback to application directory
        projectDir = QCoreApplication::applicationDirPath();
        projectDir = QFileInfo(projectDir).dir().absolutePath(); // Go up to project root
        qDebug() << "Auth - Using application directory path:" << projectDir;
#endif
        
        QString usersPhotoDir = projectDir + "/project code/UsersPhoto";
        qDebug() << "Auth - Users photo directory path:" << usersPhotoDir;

        // Create UsersPhoto directory if it doesn't exist
        QDir dir(usersPhotoDir);
        if (!dir.exists() && !dir.mkpath(".")) {
            showError(tr("Failed to create directory: %1").arg(dir.absolutePath()));
            return;
        }

        QString newFileName = baseName + "." + extension;
        QString newAbsolutePath = dir.absoluteFilePath(newFileName);

        if (QFile::exists(newAbsolutePath)) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            newFileName = baseName + "_" + timestamp + "." + extension;
            newAbsolutePath = dir.absoluteFilePath(newFileName);
        }

        // Save both the original and circular versions
        if (!image.save(newAbsolutePath)) {
            showError(tr("Failed to save photo to: %1").arg(newAbsolutePath));
            return;
        }

        // Store the relative path for the user data
        selectedImagePath = "project code/UsersPhoto/" + newFileName;
        showSuccess(tr("Photo uploaded successfully!"));
    }
}

void AuthPage::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    ThemeManager::getInstance().setDarkTheme(isDarkTheme);
}

void AuthPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;

    // Update base window style
    const QString baseStyle = isDark ?
        "QMainWindow { background-color: #1A1F2C; }"
        "#contentWidget { background-color: #1A1F2C; }"
        "#sliderWidget { background-color: #1A1F2C; }"
        "#authWidget { background-color: #1A1F2C; color: #F9FAFB; }"
        "#authContainer {"
        "   background: rgba(30, 41, 59, 0.95);"
        "   border: 1px solid rgba(255, 255, 255, 0.1);"
        "   border-radius: 24px;"
        "   color: #F9FAFB;"
        "}"
        "#navBar {"
        "   background: rgba(255, 255, 255, 0.1);"
        "   backdrop-filter: blur(12px);"
        "   border-bottom: 1px solid rgba(255, 255, 255, 0.1);"
        "   box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);"
        "}"
        "QLabel { color: #F9FAFB; }" :
        "QMainWindow { background-color: #F0F4FA; }"
        "#contentWidget { background-color: #F0F4FA; }"
        "#sliderWidget { background-color: #FFFFFF; }"
        "#authWidget { background-color: #F0F4FA; color: #111827; }"
        "#authContainer {"
        "   background: rgba(225, 233, 245, 0.95);"
        "   border: 1px solid rgba(0, 0, 0, 0.1);"
        "   border-radius: 24px;"
        "   color: #111827;"
        "}"
        "#navBar {"
        "   background: rgba(225, 233, 245, 0.1);"
        "   backdrop-filter: blur(12px);"
        "   border-bottom: 1px solid rgba(0, 0, 0, 0.1);"
        "   box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);"
        "}"
        "QLabel { color: #111827; }";

    setStyleSheet(baseStyle);

    // Update all components with new theme
    updateAllTextColors();

    // Update message widget theme
    if (messageWidget) {
        messageWidget->setStyleSheet(UIUtils::getMessageWidgetStyle(isDarkTheme, false));
    }
    if (messageIcon) {
        messageIcon->setStyleSheet(UIUtils::getMessageIconStyle(isDarkTheme));
    }

    // Update image slider theme
    if (const auto contentWidget = findChild<QWidget*>("contentWidget")) {
        if (contentWidget->layout() && contentWidget->layout()->itemAt(0)) {
            if (const QWidget* sliderContainer = contentWidget->layout()->itemAt(0)->widget()) {
                if (const auto slider = sliderContainer->findChild<ImageSlider*>()) {
                    slider->updateTheme(isDarkTheme);
                }
            }
        }
    }

    // Update profile image button
    if (profileImageButton) {
        const int size = profileImageButton->width();
        if (!selectedImagePath.isEmpty() && !lastCircularPhoto.isNull()) {
            QPixmap circularPhoto(size, size);
            circularPhoto.fill(Qt::transparent);

            QPainter painter(&circularPhoto);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            QPainterPath path;
            path.addEllipse(0, 0, size, size);
            painter.setClipPath(path);

            painter.drawPixmap(0, 0, lastCircularPhoto.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            painter.setClipping(false);
            painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
            painter.drawEllipse(1, 1, size-2, size-2);

            profileImageButton->setIcon(QIcon(circularPhoto));
            profileImageButton->setIconSize(QSize(size, size));
        } else {
            QPixmap defaultPhoto(size, size);
            defaultPhoto.fill(Qt::transparent);

            QPainter painter(&defaultPhoto);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addEllipse(0, 0, size, size);
            painter.setClipPath(path);

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(isDarkTheme ? "#1e293b" : "#f1f5f9"));
            painter.drawEllipse(0, 0, size, size);

            painter.setClipping(false);
            painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
            painter.drawEllipse(1, 1, size-2, size-2);

            const QPixmap defaultIcon = UIUtils::getIcon("person.png", size/2);
            if (!defaultIcon.isNull()) {
                painter.drawPixmap(size/4, size/4, defaultIcon);
            }

            profileImageButton->setIcon(QIcon(defaultPhoto));
            profileImageButton->setIconSize(QSize(size, size));
        }

        profileImageButton->setStyleSheet(
            "QPushButton {"
            "   background-color: transparent;"
            "   border: none;"
            "   border-radius: " + QString::number(size/2) + "px;"
            "}"
            "QPushButton:hover {"
            "   background-color: transparent;"
            "}"
        );
    }

    // Update all input fields
    if (loginEmailInput) loginEmailInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (loginPasswordInput) loginPasswordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (signupNameInput) signupNameInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (signupEmailInput) signupEmailInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (signupPasswordInput) signupPasswordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));

    // Update buttons
    if (loginButton) loginButton->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));
    if (signupButton) signupButton->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));
    if (forgotPasswordButton) {
        forgotPasswordButton->setStyleSheet(
            "QPushButton {"
            "   border: none;"
            "   color: #8B5CF6;"
            "   font-size: 13px;"
            "   font-weight: 500;"
            "   padding: 0;"
            "}"
            "QPushButton:hover {"
            "   color: #7C3AED;"
            "   text-decoration: underline;"
            "}"
        );
    }

    // Update checkboxes
    if (rememberMeCheckbox) rememberMeCheckbox->setStyleSheet(UIUtils::getCheckboxStyle(isDarkTheme));
    if (termsCheckbox) termsCheckbox->setStyleSheet(UIUtils::getCheckboxStyle(isDarkTheme));

    // Save theme preference
    QSettings settings;
    settings.setValue("darkTheme", isDarkTheme);
}

void AuthPage::updateAllTextColors() const
{
    // Update welcome label
    if (const auto welcomeLabel = findChild<QLabel*>("welcomeLabel")) {
        welcomeLabel->setStyleSheet(QString("QLabel { font-size: 24px; font-weight: 600; color: %1; }")
            .arg(isDarkTheme ? "#F9FAFB" : "#111827"));
    }

    // Update subtitle label
    if (const auto subtitleLabel = findChild<QLabel*>("subtitleLabel")) {
        subtitleLabel->setStyleSheet(UIUtils::getSubtitleLabelStyle());
    }

    // Update title in nav bar
    if (const auto titleLabel = findChild<QLabel*>("titleLabel")) {
        titleLabel->setStyleSheet(QString("QLabel { font-size: 20px; font-weight: 600; color: %1; }")
            .arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }

    // Update login/signup tabs
    if (const auto loginTab = findChild<QPushButton*>("loginTab")) {
        loginTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, loginTab->property("active").toBool()));
    }
    if (const auto signupTab = findChild<QPushButton*>("signupTab")) {
        signupTab->setStyleSheet(UIUtils::getTabStyle(isDarkTheme, signupTab->property("active").toBool()));
    }

    // Update auth container
    if (const auto authContainer = findChild<QWidget*>("authContainer")) {
        authContainer->setStyleSheet(UIUtils::getAuthContainerStyle(isDarkTheme));
    }

    // Update input fields
    QList<QLineEdit*> inputs = findChildren<QLineEdit*>();
    for (QLineEdit* input : inputs) {
        input->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    }

    // Update checkboxes
    QList<QCheckBox*> checkboxes = findChildren<QCheckBox*>();
    for (QCheckBox* checkbox : checkboxes) {
        checkbox->setStyleSheet(UIUtils::getCheckboxStyle(isDarkTheme));
    }

    // Update buttons
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->objectName() != "loginTab" && button->objectName() != "signupTab") {
            button->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));
        }
    }
}

void AuthPage::showError(const QString& message) const
{
    messageText->setText(message);
    animateMessageWidget(true);
}

void AuthPage::showSuccess(const QString& message) const
{
    messageText->setText(message);
    animateMessageWidget(false);
}

void AuthPage::setupMessageWidget()
{
    // Create message widget
    messageWidget = new QWidget(this);
    messageWidget->setObjectName("messageWidget");
    messageWidget->setMinimumHeight(48);
    messageWidget->setMaximumWidth(600);
    messageWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    messageWidget->setAttribute(Qt::WA_TranslucentBackground);
    messageWidget->hide();

    const auto messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(16, 12, 16, 12);
    messageLayout->setSpacing(12);
    messageLayout->setAlignment(Qt::AlignCenter);

    // Create icon container for vertical centering
    const auto iconContainer = new QWidget;
    const auto iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setSpacing(0);
    iconLayout->setAlignment(Qt::AlignCenter);

    // Create icon label
    messageIcon = new QLabel;
    messageIcon->setFixedSize(24, 24);
    messageIcon->setStyleSheet(UIUtils::getMessageIconStyle(isDarkTheme));
    messageIcon->setAlignment(Qt::AlignCenter);
    iconLayout->addWidget(messageIcon);

    // Create text container for better alignment
    const auto textContainer = new QWidget;
    const auto textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(0);
    textLayout->setAlignment(Qt::AlignCenter);

    // Create text label with centered alignment
    messageText = new QLabel;
    messageText->setObjectName("messageText");
    messageText->setWordWrap(true);
    messageText->setAlignment(Qt::AlignCenter);
    messageText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    messageText->setMinimumWidth(250);
    textLayout->addWidget(messageText);

    messageLayout->addWidget(iconContainer);
    messageLayout->addWidget(textContainer, 1);

    // Create animations
    messageAnimation = new QSequentialAnimationGroup(this);

    slideAnimation = new QPropertyAnimation(messageWidget, "pos");
    slideAnimation->setDuration(400);
    slideAnimation->setEasingCurve(QEasingCurve::OutBack);

    shakeAnimation = new QPropertyAnimation(messageWidget, "pos");
    shakeAnimation->setDuration(150);
    shakeAnimation->setEasingCurve(QEasingCurve::OutBounce);

    fadeAnimation = new QPropertyAnimation(messageWidget, "windowOpacity");
    fadeAnimation->setDuration(250);
    fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    messageTimer = new QTimer(this);
    messageTimer->setSingleShot(true);
    connect(messageTimer, &QTimer::timeout, this, [this]() {
        fadeAnimation->setStartValue(1.0);
        fadeAnimation->setEndValue(0.0);
        fadeAnimation->start();
        connect(fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            messageWidget->hide();
        }, Qt::SingleShotConnection);
    });

    messageAnimation->addAnimation(slideAnimation);
    messageAnimation->addAnimation(shakeAnimation);
    messageAnimation->addAnimation(fadeAnimation);
}

void AuthPage::animateMessageWidget(bool isError) const
{
    // Set message widget style
    messageWidget->setStyleSheet(UIUtils::getMessageWidgetStyle(isDarkTheme, isError));

    // Set icon with error checking
    const QPixmap icon = UIUtils::getIcon(isError ? "error.png" : "success.png", 24);
    if (!icon.isNull()) {
        messageIcon->setPixmap(icon);
    } else {
        qDebug() << "Failed to load" << (isError ? "error" : "success") << "icon";
        messageIcon->setText(isError ? "X" : "✓");
    }

    // Reset animations
    messageAnimation->stop();
    slideAnimation->stop();
    shakeAnimation->stop();
    fadeAnimation->stop();
    messageTimer->stop();

    // Show widget and ensure it's sized correctly
    messageWidget->show();
    messageWidget->adjustSize();
    messageWidget->setWindowOpacity(1.0);

    // Get the welcome label and auth container
    const auto welcomeLabel = findChild<QLabel*>("welcomeLabel");
    const auto authContainer = findChild<QWidget*>("authContainer");
    if (!welcomeLabel || !authContainer) {
        qDebug() << "Failed to find welcome label or auth container";
        return;
    }

    // Calculate center position
    const QPoint containerCenter = authContainer->mapTo(this, authContainer->rect().center());
    int xPos = containerCenter.x() - (messageWidget->width() / 2);
    
    // Position the message between the welcome label and the form
    const int welcomeLabelBottom = welcomeLabel->mapTo(this, QPoint(0, welcomeLabel->height())).y();
    const int formTop = stackedWidget->mapTo(this, QPoint(0, 0)).y();
    const int yPos = welcomeLabelBottom + (formTop - welcomeLabelBottom) / 2 - messageWidget->height() / 2 + 20;

    // Ensure the message stays within the container bounds
    const int leftBound = authContainer->mapTo(this, QPoint(32, 0)).x();
    const int rightBound = authContainer->mapTo(this, QPoint(authContainer->width() - 32, 0)).x();
    xPos = qMax(leftBound, qMin(xPos, rightBound - messageWidget->width()));

    // Set up slide animation with adjusted starting position
    const QPoint startPos(xPos, yPos - 25);
    const QPoint endPos(xPos, yPos);

    slideAnimation->setStartValue(startPos);
    slideAnimation->setEndValue(endPos);

    messageWidget->move(startPos);

    if (isError) {

        QList<QPoint> shakePoints = {
            endPos + QPoint(6, 0),
            endPos + QPoint(-6, 0),
            endPos + QPoint(4, 0),
            endPos + QPoint(-4, 0),
            endPos + QPoint(2, 0),
            endPos
        };

        for (int i = 0; i < shakePoints.size(); ++i) {
            const qreal time = static_cast<qreal>(i) / (shakePoints.size() - 1);
            shakeAnimation->setKeyValueAt(time, QVariant(shakePoints[i]));
        }
    } else {
        shakeAnimation->setStartValue(endPos + QPoint(0, -4));
        shakeAnimation->setEndValue(endPos);
    }

    // Set up fade animation
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);

    // Start animation sequence
    messageAnimation->start();

    // Auto-hide after 5 seconds
    messageTimer->start(5000);
}

void AuthPage::clearFields()
{
    loginEmailInput->clear();
    loginPasswordInput->clear();
    signupNameInput->clear();
    signupEmailInput->clear();
    signupPasswordInput->clear();
    signupAgeInput->clear();
    selectedImagePath.clear();

    // Reset profile image button to default state with circular shape
    constexpr int size = 140;
    QPixmap defaultPhoto(size, size);
    defaultPhoto.fill(Qt::transparent);

    QPainter painter(&defaultPhoto);
    painter.setRenderHint(QPainter::Antialiasing);

    // Create circular path for clipping
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    // Draw circular background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(isDarkTheme ? "#1e293b" : "#f1f5f9"));
    painter.drawEllipse(0, 0, size, size);

    // Draw border
    painter.setClipping(false);
    painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
    painter.drawEllipse(1, 1, size-2, size-2);

    // Set default icon
    const QPixmap defaultIcon = UIUtils::getIcon("person.png", size/2);
    if (!defaultIcon.isNull()) {
        painter.drawPixmap(size/4, size/4, defaultIcon);
    }

    profileImageButton->setIcon(QIcon(defaultPhoto));
    profileImageButton->setIconSize(QSize(size, size));
    lastCircularPhoto = QPixmap();  // Clear stored circular photo

    termsCheckbox->setChecked(false);
}

void AuthPage::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateLayout();
    update();
}

void AuthPage::updateLayout() const
{
    const QSize size = this->size();
    const auto contentWidget = findChild<QWidget*>("contentWidget");
    if (!contentWidget || !contentWidget->layout()) return;

    const auto sliderWidget = findChild<QWidget*>("sliderWidget");
    const auto authWidget = findChild<QWidget*>("authWidget");
    const auto contentLayout = qobject_cast<QHBoxLayout*>(contentWidget->layout());
    if (!contentLayout) return;

    // Store current visibility state
    const bool wasSliderVisible = sliderWidget && sliderWidget->isVisible();
    
    // Responsive layout based on screen size
    if (size.width() < 1024) {
        // For small/medium screens, hide slider and give full width to auth form
        if (sliderWidget) {
            sliderWidget->hide();
            sliderWidget->setFixedWidth(0);
            contentLayout->setStretch(0, 0);
        }
        
        if (authWidget) {
            // Set minimum width based on screen size
            if (size.width() < 600) {
                authWidget->setMinimumWidth(280);
            } else if (size.width() < 800) {
                authWidget->setMinimumWidth(350);
            } else {
                authWidget->setMinimumWidth(450);
            }
            
            authWidget->setMaximumWidth(QWIDGETSIZE_MAX);
            contentLayout->setStretch(1, 1);
            
            // Adjust margins based on screen size
            if (const auto authLayout = qobject_cast<QVBoxLayout*>(authWidget->layout())) {
                if (size.width() < 800) {
                    authLayout->setContentsMargins(16, 16, 16, 16);
                } else {
                    authLayout->setContentsMargins(24, 24, 24, 24);
                }
            }
        }
    } else {
        // For large screens, show slider with proper size
        if (sliderWidget) {
            if (!wasSliderVisible) {
                sliderWidget->setFixedWidth(400);
            }
            sliderWidget->show();
            
            // Set minimum width based on screen size
            if (size.width() < 1200) {
                sliderWidget->setMinimumWidth(400);
            } else if (size.width() < 1600) {
                sliderWidget->setMinimumWidth(500);
            } else {
                sliderWidget->setMinimumWidth(600);
            }
            
            sliderWidget->setMaximumWidth(QWIDGETSIZE_MAX);
            contentLayout->setStretch(0, 3);
        }
        
        if (authWidget) {
            // Set minimum width based on screen size
            if (size.width() < 1200) {
                authWidget->setMinimumWidth(350);
            } else if (size.width() < 1600) {
                authWidget->setMinimumWidth(400);
            } else {
                authWidget->setMinimumWidth(450);
            }
            
            authWidget->setMaximumWidth(QWIDGETSIZE_MAX);
            contentLayout->setStretch(1, 2);
            
            if (const auto authLayout = qobject_cast<QVBoxLayout*>(authWidget->layout())) {
                authLayout->setContentsMargins(24, 24, 24, 24);
            }
        }
    }

    // Force immediate layout update
    contentLayout->invalidate();
    contentLayout->activate();
    contentWidget->updateGeometry();

    // Update auth container layout
    if (const auto authContainer = findChild<QWidget*>("authContainer")) {
        if (auto* containerLayout = qobject_cast<QVBoxLayout*>(authContainer->layout())) {
            if (size.width() < 800) {
                containerLayout->setContentsMargins(12, 12, 12, 12);
                containerLayout->setSpacing(12);
                authContainer->setMinimumHeight(450);
            } else if (size.width() < 1024) {
                containerLayout->setContentsMargins(20, 20, 20, 20);
                containerLayout->setSpacing(16);
                authContainer->setMinimumHeight(500);
            } else {
                containerLayout->setContentsMargins(24, 24, 24, 24);
                containerLayout->setSpacing(20);
                authContainer->setMinimumHeight(550);
            }
            
            // Set minimum width for auth container
            if (size.width() < 600) {
                authContainer->setMinimumWidth(280);
            } else if (size.width() < 800) {
                authContainer->setMinimumWidth(320);
            } else if (size.width() < 1024) {
                authContainer->setMinimumWidth(380);
            } else {
                authContainer->setMinimumWidth(400);
            }
            
            containerLayout->invalidate();
            containerLayout->activate();
        }
    }

    // Center the auth container when slider is hidden
    if (authWidget && (!sliderWidget || !sliderWidget->isVisible())) {
        const auto authContainer = findChild<QWidget*>("authContainer");
        if (authContainer) {
            // Maximum width for centered container
            const int maxWidth = qMin(size.width() - 64, 600);
            authContainer->setMaximumWidth(maxWidth);
            
            // Add horizontal centering for the auth container
            if (const auto authLayout = qobject_cast<QVBoxLayout*>(authWidget->layout())) {
                authLayout->setAlignment(Qt::AlignCenter);
            }
        }
    } else if (authWidget) {
        // Reset alignment when slider is visible
        const auto authContainer = findChild<QWidget*>("authContainer");
        if (authContainer) {
            authContainer->setMaximumWidth(QWIDGETSIZE_MAX);
            
            if (const auto authLayout = qobject_cast<QVBoxLayout*>(authWidget->layout())) {
                authLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            }
        }
    }

    // Update profile image size based on window width while maintaining circular shape
    if (profileImageButton) {
        const int profileSize = qMin(qMax(size.width() / 10, 80), 140);
        profileImageButton->setFixedSize(profileSize, profileSize);

        if (!selectedImagePath.isEmpty() && !lastCircularPhoto.isNull()) {
            const QPixmap scaledPhoto = lastCircularPhoto.scaled(profileSize, profileSize,
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            profileImageButton->setIcon(QIcon(scaledPhoto));
            profileImageButton->setIconSize(QSize(profileSize, profileSize));
        } else {
            // Create new default circular photo with adjusted size
            QPixmap defaultPhoto(profileSize, profileSize);
            defaultPhoto.fill(Qt::transparent);

            QPainter painter(&defaultPhoto);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addEllipse(0, 0, profileSize, profileSize);
            painter.setClipPath(path);

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(isDarkTheme ? "#1e293b" : "#f1f5f9"));
            painter.drawEllipse(0, 0, profileSize, profileSize);

            painter.setClipping(false);
            painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
            painter.drawEllipse(1, 1, profileSize-2, profileSize-2);

            const QPixmap defaultIcon = UIUtils::getIcon("person.png", profileSize/2);
            if (!defaultIcon.isNull()) {
                painter.drawPixmap(profileSize/4, profileSize/4, defaultIcon);
            }

            profileImageButton->setIcon(QIcon(defaultPhoto));
            profileImageButton->setIconSize(QSize(profileSize, profileSize));
        }
    }
}

bool AuthPage::eventFilter(QObject* obj, QEvent* event)
{
    // Handle clicks on toggle password icons
    if (event->type() == QEvent::MouseButtonPress) {
        const auto toggleIcon = qobject_cast<QLabel*>(obj);
        if (toggleIcon) {
            if (toggleIcon->parent() == loginPasswordInput) {
                if (loginPasswordInput->echoMode() == QLineEdit::Password) {
                    loginPasswordInput->setEchoMode(QLineEdit::Normal);
                    toggleIcon->setPixmap(UIUtils::getIconWithColor("open eyes.png", QColor(0x8B5CF6), 18));
                } else {
                    loginPasswordInput->setEchoMode(QLineEdit::Password);
                    toggleIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
                }
                return true;
            } else if (toggleIcon->parent() == signupPasswordInput) {
                if (signupPasswordInput->echoMode() == QLineEdit::Password) {
                    signupPasswordInput->setEchoMode(QLineEdit::Normal);
                    toggleIcon->setPixmap(UIUtils::getIconWithColor("open eyes.png", QColor(0x8B5CF6), 18));
                } else {
                    signupPasswordInput->setEchoMode(QLineEdit::Password);
                    toggleIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
                }
                return true;
            }
        }
    }
    // Handle resize events for password inputs
    else if (event->type() == QEvent::Resize) {
        const auto input = qobject_cast<QLineEdit*>(obj);
        if (input) {
            if (input == loginPasswordInput || input == signupPasswordInput) {
                QList<QLabel*> children = input->findChildren<QLabel*>();
                for (QLabel* child : children) {
                    if (child->cursor().shape() == Qt::PointingHandCursor) {
                        child->setGeometry(input->width() - 34, 14, 18, 18);
                    }
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void AuthPage::showTermsOfService()
{
    const auto dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Terms of Service"));
    dialog->setMinimumWidth(500);
    dialog->setMinimumHeight(500);
    dialog->resize(600, 600);

    const auto layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    // Add title label
    const auto titleLabel = new QLabel(tr("Terms of Service"));
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: " + QString(isDarkTheme ? "#e2e8f0" : "#1e293b") + ";"
        "   margin-bottom: 8px;"
        "}"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    const auto scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   border: none;"
        "   background-color: transparent;"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: " + QString(isDarkTheme ? "#2e2f3d" : "#f1f5f9") + ";"
        "   width: 8px;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: " + QString(isDarkTheme ? "#4b5563" : "#cbd5e1") + ";"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   border: none;"
        "   background: none;"
        "}"
    );

    const auto textContainer = new QWidget;
    const auto textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 16, 0);
    
    const auto textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setFrameStyle(QFrame::NoFrame);
    textEdit->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 16px;
            font-size: 14px;
            line-height: 1.6;
        }
    )").arg(
        isDarkTheme ? "#1a1b26" : "#ffffff",
        isDarkTheme ? "#e2e8f0" : "#1e293b",
        isDarkTheme ? "#2e2f3d" : "#e2e8f0"
    ));

    // Add terms of service text with improved formatting
    const QString termsText = tr(
        "<h2>Terms of Service for FitFlexPro</h2>"
        "<p>By accessing and using FitFlexPro, you agree to be bound by these Terms of Service.</p>"
        "<h3>1. Acceptance of Terms</h3>"
        "<p>By using FitFlexPro, you acknowledge that you have read, understood, and agree to be bound by these terms.</p>"
        "<h3>2. User Accounts</h3>"
        "<ul>"
        "<li>You must be at least 13 years old to create an account</li>"
        "<li>You are responsible for maintaining the confidentiality of your account</li>"
        "<li>You must provide accurate and complete information</li>"
        "</ul>"
        "<h3>3. Privacy</h3>"
        "<ul>"
        "<li>We collect and process your personal data as described in our Privacy Policy</li>"
        "<li>We implement security measures to protect your information</li>"
        "</ul>"
        "<h3>4. User Conduct</h3>"
        "<ul>"
        "<li>You agree not to misuse or abuse the service</li>"
        "<li>You will not attempt to breach security measures</li>"
        "<li>You will not share your account credentials</li>"
        "</ul>"
        "<h3>5. Intellectual Property</h3>"
        "<ul>"
        "<li>All content and features are owned by FitFlexPro</li>"
        "<li>You may not copy or reproduce any part of the service</li>"
        "</ul>"
        "<h3>6. Termination</h3>"
        "<ul>"
        "<li>We reserve the right to terminate accounts that violate these terms</li>"
        "<li>You may delete your account at any time</li>"
        "</ul>"
        "<h3>7. Changes to Terms</h3>"
        "<ul>"
        "<li>We may update these terms at any time</li>"
        "<li>Continued use constitutes acceptance of changes</li>"
        "</ul>"
        "<p><br>For questions about these terms, please contact <a href='mailto:fadygerges2023@gmail.com'>fadygerges2023@gmail.com</a></p>"
    );

    textEdit->setHtml(termsText);
    textLayout->addWidget(textEdit);
    scrollArea->setWidget(textContainer);
    layout->addWidget(scrollArea);

    // Button container with gradient background
    const auto buttonContainer = new QWidget;
    buttonContainer->setStyleSheet(
        "QWidget {"
        "   background: " + QString(isDarkTheme ? "#1a1b26" : "#ffffff") + ";"
        "   border-top: 1px solid " + QString(isDarkTheme ? "#2e2f3d" : "#e2e8f0") + ";"
        "   padding: 12px;"
        "}"
    );
    const auto buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 12, 0, 0);

    const auto closeButton = new QPushButton(tr("Close"));
    closeButton->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));
    closeButton->setFixedWidth(120);
    closeButton->setFixedHeight(40);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    
    layout->addWidget(buttonContainer);
    
    // Center the dialog on the parent widget
    const QPoint parentCenter = this->mapToGlobal(this->rect().center());
    dialog->move(parentCenter.x() - dialog->width() / 2,
                parentCenter.y() - dialog->height() / 2);
    
    dialog->exec();
    dialog->deleteLater();
}

void AuthPage::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    clearFields();
    
    // Load remembered credentials if they exist
    QString rememberedEmail, rememberedPassword;
    if (userDataManager->getRememberedCredentials(rememberedEmail, rememberedPassword)) {
        loginEmailInput->setText(rememberedEmail);
        loginPasswordInput->setText(rememberedPassword);
        rememberMeCheckbox->setChecked(true);
    }
}

void AuthPage::showConfirmationDialog(const QString& title, const QString& message,
    const QString& confirmText, const QString& cancelText, std::function<void()> onConfirm)
{
    auto dialog = new QDialog(this);
    dialog->setWindowTitle(title);
    dialog->setMinimumWidth(400);
    dialog->setMaximumWidth(400);
    dialog->setMinimumHeight(150);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    
    // Create main container with glass effect
    const auto container = new QWidget(dialog);
    container->setObjectName("confirmContainer");
    container->setStyleSheet(QString(R"(
        QWidget#confirmContainer {
            background: %1;
            border-radius: 16px;
            border: 1px solid %2;
        }
    )").arg(
        isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
        isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
    ));

    const auto layout = new QVBoxLayout(container);
    layout->setSpacing(24);
    layout->setContentsMargins(24, 24, 24, 24);

    // Add logo and title
    const auto headerContainer = new QWidget;
    const auto headerLayout = new QHBoxLayout(headerContainer);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);

    const auto logoLabel = new QLabel;
    const QPixmap logo(":/Images/dumbbell.png");
    logoLabel->setPixmap(logo.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    const auto titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: 600; color: %1;")
        .arg(isDarkTheme ? "#FFFFFF" : "#111827"));

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // Add close button
    const auto closeButton = new QPushButton;
    closeButton->setIcon(QIcon(":/Images/close.png"));
    closeButton->setFixedSize(24, 24);
    closeButton->setToolTip(tr("Close"));
    closeButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
        }
        QPushButton:hover {
            background: rgba(0, 0, 0, 0.1);
            border-radius: 12px;
        }
    )");
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::reject);
    headerLayout->addWidget(closeButton);

    // Message
    const auto messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet(QString("color: %1; font-size: 14px;")
        .arg(isDarkTheme ? "#E5E7EB" : "#4B5563"));

    // Buttons
    const auto buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

    const auto cancelButton = new QPushButton(cancelText);
    cancelButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: 500;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: %3;
        }
    )").arg(
        isDarkTheme ? "#374151" : "#E5E7EB",
        isDarkTheme ? "#F9FAFB" : "#374151",
        isDarkTheme ? "#4B5563" : "#D1D5DB"
    ));

    const auto confirmButton = new QPushButton(confirmText);
    confirmButton->setStyleSheet(R"(
        QPushButton {
            background-color: #EF4444;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: 500;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #DC2626;
        }
    )");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);

    // Add all elements to main layout
    layout->addWidget(headerContainer);
    layout->addWidget(messageLabel);
    layout->addLayout(buttonLayout);

    // Set up dialog layout
    const auto dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(container);

    // Center dialog on parent
    const QPoint parentCenter = this->mapToGlobal(this->rect().center());
    dialog->move(parentCenter.x() - dialog->width() / 2,
                parentCenter.y() - dialog->height() / 2);

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, [dialog, onConfirm]() {
        onConfirm();
        dialog->accept();
    });

    dialog->exec();
    dialog->deleteLater();
} 

void AuthPage::retranslateUI()
{
    // Update welcome label
    if (const auto welcomeLabel = findChild<QLabel*>("welcomeLabel")) {
        welcomeLabel->setText(tr("Welcome to FitFlexPro"));
    }

    // Update subtitle label
    if (const auto subtitleLabel = findChild<QLabel*>("subtitleLabel")) {
        subtitleLabel->setText(tr("Welcome back! Please enter your details."));
    }

    // Update tab buttons
    if (const auto loginTab = findChild<QPushButton*>("loginTab")) {
        loginTab->setText(tr("Login"));
    }
    if (const auto signupTab = findChild<QPushButton*>("signupTab")) {
        signupTab->setText(tr("Sign Up"));
    }

    // Update login form elements
    if (loginEmailInput) {
        loginEmailInput->setPlaceholderText(tr("Email address"));
    }
    if (loginPasswordInput) {
        loginPasswordInput->setPlaceholderText(tr("Password"));
    }
    if (rememberMeCheckbox) {
        rememberMeCheckbox->setText(tr("Remember me"));
    }
    if (forgotPasswordButton) {
        forgotPasswordButton->setText(tr("Forgot Password?"));
    }
    if (loginButton) {
        loginButton->setText(tr("Sign In"));
    }

    // Update signup form elements
    if (signupNameInput) {
        signupNameInput->setPlaceholderText(tr("Your Name"));
    }
    if (signupEmailInput) {
        signupEmailInput->setPlaceholderText(tr("Email address"));
    }
    if (signupPasswordInput) {
        signupPasswordInput->setPlaceholderText(tr("Password"));
    }
    if (signupButton) {
        signupButton->setText(tr("Sign Up"));
    }
    if (termsCheckbox) {
        termsCheckbox->setText(tr("I agree to the"));
    }

    // Update other labels
    if (const auto orLabel = findChild<QLabel*>("orLabel")) {
        orLabel->setText(tr("Or continue with"));
    }
    if (const auto uploadLabel = findChild<QLabel*>("uploadLabel")) {
        uploadLabel->setText(tr("Upload Photo"));
    }

    // Update image slider
    if (imageSlider) {
        imageSlider->retranslateUI();
    }

    // Force layout update
    updateLayout();
}
