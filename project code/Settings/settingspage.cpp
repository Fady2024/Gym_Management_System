#include "settingspage.h"
#include "mainpage.h"
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QDialog>
#include <QEvent>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QSizePolicy>
#include <QFile>
#include <QFileDialog>
#include "UIUtils.h"
#include <QTimer>
#include <QScrollArea>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>

SettingsPage::SettingsPage(UserDataManager* userDataManager, QWidget *parent)
    : QWidget(parent)
    , userDataManager(userDataManager)
    , isDarkTheme(false)
    , mainLayout(nullptr)
    , leftLayout(nullptr)
    , contentStack(nullptr)
    , settingsContent(nullptr)
    , developerPage(nullptr)
    , settingsTabButton(nullptr)
    , developerTabButton(nullptr)
{
    setupUI();
    loadUserData();
}

void SettingsPage::showMessageDialog(const QString& message, bool isError)
{
    auto dialog = new QDialog(this);
    dialog->setWindowTitle(isError ? tr("Error") : tr("Success"));
    dialog->setFixedSize(400, 240);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setAttribute(Qt::WA_TranslucentBackground);

    const auto container = new QWidget(dialog);
    container->setObjectName("messageContainer");
    container->setStyleSheet(QString(R"(
        QWidget#messageContainer {
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
    layout->setContentsMargins(32, 32, 32, 32);

    // Icon
    const auto iconLabel = new QLabel;
    iconLabel->setFixedSize(64, 64);
    iconLabel->setScaledContents(false);
    iconLabel->setAlignment(Qt::AlignCenter);

    QPixmap iconPixmap(isError ? ":/Images/error.png" : ":/Images/success.png");
    if (!iconPixmap.isNull()) {
        iconPixmap = iconPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        iconLabel->setPixmap(iconPixmap);
    }

    // Message
    const auto messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 500;")
        .arg(isDarkTheme ? "#F9FAFB" : "#111827"));

    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(messageLabel);

    const auto dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(container);

    // Center on parent
    const QPoint parentCenter = this->mapToGlobal(this->rect().center());
    dialog->move(parentCenter.x() - dialog->width() / 2,
                parentCenter.y() - dialog->height() / 2);

    // Add fade animations
    const auto opacityEffect = new QGraphicsOpacityEffect(dialog);
    container->setGraphicsEffect(opacityEffect);

    const auto fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    const auto fadeOut = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    const auto dismissTimer = new QTimer(dialog);
    dismissTimer->setSingleShot(true);
    connect(dismissTimer, &QTimer::timeout, [dialog, fadeOut]() {
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
    connect(fadeOut, &QPropertyAnimation::finished, dialog, &QDialog::accept);

    dialog->show();
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    dismissTimer->start(2000);

    dialog->exec();
    dialog->deleteLater();
}

void SettingsPage::setupUI()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Left sidebar
    const auto leftSidebar = new QWidget;
    leftSidebar->setObjectName("leftSidebar");
    leftSidebar->setFixedWidth(72); // Slightly reduced width
    leftSidebar->setStyleSheet(QString(R"(
        QWidget#leftSidebar {
            background: %1;
            border-right: 1px solid %2;
        }
    )").arg(
        isDarkTheme ? "rgba(31, 41, 55, 0.98)" : "rgba(255, 255, 255, 0.98)",
        isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.05)"
    ));

    leftLayout = new QVBoxLayout(leftSidebar);
    leftLayout->setSpacing(16);
    leftLayout->setContentsMargins(12, 24, 12, 24);
    leftLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // Create tab buttons with enhanced styling
    auto createTabButton = [this](const QString& icon, const QString& tooltip) {
        const auto button = new QPushButton;
        button->setToolTip(tooltip);
        button->setFixedSize(48, 48);
        button->setCursor(Qt::PointingHandCursor);
        button->setCheckable(true);
        button->setAutoExclusive(true);

        // Create colored icons for different states
        QIcon normalIcon;
        QIcon activeIcon;
        QPixmap originalPixmap(icon);
        if (!originalPixmap.isNull()) {
            // Normal state icon (gray)
            QPixmap normalPixmap = originalPixmap;
            QPainter normalPainter(&normalPixmap);
            normalPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            normalPainter.fillRect(normalPixmap.rect(), isDarkTheme ? QColor(156, 163, 175) : QColor(75, 85, 99));
            normalIcon.addPixmap(normalPixmap);

            // Active state icon (white)
            QPixmap activePixmap = originalPixmap;
            QPainter activePainter(&activePixmap);
            activePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            activePainter.fillRect(activePixmap.rect(), QColor(255, 255, 255));
            activeIcon.addPixmap(activePixmap);
        }

        button->setIcon(normalIcon);
        button->setIconSize(QSize(24, 24));
        button->setStyleSheet(QString(R"(
            QPushButton {
                background: %1;
                border: none;
                border-radius: 14px;
                padding: 0;
                margin: 0;
                qproperty-iconSize: 24px 24px;
            }
            QPushButton:hover:!checked {
                background: %2;
            }
            QPushButton:checked {
                background: %3;
            }
            QPushButton:checked:hover {
                background: %4;
            }
        )").arg(
            isDarkTheme ? "rgba(31, 41, 55, 0.5)" : "rgba(255, 255, 255, 0.5)",
            isDarkTheme ? "rgba(139, 92, 246, 0.15)" : "rgba(139, 92, 246, 0.1)",
            isDarkTheme ? "#8B5CF6" : "#7C3AED",
            isDarkTheme ? "#7C3AED" : "#6D28D9"
        ));

        // Change icon when button state changes
        connect(button, &QPushButton::toggled, [button, normalIcon, activeIcon](bool checked) {
            button->setIcon(checked ? activeIcon : normalIcon);
        });

        // Add glow effect for better depth
        QGraphicsDropShadowEffect* glowEffect = new QGraphicsDropShadowEffect(button);
        glowEffect->setBlurRadius(15);
        glowEffect->setColor(QColor(139, 92, 246, isDarkTheme ? 80 : 40));
        glowEffect->setOffset(0, 0);
        button->setGraphicsEffect(glowEffect);

        return button;
    };

    settingsTabButton = createTabButton(":/Images/settings.png", tr("Settings"));
    developerTabButton = createTabButton(":/Images/team.png", tr("Developer Team"));

    // Set up button group for mutual exclusivity
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(settingsTabButton);
    buttonGroup->addButton(developerTabButton);
    buttonGroup->setExclusive(true);

    // Add spacer for better vertical alignment
    QSpacerItem* topSpacer = new QSpacerItem(0, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    leftLayout->addItem(topSpacer);

    leftLayout->addWidget(settingsTabButton, 0, Qt::AlignCenter);
    leftLayout->addWidget(developerTabButton, 0, Qt::AlignCenter);
    leftLayout->addStretch();

    // Connect signals for navigation
    connect(settingsTabButton, &QPushButton::clicked, this, &SettingsPage::handleSettingsTab);
    connect(developerTabButton, &QPushButton::clicked, this, &SettingsPage::handleDeveloperTab);

    // Content area
    contentStack = new QStackedWidget;
    contentStack->setStyleSheet("QStackedWidget { background: transparent; }");

    // Settings content
    settingsContent = new QWidget;
    const auto settingsLayout = new QVBoxLayout(settingsContent);
    settingsLayout->setSpacing(0);
    settingsLayout->setContentsMargins(0, 0, 0, 0);
    settingsLayout->setAlignment(Qt::AlignCenter);

    // Create a container widget for centering
    const auto centerContainer = new QWidget;
    centerContainer->setStyleSheet("background: transparent;");
    const auto centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setSpacing(0);
    centerLayout->setContentsMargins(24, 24, 24, 24);
    centerLayout->setAlignment(Qt::AlignCenter);

    // Card container setup
    const auto cardContainer = new QWidget;
    cardContainer->setObjectName("cardContainer");
    cardContainer->setMinimumWidth(480);
    cardContainer->setMaximumWidth(800);
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cardContainer->setStyleSheet(R"(
        QWidget#cardContainer {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 24px;
            border: 1px solid rgba(0, 0, 0, 0.1);
        }
    )");

    const auto cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setSpacing(24);
    cardLayout->setContentsMargins(32, 32, 32, 32);

    // Profile section setup
    const auto profileSection = new QWidget;
    const auto profileLayout = new QVBoxLayout(profileSection);
    profileLayout->setSpacing(8);
    profileLayout->setAlignment(Qt::AlignCenter);

    // Profile image container with proper spacing
    const auto profileContainer = new QWidget;
    const auto profileContainerLayout = new QVBoxLayout(profileContainer);
    profileContainerLayout->setSpacing(8);
    profileContainerLayout->setAlignment(Qt::AlignCenter);
    profileContainerLayout->setContentsMargins(6, 6, 6, 8);

    profileImageButton = new QPushButton;
    profileImageButton->setFixedSize(140, 140);
    profileImageButton->setCursor(Qt::PointingHandCursor);
    profileImageButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: 3px solid #8B5CF6;
            border-radius: 70px;
            padding: 0px;
            margin: 0px;
        }
        QPushButton:hover {
            background-color: rgba(0, 0, 0, 0.1);
        }
    )");

    // Set default profile image immediately after creating the button
    resetToDefaultProfileImage();

    connect(profileImageButton, &QPushButton::clicked, this, &SettingsPage::changeProfilePicture);

    const auto uploadLabel = new QLabel("Change Photo");
    uploadLabel->setStyleSheet(UIUtils::getProfileUploadLabelStyle(false));
    uploadLabel->setAlignment(Qt::AlignCenter);

    profileContainerLayout->addWidget(profileImageButton, 0, Qt::AlignCenter);
    profileContainerLayout->addWidget(uploadLabel, 0, Qt::AlignCenter);

    profileLayout->addWidget(profileContainer);

    // User information setup with reduced spacing
    const auto userInfoLayout = new QVBoxLayout;
    userInfoLayout->setSpacing(16);
    userInfoLayout->setContentsMargins(0, 8, 0, 8);

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Your Name");
    nameEdit->setAlignment(Qt::AlignCenter);
    nameEdit->setStyleSheet(R"(
        QLineEdit {
            background: transparent;
            border: none;
            color: #1F2937;
            font-size: 24px;
            font-weight: bold;
            padding: 4px;
        }
        QLineEdit:focus {
            background: rgba(139, 92, 246, 0.1);
            border-radius: 8px;
        }
    )");

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("your.email@example.com");
    emailEdit->setAlignment(Qt::AlignCenter);
    emailEdit->setStyleSheet(R"(
        QLineEdit {
            background: transparent;
            border: none;
            color: #4B5563;
            font-size: 16px;
            padding: 4px;
        }
        QLineEdit:focus {
            background: rgba(139, 92, 246, 0.1);
            border-radius: 8px;
        }
    )");

    userInfoLayout->addWidget(nameEdit);
    userInfoLayout->addWidget(emailEdit);
    profileLayout->addLayout(userInfoLayout);

    // Create a horizontal layout for buttons
    const auto buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    buttonLayout->setAlignment(Qt::AlignCenter);

    // Save button setup
    saveButton = new QPushButton("Save Changes");
    saveButton->setCursor(Qt::PointingHandCursor);
    saveButton->setFixedHeight(40);
    saveButton->setFixedWidth(180);
    saveButton->setStyleSheet(R"(
        QPushButton {
            background: #8B5CF6;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 18px;
            font-weight: 800;
            min-width: 160px;
        }
        QPushButton:hover {
            background: #7C3AED;
        }
    )");

    // Reset button setup
    resetButton = new QPushButton("Reset");
    resetButton->setCursor(Qt::PointingHandCursor);
    resetButton->setFixedHeight(40);
    resetButton->setFixedWidth(180);
    resetButton->setStyleSheet(R"(
        QPushButton {
            background: #4B5563;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 18px;
            font-weight: 800;
            min-width: 160px;
        }
        QPushButton:hover {
            background: #374151;
        }
    )");

    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(saveButton);

    profileLayout->addLayout(buttonLayout);
    cardLayout->addWidget(profileSection);

    // Divider
    const auto divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background: rgba(0, 0, 0, 0.1);");
    cardLayout->addWidget(divider);

    // Account actions setup
    const auto actionsSection = new QWidget;
    const auto actionsLayout = new QVBoxLayout(actionsSection);
    actionsLayout->setSpacing(12);
    actionsLayout->setContentsMargins(0, 0, 0, 0);

    logoutButton = new QPushButton("  Log Out");
    logoutButton->setCursor(Qt::PointingHandCursor);
    logoutButton->setFixedHeight(40);
    logoutButton->setStyleSheet(R"(
        QPushButton {
            background: #374151;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 15px;
            font-weight: 600;
            text-align: center;
            image: url(Images/logout.png);
            qproperty-iconSize: 18px;
        }
        QPushButton:hover {
            background: #4B5563;
        }
    )");

    deleteAccountButton = new QPushButton("  Delete Account");
    deleteAccountButton->setCursor(Qt::PointingHandCursor);
    deleteAccountButton->setFixedHeight(40);
    deleteAccountButton->setStyleSheet(R"(
        QPushButton {
            background: #EF4444;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 15px;
            font-weight: 600;
            text-align: center;
            image: url(Images/delete.png);
            qproperty-iconSize: 18px;
        }
        QPushButton:hover {
            background: #DC2626;
        }
    )");

    actionsLayout->addWidget(logoutButton);
    actionsLayout->addWidget(deleteAccountButton);
    cardLayout->addWidget(actionsSection);

    // Add card to center container
    centerLayout->addWidget(cardContainer, 0, Qt::AlignCenter);

    // Add center container to settings layout
    settingsLayout->addWidget(centerContainer);

    // Create developer page
    developerPage = new DeveloperPage(this);

    // Add pages to stack
    contentStack->addWidget(settingsContent);
    contentStack->addWidget(developerPage);

    // Add widgets to main layout
    mainLayout->addWidget(leftSidebar);
    mainLayout->addWidget(contentStack);

    // Connect signals
    connect(saveButton, &QPushButton::clicked, this, &SettingsPage::saveChanges);
    connect(resetButton, &QPushButton::clicked, this, &SettingsPage::resetChanges);
    connect(logoutButton, &QPushButton::clicked, this, &SettingsPage::handleLogout);
    connect(deleteAccountButton, &QPushButton::clicked, this, &SettingsPage::handleDeleteAccount);

    // Initialize with settings tab
    handleSettingsTab();
}

