#include "onboardingpage.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QPainter>
#include "../Util/UIUtils.h"
#include "../Util/ThemeManager.h"
#include <QDebug>
#include <QSequentialAnimationGroup>

OnboardingPage::OnboardingPage(QWidget* parent)
    : QWidget(parent)
    , container(new QWidget(this))
    , topBar(new QWidget(container))
    , imageLabel(new QLabel(container))
    , titleLabel(new QLabel(container))
    , descriptionLabel(new QLabel(container))
    , skipButton(new QPushButton(container))
    , nextButton(new QPushButton(container))
    , backButton(new QPushButton(container))
    , progressDotsContainer(new QWidget(container))
    , opacityEffect(new QGraphicsOpacityEffect(container))
    , fadeAnimation(nullptr)
    , slideAnimation(nullptr)
    , currentPage(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Initialize container first
    container->setObjectName("container");
    container->setMinimumWidth(1000);
    container->setMinimumHeight(600);
    
    // Set up opacity effect
    opacityEffect->setOpacity(0.0);
    container->setGraphicsEffect(opacityEffect);

    setupUI();
    setupAnimations();

    // Connect to theme manager
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
            this, &OnboardingPage::updateTheme);
    
    // Initial theme
    updateTheme(ThemeManager::getInstance().isDarkTheme());
}

void OnboardingPage::setupUI()
{
    // Main layout with improved spacing
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create left panel for image
    auto* leftPanel = new QWidget(container);
    leftPanel->setObjectName("leftPanel");
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // Set up top bar
    topBar->setObjectName("topBar");
    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(32, 24, 32, 24);
    topBarLayout->setSpacing(0);
    
    // Set up skip button with responsive size
    skipButton->setObjectName("skipButton");
    skipButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    skipButton->setMinimumSize(100, 40);
    skipButton->setMaximumSize(120, 44);
    skipButton->setCursor(Qt::PointingHandCursor);
    skipButton->setText(tr("Skip"));

    topBarLayout->addWidget(skipButton, 0, Qt::AlignLeft);
    topBarLayout->addStretch(1);

    // Image container with responsive sizing
    auto* imageContainer = new QWidget(leftPanel);
    imageContainer->setObjectName("imageContainer");
    imageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* imageLayout = new QVBoxLayout(imageContainer);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setAlignment(Qt::AlignCenter);

    // Set up image label with responsive sizing
    imageLabel->setObjectName("imageLabel");
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel->setMinimumSize(300, 300);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLayout->addWidget(imageLabel);

    leftLayout->addWidget(topBar);
    leftLayout->addWidget(imageContainer, 1);

    // Create right panel for content with responsive width
    auto* rightPanel = new QWidget(container);
    rightPanel->setObjectName("rightPanel");
    rightPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    rightPanel->setMinimumWidth(300);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(24);
    rightLayout->setAlignment(Qt::AlignCenter);

    // Dynamic padding based on container size
    rightLayout->setContentsMargins(60, 60, 60, 60);  // Set default margins
    
    // Set up title label with responsive font size
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignLeft);
    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Set up description label with responsive font size
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setAlignment(Qt::AlignLeft);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Progress dots with improved styling
    progressDotsContainer->setObjectName("progressDotsContainer");
    auto* dotsLayout = new QHBoxLayout(progressDotsContainer);
    dotsLayout->setSpacing(12);
    dotsLayout->setAlignment(Qt::AlignLeft);
    dotsLayout->setContentsMargins(0, 16, 0, 16);
    
    for (int i = 0; i < 3; ++i) {
        auto* dot = new QLabel(progressDotsContainer);
        dot->setFixedSize(8, 8);
        dot->setProperty("dot-index", i);
        progressDots.append(dot);
        dotsLayout->addWidget(dot);
    }

    // Navigation buttons with responsive sizing
    auto* buttonsContainer = new QWidget(rightPanel);
    auto* buttonsLayout = new QHBoxLayout(buttonsContainer);
    buttonsLayout->setSpacing(16);
    buttonsLayout->setContentsMargins(0, 16, 0, 0);
    buttonsLayout->setAlignment(Qt::AlignLeft);

    // Set up back button with responsive size
    backButton->setObjectName("backButton");
    backButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    backButton->setMinimumSize(120, 44);
    backButton->setMaximumSize(160, 48);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setText(tr("Back"));
    backButton->setVisible(false);

    // Set up next button with responsive size
    nextButton->setObjectName("nextButton");
    nextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nextButton->setMinimumSize(160, 44);  // Increased minimum width
    nextButton->setMaximumSize(200, 48);
    nextButton->setCursor(Qt::PointingHandCursor);
    nextButton->setText(tr("Next"));

    buttonsLayout->addWidget(backButton);
    buttonsLayout->addWidget(nextButton);
    buttonsLayout->addStretch();

    // Add all elements to right panel
    rightLayout->addStretch();
    rightLayout->addWidget(titleLabel);
    rightLayout->addWidget(descriptionLabel);
    rightLayout->addWidget(progressDotsContainer);
    rightLayout->addWidget(buttonsContainer);
    rightLayout->addStretch();

    // Add panels to main layout with responsive ratios
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addWidget(rightPanel, 2);

    // Connect buttons
    connect(skipButton, &QPushButton::clicked, this, &OnboardingPage::onSkipClicked);
    connect(nextButton, &QPushButton::clicked, this, &OnboardingPage::onNextClicked);
    connect(backButton, &QPushButton::clicked, this, &OnboardingPage::onBackClicked);

    // Update styles based on current theme
    updateTheme(ThemeManager::getInstance().isDarkTheme());

    // Initialize first page
    updatePage(0);
}

