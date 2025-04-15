#include "languageselectionpage.h"
#include "../Util/UIUtils.h"
#include "../Util/ThemeManager.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QPainter>
#include <QEvent>
#include <QGraphicsDropShadowEffect>

LanguageSelectionPage::LanguageSelectionPage(QWidget* parent)
    : QWidget(parent)
    , container(new QWidget(this))
    , logoLabel(new QLabel(container))
    , titleLabel(new QLabel(container))
    , subtitleLabel(new QLabel(container))
    , languageSelector(nullptr)
    , continueButton(new QPushButton(container))
    , opacityEffect(new QGraphicsOpacityEffect(container))
    , fadeAnimation(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Initialize container first
    container->setObjectName("container");
    container->setFixedWidth(480);
    container->setMinimumHeight(500);
    container->setMaximumHeight(600);
    
    // Set up opacity effect
    opacityEffect->setOpacity(0.0);
    container->setGraphicsEffect(opacityEffect);

    setupUI();
    setupAnimations();

    // Connect to theme manager
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
            this, &LanguageSelectionPage::updateTheme);
    
    // Initial theme
    updateTheme(ThemeManager::getInstance().isDarkTheme());
}

void LanguageSelectionPage::setupUI()
{
    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Container layout
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(40, 40, 40, 40);
    containerLayout->setSpacing(24);
    containerLayout->setAlignment(Qt::AlignCenter);

    // Logo (optional)
    logoLabel->setObjectName("logoLabel");
    logoLabel->setAlignment(Qt::AlignCenter);
    QPixmap logo(":/Images/dumbbell.png");
    if (!logo.isNull()) {
        logoLabel->setPixmap(logo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("FitFlex Pro");
        logoLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #8B5CF6;");
    }
    containerLayout->addWidget(logoLabel);

    // Title
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel#titleLabel {"
        "   font-size: 32px;"
        "   font-weight: bold;"
        "   letter-spacing: -0.5px;"
        "   margin-bottom: 12px;"
        "}"
    );
    containerLayout->addWidget(titleLabel);

    // Subtitle
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "QLabel#subtitleLabel {"
        "   font-size: 16px;"
        "   margin-bottom: 32px;"
        "   color: #64748B;"
        "}"
    );
    containerLayout->addWidget(subtitleLabel);

    // Language selector
    languageSelector = new LanguageSelector(container);
    languageSelector->setFixedWidth(280);
    languageSelector->setFixedHeight(48);
    containerLayout->addWidget(languageSelector, 0, Qt::AlignCenter);
    containerLayout->addSpacing(24);

    // Continue button
    continueButton->setObjectName("continueButton");
    continueButton->setFixedSize(280, 48);
    continueButton->setCursor(Qt::PointingHandCursor);
    continueButton->setStyleSheet(
        "QPushButton#continueButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #8B5CF6, stop:1 #7C3AED);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 24px;"
        "   font-size: 16px;"
        "   font-weight: 600;"
        "   letter-spacing: 0.3px;"
        "   transition: all 0.2s ease;"
        "}"
        "QPushButton#continueButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #7C3AED, stop:1 #6D28D9);"
        "   transform: translateY(-1px);"
        "   box-shadow: 0 4px 12px rgba(139, 92, 246, 0.3);"
        "}"
        "QPushButton#continueButton:pressed {"
        "   transform: translateY(1px);"
        "   box-shadow: 0 2px 6px rgba(139, 92, 246, 0.2);"
        "}"
    );
    containerLayout->addWidget(continueButton, 0, Qt::AlignCenter);

    // Add container to main layout
    mainLayout->addWidget(container);

    // Connect button
    connect(continueButton, &QPushButton::clicked, 
            this, &LanguageSelectionPage::onContinueClicked);

    // Set initial theme style
    setStyleSheet(
        "LanguageSelectionPage {"
        "   background: #1E293B;"
        "}"
        "QWidget#container {"
        "   background: rgba(30, 41, 59, 0.90);"
        "   border-radius: 24px;"
        "}"
    );

    // Initialize translations
    retranslateUI();
}

void LanguageSelectionPage::setupAnimations()
{
    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(800);
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void LanguageSelectionPage::startAnimation()
{
    if (fadeAnimation) {
        fadeAnimation->start();
    }
}

void LanguageSelectionPage::updateTheme(bool isDark)
{
    // Update container style
    if (container) {
        container->setStyleSheet(QString(
            "QWidget#container {"
            "   background: %1;"
            "   border-radius: 24px;"
            "}"
        ).arg(isDark ? "rgba(30, 41, 59, 0.90)" : "rgba(255, 255, 255, 0.95)"));
    }

    // Update title label
    if (titleLabel) {
        titleLabel->setStyleSheet(QString(
            "QLabel#titleLabel {"
            "   font-size: 32px;"
            "   font-weight: bold;"
            "   color: %1;"
            "   letter-spacing: -0.5px;"
            "   margin-bottom: 12px;"
            "}"
        ).arg(isDark ? "#FFFFFF" : "#1E293B"));
    }

    // Update subtitle label
    if (subtitleLabel) {
        subtitleLabel->setStyleSheet(QString(
            "QLabel#subtitleLabel {"
            "   font-size: 16px;"
            "   margin-bottom: 32px;"
            "   color: %1;"
            "}"
        ).arg(isDark ? "#94A3B8" : "#64748B"));
    }

    // Update language selector
    if (languageSelector) {
        languageSelector->updateTheme(isDark);
    }
}

void LanguageSelectionPage::onContinueClicked()
{
    emit languageSelected();
}

void LanguageSelectionPage::retranslateUI()
{
    if (titleLabel) {
        titleLabel->setText(tr("Select Your Language"));
    }
    if (subtitleLabel) {
        subtitleLabel->setText(tr("Choose your preferred language to continue"));
    }
    if (continueButton) {
        continueButton->setText(tr("Continue"));
    }
    if (logoLabel && logoLabel->pixmap().isNull()) {
        logoLabel->setText(tr("FitFlex Pro"));
    }
}

bool LanguageSelectionPage::event(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange) {
        retranslateUI();
        return true;
    }
    return QWidget::event(e);
} 