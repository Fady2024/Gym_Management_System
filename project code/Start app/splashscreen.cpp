#include "splashscreen.h"
#include <QApplication>
#include <QScreen>
#include "../Util/UIUtils.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>

SplashScreen::SplashScreen(QWidget* parent)
    : QWidget(parent)
    , container(new QWidget(this))
    , logoLabel(new QLabel(container))
    , titleLabel(new QLabel(container))
    , opacityEffect(new QGraphicsOpacityEffect(this))
    , fadeAnimation(new QPropertyAnimation(opacityEffect, "opacity", this))
    , mainAnimation(new QSequentialAnimationGroup(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    // Initialize container with modern styling
    container->setObjectName("container");
    container->setFixedSize(600, 600);
    container->setAttribute(Qt::WA_TranslucentBackground);
    
    // Set up opacity effect for the entire container
    container->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0.0);

    // Center the splash screen on the screen
    if (QScreen* screen = QApplication::primaryScreen()) {
        const QRect screenGeometry = screen->geometry();
        const QPoint center = screenGeometry.center();
        move(center.x() - width() / 2, center.y() - height() / 2);
    }

    setupUI();
    setupAnimations();

    // Connect to theme manager
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
            this, &SplashScreen::updateTheme);
    
    // Initial theme
    updateTheme(ThemeManager::getInstance().isDarkTheme());
}

void SplashScreen::setupUI()
{
    // Main layout with improved spacing
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Container layout with modern padding
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(40);
    containerLayout->setContentsMargins(60, 60, 60, 60);
    containerLayout->setAlignment(Qt::AlignCenter);

    // Logo setup with improved scaling
    logoLabel->setObjectName("logoLabel");
    QPixmap logo(":/Images/dumbbell.png");
    if (!logo.isNull()) {
        logoLabel->setFixedSize(200, 200);
        logoLabel->setScaledContents(false);
        
        QPixmap scaledLogo = logo.scaled(logoLabel->size(), 
                                       Qt::KeepAspectRatio, 
                                       Qt::SmoothTransformation);
        
        logoLabel->setPixmap(scaledLogo);
        logoLabel->setAlignment(Qt::AlignCenter);
    }

    // Title setup with modern styling
    titleLabel->setObjectName("titleLabel");
    titleLabel->setText("FitFlex<span style='color: #8B5CF6;'>Pro</span>");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setTextFormat(Qt::RichText);

    // Add drop shadow effect to title
    auto* shadowEffect = new QGraphicsDropShadowEffect(titleLabel);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(0, 2);
    titleLabel->setGraphicsEffect(shadowEffect);

    // Add widgets to container with proper spacing
    containerLayout->addStretch(1);
    containerLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    containerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    containerLayout->addStretch(1);

    // Add container to main layout
    mainLayout->addWidget(container, 0, Qt::AlignCenter);
}

void SplashScreen::setupAnimations()
{
    // Create smooth fade-in animation
    fadeAnimation->setDuration(1500);
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
    fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Add fade animation to sequence
    mainAnimation->addAnimation(fadeAnimation);

    // Wait for 2 seconds after fade-in completes before emitting finished signal
    connect(mainAnimation, &QSequentialAnimationGroup::finished, this, [this]() {
        QTimer::singleShot(2000, this, &SplashScreen::animationFinished);
    });
}

void SplashScreen::startAnimation()
{
    if (mainAnimation) {
        mainAnimation->start();
    }
}

void SplashScreen::updateTheme(bool isDark)
{
    // Set completely transparent background
    setStyleSheet("QWidget { background: transparent; }");
    container->setStyleSheet("QWidget#container { background: transparent; }");

    // Update title style with improved typography and shadow
    titleLabel->setStyleSheet(QString(
        "QLabel#titleLabel {"
        "   font-size: 48px;"
        "   font-weight: bold;"
        "   letter-spacing: -0.5px;"
        "   color: white;"
        "   background: transparent;"
        "}"
    ));
}

void SplashScreen::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (container) {
        const QSize newSize(qMin(width() - 80, 600), qMin(height() - 80, 600));
        container->setFixedSize(newSize);
    }
}

void SplashScreen::retranslateUI()
{
    if (titleLabel) {
        titleLabel->setText("FitFlex<span style='color: #8B5CF6;'>Pro</span>");
    }
}

bool SplashScreen::event(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange) {
        retranslateUI();
        return true;
    }
    return QWidget::event(e);
} 