void OnboardingPage::setupAnimations()
{
    if (!container) return;  // Safety check

    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(500);
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    slideAnimation = new QPropertyAnimation(container, "pos", this);
    slideAnimation->setDuration(400);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void OnboardingPage::startAnimation()
{
    if (fadeAnimation) {
        fadeAnimation->start();
    }
}

void OnboardingPage::updatePage(int pageIndex)
{
    if (!container) return;

    // Store previous page for animation direction
    const int previousPage = currentPage;
    currentPage = pageIndex;
    
    // Show/hide back button based on page
    if (backButton) {
        backButton->setVisible(pageIndex > 0);
    }

    // Update progress dots with consistent circular shape
    bool isDark = ThemeManager::getInstance().isDarkTheme();
    QString activeDotColor = isDark ? "#8B5CF6" : "#7C3AED";
    QString inactiveDotColor = isDark ? "#475569" : "#e2e8f0";
    
    for (int i = 0; i < progressDots.size(); ++i) {
        QLabel* dot = progressDots[i];
        const bool isActive = i == pageIndex;
        const int size = isActive ? 10 : 8;
        dot->setFixedSize(size, size);
        dot->setStyleSheet(QString(
            "QLabel {"
            "   background-color: %1;"
            "   border-radius: %2px;"
            "   transition: all 0.3s ease;"
            "}"
        ).arg(i == pageIndex ? activeDotColor : inactiveDotColor)
         .arg(size / 2));
    }

    // Create fade out animation
    auto fadeOut = new QPropertyAnimation(container, "windowOpacity", this);
    fadeOut->setDuration(150);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InQuad);

    // Create fade in animation
    auto fadeIn = new QPropertyAnimation(container, "windowOpacity", this);
    fadeIn->setDuration(150);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutQuad);

    // Create animation group
    auto animGroup = new QSequentialAnimationGroup(this);
    animGroup->addAnimation(fadeOut);

    // Update content during the fade
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, pageIndex]() {
        // Update content
        switch (pageIndex) {
            case 0:
                titleLabel->setText(tr("Visible Changes in 3 Months"));
                descriptionLabel->setText(tr("Transform your body and see real results with our personalized workout plans and expert guidance"));
                loadAndScaleImage(":/Images/onboarding1.png");
                nextButton->setText(tr("Next"));
                break;
            case 1:
                titleLabel->setText(tr("Forget About Strict Diet"));
                descriptionLabel->setText(tr("Enjoy flexible nutrition plans that work with your lifestyle while achieving your fitness goals"));
                loadAndScaleImage(":/Images/onboarding2.png");
                nextButton->setText(tr("Next"));
                break;
            case 2:
                titleLabel->setText(tr("Save Money on Gym Membership"));
                descriptionLabel->setText(tr("Get access to premium workout plans and nutrition guidance at a fraction of traditional gym costs"));
                loadAndScaleImage(":/Images/onboarding3.png");
                nextButton->setText(tr("Get Started"));
                break;
        }
    });

    animGroup->addAnimation(fadeIn);
    animGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void OnboardingPage::loadAndScaleImage(const QString& imagePath)
{
    if (!imageLabel) return;

    QPixmap originalPixmap(imagePath);
    if (originalPixmap.isNull()) {
        qDebug() << "Failed to load image:" << imagePath;
        return;
    }

    // Get the label's size
    const QSize labelSize = imageLabel->size();
    
    // Calculate the target size based on the container's dimensions
    const float containerAspect = static_cast<float>(container->width()) / container->height();
    const bool isVerticalLayout = containerAspect < 1.2f;
    
    // Adjust scale factor based on layout
    const float scaleFactor = isVerticalLayout ? 0.7f : 0.85f;
    
    // Calculate target dimensions while maintaining aspect ratio
    const int maxWidth = static_cast<int>(labelSize.width() * scaleFactor);
    const int maxHeight = static_cast<int>(labelSize.height() * scaleFactor);
    
    // Scale image while preserving aspect ratio
    QPixmap scaledPixmap = originalPixmap.scaled(
        maxWidth,
        maxHeight,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    // Create a transparent pixmap for the final image
    QPixmap finalPixmap(labelSize);
    finalPixmap.fill(Qt::transparent);

    // Center the scaled image
    QPainter painter(&finalPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const int x = (labelSize.width() - scaledPixmap.width()) / 2;
    const int y = (labelSize.height() - scaledPixmap.height()) / 2;

    painter.drawPixmap(x, y, scaledPixmap);
    imageLabel->setPixmap(finalPixmap);
}

void OnboardingPage::updateTheme(bool isDark)
{
    // Update panel styles
    if (auto* leftPanel = findChild<QWidget*>("leftPanel")) {
        leftPanel->setStyleSheet(QString(
            "QWidget#leftPanel {"
            "   background: %1;"
            "}"
        ).arg(isDark ? "rgba(30, 41, 59, 0.95)" : "rgba(255, 255, 255, 0.95)"));
    }

    if (auto* rightPanel = findChild<QWidget*>("rightPanel")) {
        rightPanel->setStyleSheet(QString(
            "QWidget#rightPanel {"
            "   background: %1;"
            "}"
        ).arg(isDark ? "#1E293B" : "#FFFFFF"));
    }

    // Update back button style
    if (backButton) {
        backButton->setStyleSheet(QString(
            "QPushButton#backButton {"
            "   background: transparent;"
            "   color: %1;"
            "   border: 2px solid %2;"
            "   border-radius: 22px;"
            "   font-size: 16px;"
            "   font-weight: 500;"
            "   padding: 10px 32px;"
            "   min-height: 44px;"
            "   max-height: 44px;"
            "   min-width: 120px;"
            "   transition: all 0.3s ease;"
            "}"
            "QPushButton#backButton:hover {"
            "   background: %2;"
            "   color: %3;"
            "   transform: scale(1.02);"
            "   box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);"
            "}"
            "QPushButton#backButton:pressed {"
            "   transform: scale(0.98);"
            "   box-shadow: none;"
            "}"
        ).arg(
            isDark ? "#94A3B8" : "#64748B",
            isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.05)",
            isDark ? "#FFFFFF" : "#1E293B"
        ));
    }

    // Update skip button style
    if (skipButton) {
        skipButton->setStyleSheet(QString(
            "QPushButton#skipButton {"
            "   background: transparent;"
            "   color: %1;"
            "   border: 2px solid %2;"
            "   border-radius: 20px;"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   padding: 8px 24px;"
            "   min-height: 40px;"
            "   max-height: 40px;"
            "   min-width: 100px;"
            "   transition: all 0.3s ease;"
            "}"
            "QPushButton#skipButton:hover {"
            "   background: %2;"
            "   color: %3;"
            "   transform: scale(1.02);"
            "   box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);"
            "}"
            "QPushButton#skipButton:pressed {"
            "   transform: scale(0.98);"
            "   box-shadow: none;"
            "}"
        ).arg(
            isDark ? "#94A3B8" : "#64748B",
            isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.05)",
            isDark ? "#FFFFFF" : "#1E293B"
        ));
    }

    // Update next button with more modern pill shape
    if (nextButton) {
        nextButton->setStyleSheet(
            "QPushButton#nextButton {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #8B5CF6, stop:0.5 #7C3AED, stop:1 #6D28D9);"
            "   color: white;"
            "   border: none;"
            "   border-radius: 22px;"
            "   font-size: 16px;"
            "   font-weight: 600;"
            "   letter-spacing: 0.3px;"
            "   padding: 0;"
            "   margin: 0;"
            "   min-height: 44px;"
            "   max-height: 48px;"
            "   min-width: 160px;"
            "   max-width: 200px;"
            "   text-align: center;"
            "   transition: all 0.3s ease;"
            "   box-shadow: 0 4px 12px rgba(139, 92, 246, 0.2);"
            "   qproperty-alignment: AlignCenter;"
            "}"
            "QPushButton#nextButton:hover {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #7C3AED, stop:0.5 #6D28D9, stop:1 #5B21B6);"
            "   transform: translateY(-2px) scale(1.02);"
            "   box-shadow: 0 6px 16px rgba(139, 92, 246, 0.3);"
            "}"
            "QPushButton#nextButton:pressed {"
            "   transform: translateY(1px) scale(0.98);"
            "   box-shadow: 0 2px 8px rgba(139, 92, 246, 0.2);"
            "}"
        );
    }

    // Update progress dots
    QString activeDotColor = isDark ? "#8B5CF6" : "#7C3AED";
    QString inactiveDotColor = isDark ? "#475569" : "#e2e8f0";
    
    for (int i = 0; i < progressDots.size(); ++i) {
        QLabel* dot = progressDots[i];
        const bool isActive = i == currentPage;
        const int size = isActive ? 10 : 8;
        dot->setFixedSize(size, size);
        dot->setStyleSheet(QString(
            "QLabel {"
            "   background-color: %1;"
            "   border-radius: %2px;"
            "   transition: all 0.3s ease;"
            "}"
        ).arg(i == currentPage ? activeDotColor : inactiveDotColor)
         .arg(size / 2));
    }
    
    // Update title label
    if (titleLabel) {
        const int containerWidth = container->width();
        const int fontSize = qMax(24, qMin(42, static_cast<int>(containerWidth / 25)));
        const QString titleStyle = QString(
            "QLabel#titleLabel {"
            "   font-size: %1px;"
            "   font-weight: bold;"
            "   letter-spacing: -0.5px;"
            "   line-height: 1.2;"
            "   color: %2;"
            "}"
        ).arg(
            QString::number(fontSize),
            ThemeManager::getInstance().isDarkTheme() ? "#FFFFFF" : "#1E293B"
        );
        titleLabel->setStyleSheet(titleStyle);
    }
    
    // Update description label
    if (descriptionLabel) {
        const int containerWidth = container->width();
        const int fontSize = qMax(14, qMin(18, static_cast<int>(containerWidth / 50)));
        const QString descStyle = QString(
            "QLabel#descriptionLabel {"
            "   font-size: %1px;"
            "   line-height: 1.6;"
            "   margin-top: 16px;"
            "   color: %2;"
            "}"
        ).arg(
            QString::number(fontSize),
            ThemeManager::getInstance().isDarkTheme() ? "#94A3B8" : "#64748B"
        );
        descriptionLabel->setStyleSheet(descStyle);
    }
}