void SettingsPage::handleSettingsTab()
{
    contentStack->setCurrentWidget(settingsContent);
    updateTabButtons();
    settingsTabButton->setChecked(true);
}

void SettingsPage::handleDeveloperTab()
{
    // Create developer page if it doesn't exist
    if (!developerPage) {
        developerPage = new DeveloperPage(this);
        contentStack->addWidget(developerPage);
    }

    // Show developer page
    contentStack->setCurrentWidget(developerPage);
    
    // Update tab buttons
    settingsTabButton->setChecked(false);
    developerTabButton->setChecked(true);
    
    // Ensure proper sizing
    developerPage->setMinimumSize(contentStack->size());
    developerPage->updateLayout();
    
    // Force update
    developerPage->show();
    developerPage->raise();
    
    // Update the layout after a short delay to ensure proper rendering
    QTimer::singleShot(100, developerPage, &DeveloperPage::updateLayout);
}

void SettingsPage::updateTabButtons()
{
    // Update button styles based on current theme
    const auto buttonStyle = isDarkTheme ? R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #9CA3AF;
            font-size: 14px;
            font-weight: 500;
            padding: 8px 16px;
            border-radius: 8px;
            text-align: left;
        }
        QPushButton:hover {
            background: rgba(139, 92, 246, 0.1);
            color: #8B5CF6;
        }
        QPushButton:checked {
            background: #8B5CF6;
            color: white;
        }
    )" : R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #64748b;
            font-size: 14px;
            font-weight: 500;
            padding: 8px 16px;
            border-radius: 8px;
            text-align: left;
        }
        QPushButton:hover {
            background: rgba(139, 92, 246, 0.1);
            color: #8B5CF6;
        }
        QPushButton:checked {
            background: #8B5CF6;
            color: white;
        }
    )";

    settingsTabButton->setStyleSheet(buttonStyle);
    developerTabButton->setStyleSheet(buttonStyle);
}

void SettingsPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;

    // Update left sidebar
    if (const auto leftSidebar = findChild<QWidget*>("leftSidebar")) {
        leftSidebar->setStyleSheet(QString(R"(
            QWidget#leftSidebar {
                background: %1;
                border-right: 1px solid %2;
            }
        )").arg(
            isDark ? "rgba(31, 41, 55, 0.98)" : "rgba(255, 255, 255, 0.98)",
            isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.05)"
        ));
    }

    // Update tab buttons
    const QString tabButtonStyle = QString(R"(
        QPushButton {
            background: transparent;
            border: none;
            color: %1;
            font-size: 14px;
            font-weight: 500;
            padding: 8px 16px;
            border-radius: 8px;
            text-align: left;
        }
        QPushButton:hover {
            background: rgba(139, 92, 246, 0.1);
            color: #8B5CF6;
        }
        QPushButton:checked {
            background: #8B5CF6;
            color: white;
        }
    )").arg(isDark ? "#64748b" : "#64748b");

    settingsTabButton->setStyleSheet(tabButtonStyle);
    developerTabButton->setStyleSheet(tabButtonStyle);

    // Update card container
    if (const auto cardContainer = findChild<QWidget*>("cardContainer")) {
        cardContainer->setStyleSheet(QString(R"(
            QWidget#cardContainer {
                background: %1;
                border-radius: 24px;
                border: 1px solid %2;
            }
        )").arg(
            isDark ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
            isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
        ));
    }

    // Update input fields
    nameEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background: transparent;
            border: none;
            color: %1;
            font-size: 24px;
            font-weight: bold;
            padding: 4px;
        }
        QLineEdit:focus {
            background: rgba(139, 92, 246, 0.1);
            border-radius: 8px;
        }
    )").arg(isDark ? "#FFFFFF" : "#1F2937"));

    emailEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background: transparent;
            border: none;
            color: %1;
            font-size: 16px;
            padding: 4px;
        }
        QLineEdit:focus {
            background: rgba(139, 92, 246, 0.1);
            border-radius: 8px;
        }
    )").arg(isDark ? "rgba(255, 255, 255, 0.8)" : "#4B5563"));

    // Update buttons
    saveButton->setStyleSheet(R"(
        QPushButton {
            background: #8B5CF6;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 18px;
            font-weight: 800;
            min-width: 160px;
        }
        QPushButton:hover {
            background: #7C3AED;
        }
    )");

    resetButton->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 18px;
            font-weight: 800;
            min-width: 160px;
        }
        QPushButton:hover {
            background: %2;
        }
    )").arg(
        isDark ? "#4B5563" : "#6B7280",
        isDark ? "#374151" : "#4B5563"
    ));

    logoutButton->setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 15px;
            font-weight: 600;
            text-align: center;
            image: url(Images/logout.png);
            qproperty-iconSize: 18px;
        }
        QPushButton:hover {
            background: %2;
        }
    )").arg(
        isDark ? "#374151" : "#4B5563",
        isDark ? "#4B5563" : "#6B7280"
    ));

    deleteAccountButton->setStyleSheet(R"(
        QPushButton {
            background: #EF4444;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: 15px;
            font-weight: 600;
            text-align: center;
            image: url(Images/delete.png);
            qproperty-iconSize: 18px;
        }
        QPushButton:hover {
            background: #DC2626;
        }
    )");

    // Update developer page theme
    if (developerPage) {
        developerPage->updateTheme(isDark);
    }

    // Update the default profile image for the new theme
    if (currentPhotoPath.isEmpty()) {
        resetToDefaultProfileImage();
    }
}

