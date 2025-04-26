#include "addmemberpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QImageReader>
#include <QPainter>
#include <QTimer>
#include <QDebug>
AddMemberPage::AddMemberPage(UserDataManager* userDataManager, MemberDataManager* memberDataManger, QWidget* parent)
    : QMainWindow(parent)
    , isDarkTheme(ThemeManager::getInstance().isDarkTheme())
    , userDataManager(userDataManager)
    , memberDataManger(memberDataManger)
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
        this, &AddMemberPage::retranslateUI);
}
void AddMemberPage::setupUI()
{
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(0, 0, 0, 0);

    // Content Area
    auto* contentWidget = new QWidget;
    contentWidget->setObjectName("contentWidget");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setContentsMargins(24, 24, 24, 24);

    // Form Container
    auto* formContainer = new QWidget;
    formContainer->setObjectName("formContainer");
    formContainer->setStyleSheet(UIUtils::getAuthContainerStyle(isDarkTheme));
    formContainer->setFixedWidth(400);

    auto* formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(16);
    formLayout->setContentsMargins(24, 24, 24, 24);
    formLayout->setAlignment(Qt::AlignCenter);

    // Title
    auto* titleLabel2 = new QLabel(tr("Add New Member"));
    titleLabel2->setStyleSheet(UIUtils::getWelcomeLabelStyle(isDarkTheme));
    titleLabel2->setTextFormat(Qt::RichText);
    titleLabel2->setAlignment(Qt::AlignCenter);

    // Profile Image
    profileImageButton = new QPushButton;
    profileImageButton->setFixedSize(100, 100);
    profileImageButton->setIcon(UIUtils::getIcon("person.png", 50));
    profileImageButton->setIconSize(QSize(50, 50));
    profileImageButton->setStyleSheet(UIUtils::getProfileUploadStyle(isDarkTheme));
    //mainVLayout->addWidget(profileImageButton, 0, Qt::AlignCenter);
    connect(profileImageButton, &QPushButton::clicked, this, &AddMemberPage::selectProfileImage);

    auto* uploadLabel = new QLabel(tr("Upload Photo"));
    uploadLabel->setStyleSheet(UIUtils::getProfileUploadLabelStyle(isDarkTheme));
    uploadLabel->setAlignment(Qt::AlignCenter);

    // Input Fields
    nameInput = new QLineEdit;
    nameInput->setPlaceholderText(tr("Full Name"));
    nameInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    nameInput->setFixedHeight(45);
    nameInput->setFixedWidth(352);

    auto* nameIcon = new QLabel(nameInput);
    nameIcon->setPixmap(UIUtils::getIconWithColor("person_bw.png", QColor(0x8B5CF6), 18));
    nameIcon->setGeometry(16, 14, 18, 18);

    emailInput = new QLineEdit;
    emailInput->setPlaceholderText(tr("Email Address"));
    emailInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    emailInput->setFixedHeight(45);
    emailInput->setFixedWidth(352);

    auto* emailIcon = new QLabel(emailInput);
    emailIcon->setPixmap(UIUtils::getIconWithColor("mail.png", QColor(0x8B5CF6), 18));
    emailIcon->setGeometry(16, 14, 18, 18);

    passwordInput = new QLineEdit;
    passwordInput->setPlaceholderText(tr("Password"));
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    passwordInput->setFixedHeight(45);
    passwordInput->setFixedWidth(352);

    auto* lockIcon = new QLabel(passwordInput);
    lockIcon->setPixmap(UIUtils::getIconWithColor("lock.png", QColor(0x8B5CF6), 18));
    lockIcon->setGeometry(16, 14, 18, 18);

    auto* togglePasswordIcon = new QLabel(passwordInput);
    togglePasswordIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
    togglePasswordIcon->setStyleSheet("QLabel { cursor: pointer; }");
    togglePasswordIcon->setGeometry(352 - 34, 14, 18, 18);
    togglePasswordIcon->installEventFilter(this);

    dateOfBirthInput = new QDateEdit;
    dateOfBirthInput->setDisplayFormat("dd/MM/yyyy");
    dateOfBirthInput->setCalendarPopup(true);
    dateOfBirthInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    dateOfBirthInput->setFixedHeight(45);
    dateOfBirthInput->setFixedWidth(352);

    auto* calendarIcon = new QLabel(dateOfBirthInput);
    calendarIcon->setPixmap(UIUtils::getIconWithColor("calendar.png", QColor(0x8B5CF6), 18));
    calendarIcon->setGeometry(16, 14, 18, 18);

    // Add Member Button
    addMemberButton = new QPushButton(tr("Add Member"));
    addMemberButton->setFixedHeight(45);
    addMemberButton->setFixedWidth(352);
    addMemberButton->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));
    connect(addMemberButton, &QPushButton::clicked, this, &AddMemberPage::handleAddMember);

    // Add widgets to form layout
    formLayout->addWidget(titleLabel2);
    formLayout->addWidget(profileImageButton);
    formLayout->addWidget(uploadLabel);
    formLayout->addWidget(nameInput);
    formLayout->addWidget(emailInput);
    formLayout->addWidget(passwordInput);
    formLayout->addWidget(dateOfBirthInput);
    formLayout->addWidget(addMemberButton);

    contentLayout->addStretch();
    contentLayout->addWidget(formContainer);
    contentLayout->addStretch();
    mainVLayout->addWidget(contentWidget);
}
void AddMemberPage::handleAddMember(){}
void AddMemberPage::selectProfileImage(){}
void AddMemberPage::retranslateUI(){}
void AddMemberPage::setupGlassEffect(){}
void AddMemberPage::setupMessageWidget(){}
void AddMemberPage::updateTheme(bool){}