void OnboardingPage::onNextClicked()
{
    if (currentPage < 2) {
        currentPage++;
        updatePage(currentPage);
    } else {
        emit onboardingCompleted();
    }
}

void OnboardingPage::onSkipClicked()
{
    emit onboardingCompleted();
}

void OnboardingPage::onBackClicked()
{
    if (currentPage > 0) {
        currentPage--;
        updatePage(currentPage);
    }
}

void OnboardingPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    const int w = width();
    const int h = height();
    
    // Determine layout mode based on aspect ratio and minimum sizes
    const bool useVerticalLayout = (w < 1000) || (static_cast<float>(w) / h < 1.2f);
    
    if (auto* mainLayout = layout()) {
        // Convert to QBoxLayout to handle both horizontal and vertical layouts
        if (auto* boxLayout = qobject_cast<QBoxLayout*>(mainLayout)) {
            if (useVerticalLayout) {
                boxLayout->setDirection(QBoxLayout::TopToBottom);
                // Adjust panel sizes for vertical layout
                if (auto* leftPanel = findChild<QWidget*>("leftPanel")) {
                    leftPanel->setMaximumHeight(h * 0.6);
                }
                if (auto* rightPanel = findChild<QWidget*>("rightPanel")) {
                    rightPanel->setMinimumWidth(0);
                    rightPanel->setMaximumWidth(QWIDGETSIZE_MAX);
                }
            } else {
                boxLayout->setDirection(QBoxLayout::LeftToRight);
                // Reset panel constraints for horizontal layout
                if (auto* leftPanel = findChild<QWidget*>("leftPanel")) {
                    leftPanel->setMaximumHeight(QWIDGETSIZE_MAX);
                }
                if (auto* rightPanel = findChild<QWidget*>("rightPanel")) {
                    rightPanel->setMinimumWidth(300);
                    rightPanel->setMaximumWidth(600);
                }
            }
        }
    }

    // Update button sizes
    const int containerWidth = container->width();
    const int buttonWidth = qMax<int>(120, qMin<int>(160, static_cast<int>(containerWidth / 8)));
    const int buttonHeight = qMax<int>(44, qMin<int>(48, static_cast<int>(containerWidth / 25)));
    
    const int skipButtonWidth = qMax<int>(100, qMin<int>(120, static_cast<int>(buttonWidth * 0.75)));
    const int skipButtonHeight = qMax<int>(40, qMin<int>(44, static_cast<int>(buttonHeight * 0.92)));
    
    skipButton->setFixedSize(skipButtonWidth, skipButtonHeight);
    backButton->setFixedSize(buttonWidth, buttonHeight);
    nextButton->setFixedSize(buttonWidth, buttonHeight);

    // Update image with a small delay to ensure proper layout
    QTimer::singleShot(50, this, [this]() {
        if (currentPage >= 0 && currentPage < imagePaths.size()) {
            loadAndScaleImage(imagePaths[currentPage]);
        }
    });
}

void OnboardingPage::retranslateUI()
{
    // Update skip and next buttons
    if (skipButton) {
        skipButton->setText(tr("Skip"));
    }
    if (nextButton) {
        nextButton->setText(currentPage == 2 ? tr("Get Started") : tr("Next"));
    }

    // Update content based on current page
    if (titleLabel && descriptionLabel) {
        switch (currentPage) {
            case 0:
                titleLabel->setText(tr("Visible Changes in 3 Months"));
                descriptionLabel->setText(tr("Transform your body and see real results with our personalized workout plans and expert guidance"));
                break;
            case 1:
                titleLabel->setText(tr("Forget About Strict Diet"));
                descriptionLabel->setText(tr("Enjoy flexible nutrition plans that work with your lifestyle while achieving your fitness goals"));
                break;
            case 2:
                titleLabel->setText(tr("Save Money on Gym Membership"));
                descriptionLabel->setText(tr("Get access to premium workout plans and nutrition guidance at a fraction of traditional gym costs"));
                break;
        }
    }
}

bool OnboardingPage::event(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange) {
        retranslateUI();
        return true;
    }
    return QWidget::event(e);
} 