void SettingsPage::onUserDataLoaded(const QString& email) {
    loadUserData(email);
}

void SettingsPage::loadUserData(const QString& email)
{
    if (email.isEmpty()) {
        nameEdit->clear();
        emailEdit->clear();
        resetToDefaultProfileImage();
        currentPhotoPath = "";
        return;
    }

    const User userData = userDataManager->getUserData(email);
    if (!userData.getEmail().isEmpty()) {
        nameEdit->setText(userData.getName());
        emailEdit->setText(userData.getEmail());

        const QString photoPath = userData.getUserPhotoPath();
        if (!photoPath.isEmpty() && QFile::exists(photoPath)) {
            const QPixmap photo(photoPath);
            if (!photo.isNull()) {
                QPixmap circularPhoto(140, 140);
                circularPhoto.fill(Qt::transparent);

                QPainter painter(&circularPhoto);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);

                QPainterPath path;
                path.addEllipse(0, 0, 140, 140);
                painter.setClipPath(path);

                constexpr QSize targetSize(140, 140);
                QPixmap scaledPhoto;

                const qreal sourceRatio = static_cast<qreal>(photo.width()) / photo.height();
                constexpr qreal targetRatio = static_cast<qreal>(targetSize.width()) / targetSize.height();

                if (sourceRatio > targetRatio) {
                    scaledPhoto = photo.scaledToHeight(targetSize.height(), Qt::SmoothTransformation);
                } else {
                    scaledPhoto = photo.scaledToWidth(targetSize.width(), Qt::SmoothTransformation);
                }

                const int x = (scaledPhoto.width() - targetSize.width()) / 2;
                const int y = (scaledPhoto.height() - targetSize.height()) / 2;

                painter.drawPixmap(-x, -y, scaledPhoto);

                painter.setClipping(false);
                painter.setPen(QPen(QColor("#8B5CF6"), 3));
                painter.drawEllipse(1, 1, 137, 137);

                profileImageButton->setIcon(QIcon(circularPhoto));
                profileImageButton->setIconSize(QSize(140, 140));
                currentPhotoPath = photoPath;
            }
        } else {
            resetToDefaultProfileImage();
        }
    } else {
        nameEdit->clear();
        emailEdit->clear();
        resetToDefaultProfileImage();
        currentPhotoPath = "";
    }
}

void SettingsPage::changeProfilePicture()
{
    const QString filePath = QFileDialog::getOpenFileName(this,
        "Select Profile Picture", "", "Image Files (*.png *.jpg *.jpeg)");

    if (!filePath.isEmpty()) {
        const QPixmap photo(filePath);
        if (!photo.isNull()) {
            // Create a pixmap for the content
            QPixmap circularPhoto(110, 110);
            circularPhoto.fill(Qt::transparent);

            QPainter painter(&circularPhoto);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            QPainterPath path;
            path.addEllipse(0, 0, 110, 110);
            painter.setClipPath(path);

            const QPixmap scaledPhoto = photo.scaled(110, 110, 
                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

            // Center the photo
            const int x = (scaledPhoto.width() - 110) / 2;
            const int y = (scaledPhoto.height() - 110) / 2;
            painter.drawPixmap(-x, -y, scaledPhoto);

            profileImageButton->setIcon(QIcon(circularPhoto));
            profileImageButton->setIconSize(QSize(110, 110));
            currentPhotoPath = filePath;
        }
    }
}

void SettingsPage::updateSuccessDialogTheme(QDialog* dialog, bool isDark)
{
    if (const auto container = dialog->findChild<QWidget*>("successContainer")) {
        container->setStyleSheet(QString(R"(
            QWidget#successContainer {
                background: %1;
                border-radius: 16px;
                border: 1px solid %2;
                box-shadow: 0 4px 6px %3;
            }
        )").arg(
            isDark ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
            isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)",
            isDark ? "rgba(0, 0, 0, 0.3)" : "rgba(0, 0, 0, 0.1)"
        ));
    }

    if (const auto messageLabel = dialog->findChild<QLabel*>("messageLabel")) {
        messageLabel->setStyleSheet(QString(R"(
            QLabel#messageLabel {
                color: %1;
                font-size: 18px;
                font-weight: 600;
                padding: 8px;
            }
        )").arg(isDark ? "#FFFFFF" : "#1F2937"));
    }

    dialog->setStyleSheet(QString(R"(
        QDialog {
            background: %1;
        }
    )").arg(isDark ? "rgba(0, 0, 0, 0.5)" : "rgba(0, 0, 0, 0.2)"));
}

void SettingsPage::saveChanges()
{
    QString currentUserEmail;
    QString dummyPassword;
    if (userDataManager->getRememberedCredentials(currentUserEmail, dummyPassword)) {
        User userData = userDataManager->getUserData(currentUserEmail);
        
        // Store current values before validation
        QString previousName = userData.getName();
        QString previousEmail = userData.getEmail();
        QString previousPhotoPath = userData.getUserPhotoPath();

        bool hasValidationError = false;
        QString errorMessage;

        // Validate both name and email fields
        if (nameEdit->text().trimmed().isEmpty()) {
            hasValidationError = true;
            errorMessage = tr("Name field cannot be empty!");
        } else if (emailEdit->text().trimmed().isEmpty()) {
            hasValidationError = true;
            errorMessage = tr("Email field cannot be empty!");
        }

        if (hasValidationError) {
            // Restore all previous values
            nameEdit->setText(previousName);
            emailEdit->setText(previousEmail);
            
            // Restore previous photo
            if (!previousPhotoPath.isEmpty() && QFile::exists(previousPhotoPath)) {
                currentPhotoPath = previousPhotoPath;
                QPixmap photo(previousPhotoPath);
                if (!photo.isNull()) {
                    QPixmap circularPhoto(140, 140);
                    circularPhoto.fill(Qt::transparent);

                    QPainter painter(&circularPhoto);
                    painter.setRenderHint(QPainter::Antialiasing);
                    painter.setRenderHint(QPainter::SmoothPixmapTransform);

                    QPainterPath path;
                    path.addEllipse(0, 0, 140, 140);
                    painter.setClipPath(path);

                    const QPixmap scaledPhoto = photo.scaled(140, 140, 
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    painter.drawPixmap(0, 0, scaledPhoto);

                    painter.setClipping(false);
                    painter.setPen(QPen(QColor("#8B5CF6"), 3));
                    painter.drawEllipse(1, 1, 137, 137);

                    profileImageButton->setIcon(QIcon(circularPhoto));
                    profileImageButton->setIconSize(QSize(140, 140));
                }
            } else {
                resetToDefaultProfileImage();
            }

            showMessageDialog(tr("%1\nPrevious data has been restored.").arg(errorMessage), true);
            return;
        }

        userData.setName(nameEdit->text());
        userData.setEmail(emailEdit->text());
        userData.setUserPhotoPath(currentPhotoPath);

        QString saveErrorMessage;
        if (userDataManager->saveUserData(userData, saveErrorMessage)) {
            showMessageDialog(tr("Your profile has been updated successfully!"));
        } else {
            // If save fails, restore previous data
            nameEdit->setText(previousName);
            emailEdit->setText(previousEmail);
            if (!previousPhotoPath.isEmpty() && QFile::exists(previousPhotoPath)) {
                currentPhotoPath = previousPhotoPath;
                QPixmap photo(previousPhotoPath);
                if (!photo.isNull()) {
                    QPixmap circularPhoto(140, 140);
                    circularPhoto.fill(Qt::transparent);

                    QPainter painter(&circularPhoto);
                    painter.setRenderHint(QPainter::Antialiasing);
                    painter.setRenderHint(QPainter::SmoothPixmapTransform);

                    QPainterPath path;
                    path.addEllipse(0, 0, 140, 140);
                    painter.setClipPath(path);

                    const QPixmap scaledPhoto = photo.scaled(140, 140, 
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    painter.drawPixmap(0, 0, scaledPhoto);

                    painter.setClipping(false);
                    painter.setPen(QPen(QColor("#8B5CF6"), 3));
                    painter.drawEllipse(1, 1, 137, 137);

                    profileImageButton->setIcon(QIcon(circularPhoto));
                    profileImageButton->setIconSize(QSize(140, 140));
                }
            } else {
                resetToDefaultProfileImage();
            }
            showMessageDialog(tr("Failed to update profile: %1\nPrevious data has been restored.").arg(saveErrorMessage), true);
        }
    }
}

void SettingsPage::resetChanges()
{
    QString currentUserEmail;
    QString dummyPassword;
    if (userDataManager->getRememberedCredentials(currentUserEmail, dummyPassword)) {
        User userData = userDataManager->getUserData(currentUserEmail);
        
        // Restore all values to last saved state
        nameEdit->setText(userData.getName());
        emailEdit->setText(userData.getEmail());
        
        const QString photoPath = userData.getUserPhotoPath();
        if (!photoPath.isEmpty() && QFile::exists(photoPath)) {
            currentPhotoPath = photoPath;
            QPixmap photo(photoPath);
            if (!photo.isNull()) {
                QPixmap circularPhoto(140, 140);
                circularPhoto.fill(Qt::transparent);

                QPainter painter(&circularPhoto);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);

                QPainterPath path;
                path.addEllipse(0, 0, 140, 140);
                painter.setClipPath(path);

                const QPixmap scaledPhoto = photo.scaled(140, 140, 
                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
                painter.drawPixmap(0, 0, scaledPhoto);

                painter.setClipping(false);
                painter.setPen(QPen(QColor("#8B5CF6"), 3));
                painter.drawEllipse(1, 1, 137, 137);

                profileImageButton->setIcon(QIcon(circularPhoto));
                profileImageButton->setIconSize(QSize(140, 140));
            }
        } else {
            resetToDefaultProfileImage();
        }

        showMessageDialog(tr("All changes have been reset to last saved state."));
    }
}

void SettingsPage::handleLogout()
{
    showConfirmationDialog(
        tr("Confirm Logout"),
        tr("Are you sure you want to logout?"),
        tr("Yes"),
        tr("No"),
        [this]() {
            nameEdit->clear();
            emailEdit->clear();
            resetToDefaultProfileImage();
            currentPhotoPath = "";

            MainPage* mainPage = qobject_cast<MainPage*>(parent()->parent());
            if (mainPage) {
                mainPage->clearUserData();
            }

            emit logoutRequested();
        }
    );
}

void SettingsPage::handleDeleteAccount()
{
    showConfirmationDialog(
        tr("Delete Account"),
        tr("Are you sure you want to delete your account? This action cannot be undone."),
        tr("Delete Account"),
        tr("Cancel"),
        [this]() {
            QString errorMessage;
            if (userDataManager->deleteAccount(emailEdit->text(), errorMessage)) {
                userDataManager->clearRememberedCredentials();
                showMessageDialog(tr("Account successfully deleted."));
                emit accountDeleted();
                emit logoutRequested();
            } else {
                showMessageDialog(tr("Failed to delete account: %1").arg(errorMessage), true);
            }
        }
    );
}

void SettingsPage::showConfirmationDialog(const QString& title, const QString& message,
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

    // Add fade animations
    const auto opacityEffect = new QGraphicsOpacityEffect(dialog);
    container->setGraphicsEffect(opacityEffect);

    const auto fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, [dialog, onConfirm]() {
        onConfirm();
        dialog->accept();
    });

    dialog->show();
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    dialog->exec();
    dialog->deleteLater();
}

void SettingsPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void SettingsPage::updateLayout()
{
    const QSize windowSize = size();

    int cardWidth;
    int contentPadding;
    int profileImageSize;
    int buttonWidth;
    
    if (windowSize.width() < 600) {
        cardWidth = qMax(320, static_cast<int>(windowSize.width() * 0.9));
        contentPadding = 16;
        profileImageSize = 100;
        buttonWidth = cardWidth - (contentPadding * 2);
    } else if (windowSize.width() < 800) {
        cardWidth = qMax(400, static_cast<int>(windowSize.width() * 0.8));
        contentPadding = 24;
        profileImageSize = 120;
        buttonWidth = cardWidth - (contentPadding * 2);
    } else {
        cardWidth = qBound(480, static_cast<int>(windowSize.width() * 0.7), 800);
        contentPadding = 40;
        profileImageSize = 140;
        buttonWidth = cardWidth - (contentPadding * 2);
    }

    // Update card container
    if (const auto container = findChild<QWidget*>("cardContainer")) {
        container->setFixedWidth(cardWidth);
        container->setStyleSheet(QString(R"(
            QWidget#cardContainer {
                background: %1;
                border-radius: 24px;
                border: 1px solid %2;
            }
        )").arg(
            isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
            isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
        ));
        
        if (auto* layout = qobject_cast<QVBoxLayout*>(container->layout())) {
            layout->setContentsMargins(contentPadding, contentPadding, contentPadding, contentPadding);
        }
    }

    // Update profile image button size
    if (profileImageButton) {
        profileImageButton->setFixedSize(profileImageSize, profileImageSize);
        if (!currentPhotoPath.isEmpty()) {
            QPixmap photo(currentPhotoPath);
            if (!photo.isNull()) {
                QPixmap circularPhoto(profileImageSize, profileImageSize);
                circularPhoto.fill(Qt::transparent);

                QPainter painter(&circularPhoto);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);

                QPainterPath path;
                path.addEllipse(0, 0, profileImageSize, profileImageSize);
                painter.setClipPath(path);

                const QPixmap scaledPhoto = photo.scaled(profileImageSize, profileImageSize, 
                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
                painter.drawPixmap(0, 0, scaledPhoto);

                painter.setClipping(false);
                painter.setPen(QPen(QColor("#8B5CF6"), 3));
                painter.drawEllipse(1, 1, profileImageSize - 2, profileImageSize - 2);

                profileImageButton->setIcon(QIcon(circularPhoto));
                profileImageButton->setIconSize(QSize(profileImageSize, profileImageSize));
            }
        }
    }

    // Update input fields width
    if (nameEdit) nameEdit->setFixedWidth(buttonWidth);
    if (emailEdit) emailEdit->setFixedWidth(buttonWidth);

    // Update buttons width
    if (logoutButton) logoutButton->setFixedWidth(buttonWidth);
    if (deleteAccountButton) deleteAccountButton->setFixedWidth(buttonWidth);
    if (saveButton) saveButton->setFixedWidth(180);

    // Update font sizes
    const QString nameFontSize = windowSize.width() < 600 ? "20px" : "24px";
    const QString emailFontSize = windowSize.width() < 600 ? "14px" : "16px";
    const QString buttonFontSize = windowSize.width() < 600 ? "13px" : "15px";

    if (nameEdit) {
        nameEdit->setStyleSheet(QString(R"(
            QLineEdit {
                background: transparent;
                border: none;
                color: %1;
                font-size: %2;
                font-weight: bold;
                padding: 4px;
            }
            QLineEdit:focus {
                background: rgba(139, 92, 246, 0.1);
                border-radius: 8px;
            }
        )").arg(isDarkTheme ? "#FFFFFF" : "#1F2937", nameFontSize));
    }

    if (emailEdit) {
        emailEdit->setStyleSheet(QString(R"(
            QLineEdit {
                background: transparent;
                border: none;
                color: %1;
                font-size: %2;
                padding: 4px;
            }
            QLineEdit:focus {
                background: rgba(139, 92, 246, 0.1);
                border-radius: 8px;
            }
        )").arg(isDarkTheme ? "rgba(255, 255, 255, 0.8)" : "#4B5563", emailFontSize));
    }

    // Update button styles with new font size
    const QString buttonStyle = QString(R"(
        QPushButton {
            background: #8B5CF6;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 8px 24px;
            font-size: %1;
            font-weight: 600;
        }
        QPushButton:hover {
            background: #7C3AED;
        }
    )").arg(buttonFontSize);

    if (saveButton) saveButton->setStyleSheet(buttonStyle);
}

void SettingsPage::resetToDefaultProfileImage()
{
    // Create a larger pixmap for better quality
    QPixmap defaultPhoto(140, 140);
    defaultPhoto.fill(Qt::transparent);

    QPainter painter(&defaultPhoto);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Draw the background circle
    painter.setPen(Qt::NoPen);
    painter.setBrush(isDarkTheme ? QColor("#374151") : QColor("#F3F4F6"));
    painter.drawEllipse(3, 3, 134, 134);

    // Draw the outer border circle (purple)
    painter.setPen(QPen(QColor("#8B5CF6"), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(3, 3, 134, 134);

    // Load and draw the appropriate user image based on theme (reversed)
    QPixmap userImage(isDarkTheme ? ":/Images/userW.png" : ":/Images/userB.png");
    if (!userImage.isNull()) {
        userImage = userImage.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // Calculate position to center the image
        int x = (defaultPhoto.width() - userImage.width()) / 2;
        int y = (defaultPhoto.height() - userImage.height()) / 2;
        painter.drawPixmap(x, y, userImage);
    }

    // Set the icon with the default avatar
    profileImageButton->setIcon(QIcon(defaultPhoto));
    profileImageButton->setIconSize(QSize(140, 140));
    
    // Clear the current photo path
    currentPhotoPath = "";

    // Update button style to ensure border is visible
    profileImageButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: transparent;
            border: 3px solid #8B5CF6;
            border-radius: 70px;
            padding: 0px;
            margin: 0px;
        }
        QPushButton:hover {
            background-color: %1;
        }
    )").arg(isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"));
}

void SettingsPage::retranslateUI()
{
    // Update all button texts
    if (saveButton) {
        saveButton->setText(tr("Save Changes"));
    }
    if (resetButton) {
        resetButton->setText(tr("Reset"));
    }
    if (logoutButton) {
        logoutButton->setText(tr("Log Out"));
    }
    if (deleteAccountButton) {
        deleteAccountButton->setText(tr("Delete Account"));
    }
    if (profileImageButton) {
        profileImageButton->setToolTip(tr("Change profile picture"));
    }

    // Update upload label if it exists
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->text() == "Change Photo" || label->text() == "Changer la Photo" || label->text() == "Foto ändern") {
            label->setText(tr("Change Photo"));
        }
    }
}

bool SettingsPage::eventFilter(QObject* watched, QEvent* event)
{
    // Handle events for specific widgets if needed
    if (event->type() == QEvent::MouseButtonPress) {
        // Handle mouse press events
        return false; // Let the event continue to propagate
    }
    
    return QWidget::eventFilter(watched, event);
} 